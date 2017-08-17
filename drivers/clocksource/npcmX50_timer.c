/*---------------------------------------------------------------------------------------------------------*/
/*  Nuvoton Technology Corporation confidential                                                            */
/*                                                                                                         */
/*  Copyright (c) 2008 by Nuvoton Technology Corporation                                                   */
/*  All rights reserved                                                                                    */
/*                                                                                                         */
/*<<<------------------------------------------------------------------------------------------------------*/
/* File Contents:                                                                                          */
/*   npcmX50_timer.c                                                                                       */
/*            This file contains Kernel timer implementation                                               */
/*  Project:                                                                                               */
/*            Linux Kernel                                                                                 */
/*---------------------------------------------------------------------------------------------------------*/


#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/leds.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/of_irq.h>
#include <linux/irq.h>
#include <linux/clk-provider.h>
#include <linux/clkdev.h>
#include <linux/of_address.h>
#include <linux/clocksource.h>
#include <linux/clk/nuvoton.h>

#include <asm/mach-types.h>
#include <asm/mach/irq.h>
#include <asm/mach/time.h>

//--------------------------#include <mach/map.h>

// #include <mach/hal.h>
#include "npcmX50_timer.h"

//#ifdef CONFIG_OF
//static void __iomem *timer_base = NULL;
//#define timer_base(n)     (n==0)?timer_bases[0]:((n==1)?timer_bases[1]:timer_bases[2])
//#else
#define NPCMX50_TMR0_BASE_ADDR                  0xF0008000      /* Timer0  module registers  */
//#endif

/*---------------------------------------------------------------------------------------------------------*/
/* timer used by CPU for clock events                                                                      */
/*---------------------------------------------------------------------------------------------------------*/
//#define TIMER_CPU 0

#define REG_TCSR0   (( volatile void __iomem*)(timer_base))
#define REG_TICR0   (( volatile void __iomem*)(timer_base + 0x8))
#define REG_TCSR1   (( volatile void __iomem*)(timer_base + 0x4))
#define REG_TICR1   (( volatile void __iomem*)(timer_base + 0xc))
#define REG_TDR1    (( volatile void __iomem*)(timer_base + 0x14)) 
#define REG_TISR    (( volatile void __iomem*)(timer_base + 0x18))

#define RESETINT	0x1f
#define PERIOD		(0x01 << 27)
#define ONESHOT		(0x00 << 27)
#define COUNTEN		(0x01 << 30)
#define INTEN		(0x01 << 29)

#define TICKS_PER_SEC	100
#define PRESCALE	0x63 /* Divider = prescale + 1 */

#define	TDR_SHIFT	24

static void __iomem *timer_base;
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
	.name				= "npcm750-timer0",
	.features			= CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT,
	.set_next_event		= npcm750_clockevent_setnextevent,
	.set_state_shutdown	= npcm750_timer_oneshot,
	.set_state_periodic	= npcm750_timer_periodic,
	.set_state_oneshot	= npcm750_timer_oneshot,
	.tick_resume		= npcm750_timer_oneshot,
	.rating				= 300,
};

/*IRQ handler for the timer*/

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

static void __init npcm750_clockevents_init(int irq,struct device_node *np)
{
	unsigned int rate;
    struct clk *clk;
	/*struct clk *clk = clk_get(NULL, "timer0");
	BUG_ON(IS_ERR(clk));*/

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

	/*clk_enable(clk);
	rate = clk_get_rate(clk) / (PRESCALE + 1);*/

	/*rate = 25000000 / (PRESCALE + 1);*/

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
	/*struct clk *clk = clk_get(NULL, "timer1");

	BUG_ON(IS_ERR(clk));*/

	__raw_writel(0x00, REG_TCSR1);

	/*clk_enable(clk);
	rate = clk_get_rate(clk) / (PRESCALE + 1);*/
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

	irq = irq_of_parse_and_map(np, 0);
	if (!irq) {
		pr_err("%s: No irq passed for timer via DT\n", __func__);
		return -1;
	}

	timer_base = ioremap(NPCMX50_TMR0_BASE_ADDR, SZ_4K);

#ifdef CONFIG_CLKSRC_MMIO
	npcm750_clocksource_init(np);
#endif
	npcm750_clockevents_init(irq,np);

	pr_info("npcm750_timer_init\n");

	return 0;
}

TIMER_OF_DECLARE(npcm750, "nuvoton,npcm750-timer", npcm750_timer_init);

