/*
 * Copyright (c) 2014 Nuvoton Technology corporation.
 * Copyright 2017 Google, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation;version 2 of the License.
 */

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/clockchips.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>

/*---------------------------------------------------------------------------*/
/* timer used by CPU for clock events                                        */
/*---------------------------------------------------------------------------*/

static void __iomem *timer_base;
#define REG_TCSR0	(timer_base)
#define REG_TICR0	(timer_base + 0x8)
#define REG_TCSR1	(timer_base + 0x4)
#define REG_TICR1	(timer_base + 0xc)
#define REG_TDR1	(timer_base + 0x14)
#define REG_TISR	(timer_base + 0x18)

#define RESETINT	0x1f
#define PERIOD		(0x01 << 27)
#define ONESHOT		(0x00 << 27)
#define COUNTEN		(0x01 << 30)
#define INTEN		(0x01 << 29)

#define TICKS_PER_SEC	100
#define PRESCALE	0x63 /* Divider = prescale + 1 */

#define	TDR_SHIFT	24

static unsigned int timer0_load;

static int npcm750_timer_oneshot(struct clock_event_device *evt)
{
	unsigned int val;

	val = __raw_readl(REG_TCSR0);
	val &= ~(0x03 << 27);
	val |= (ONESHOT | COUNTEN | INTEN | PRESCALE);
	__raw_writel(val, REG_TCSR0);

	return 0;
}

static int npcm750_timer_periodic(struct clock_event_device *evt)
{
	unsigned int val;

	val = __raw_readl(REG_TCSR0);
	val &= ~(0x03 << 27);

	__raw_writel(timer0_load, REG_TICR0);
	val |= (PERIOD | COUNTEN | INTEN | PRESCALE);

	__raw_writel(val, REG_TCSR0);

	return 0;
}

static int npcm750_clockevent_setnextevent(unsigned long evt,
		struct clock_event_device *clk)
{
	unsigned int val;

	__raw_writel(evt, REG_TICR0);
	val = __raw_readl(REG_TCSR0);
	val |= (COUNTEN | INTEN | PRESCALE);
	__raw_writel(val, REG_TCSR0);

	return 0;
}

static struct clock_event_device npcm750_clockevent_device = {
	.name		    = "npcm750-timer0",
	.features	    = CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT,
	.set_next_event	    = npcm750_clockevent_setnextevent,
	.set_state_shutdown = npcm750_timer_oneshot,
	.set_state_periodic = npcm750_timer_periodic,
	.set_state_oneshot  = npcm750_timer_oneshot,
	.tick_resume	    = npcm750_timer_oneshot,
	.rating		    = 300,
};

static irqreturn_t npcm750_timer0_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *evt = &npcm750_clockevent_device;

	__raw_writel(0x01, REG_TISR); /* clear TIF0 */

	if (evt->event_handler)
		evt->event_handler(evt);

	return IRQ_HANDLED;
}

static struct irqaction npcm750_timer0_irq = {
	.name		= "npcm750-timer0",
	.flags		= IRQF_TIMER | IRQF_IRQPOLL,
	.handler	= npcm750_timer0_interrupt,
};

static void __init npcm750_clockevents_init(int irq, struct device_node *np)
{
	unsigned int rate;
	struct clk *clk;

	__raw_writel(0x00, REG_TCSR0);

	clk = of_clk_get(np, 0);

	if (IS_ERR(clk)) {
		pr_info("Unable to get timer clock. Assuming 25Mhz input clock.\n");
		rate = 25000000;
	} else {
		clk_prepare_enable(clk);
		rate = clk_get_rate(clk);
	}

	rate = rate / (PRESCALE + 1);
	timer0_load = (rate / TICKS_PER_SEC);
	__raw_writel(RESETINT, REG_TISR);
	setup_irq(irq, &npcm750_timer0_irq);
	npcm750_clockevent_device.cpumask = cpumask_of(0);

	clockevents_config_and_register(&npcm750_clockevent_device, rate,
					0xf, 0xffffffff);
}

#ifdef CONFIG_CLKSRC_MMIO
static void __init npcm750_clocksource_init(struct device_node *np)
{
	unsigned int val;
	unsigned int rate;

	__raw_writel(0x00, REG_TCSR1);

	clk = of_clk_get(np, 0);

	if (IS_ERR(clk)) {
		pr_info("Unable to get timer clock. Assuming 25Mhz input clock.\n");
		rate = 25000000;
	} else {
		clk_prepare_enable(clk);
		rate = clk_get_rate(clk);
	}

	rate = rate / (PRESCALE + 1);

	__raw_writel(0xffffffff, REG_TICR1);

	val = __raw_readl(REG_TCSR1);
	val |= (COUNTEN | PERIOD | PRESCALE);
	__raw_writel(val, REG_TCSR1);

	clocksource_mmio_init(REG_TDR1, "npcm750-timer1", rate, 200,
		TDR_SHIFT, clocksource_mmio_readl_down);
}
#endif

static int __init npcm750_timer_init(struct device_node *np)
{
	int irq;
	struct resource res;
	int ret;

	irq = irq_of_parse_and_map(np, 0);
	if (!irq) {
		pr_err("%s : No irq passed for timer via DT\n", __func__);
		return -1;
	}

	ret = of_address_to_resource(np, 0, &res);
	if (ret) {
		pr_err("Timer of_address_to_resource fail ret %d\n", ret);
		return -EINVAL;
	}

	timer_base = ioremap(res.start, resource_size(&res));
	if (!timer_base) {
		pr_err("Timer_base ioremap fail\n");
		return -ENOMEM;
	}
#ifdef CONFIG_CLKSRC_MMIO
	npcm750_clocksource_init(np);
#endif
	npcm750_clockevents_init(irq, np);

	pr_info("%s Done\n", __func__);

	return 0;
}

TIMER_OF_DECLARE(npcm750, "nuvoton,npcm750-timer", npcm750_timer_init);

