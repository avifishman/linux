/*
 * Copyright (C) 2002 ARM Ltd.
 * Copyright (C) 2008 STMicroelctronics.
 * Copyright (C) 2009 ST-Ericsson.
 * Copyright 2017 Google, Inc.
 *
 * This file is based on arm realview platform
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#define pr_fmt(fmt) "PLATSMP: " fmt

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/smp.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <asm/cacheflush.h>
#include <asm/smp.h>
#include <asm/smp_plat.h>
#include <asm/smp_scu.h>

#define NPCM7XX_SCRPAD_REG 0x13c

static void __iomem *gcr_base;
static void __iomem *scu_base;

/* This is called from headsmp.S to wakeup the secondary core */
extern void npcm7xx_secondary_startup(void);
extern void npcm7xx_wakeup_z1(void);

/*
 * Write pen_release in a way that is guaranteed to be visible to all
 * observers, irrespective of whether they're taking part in coherency
 * or not.  This is necessary for the hotplug code to work reliably.
 */
static void npcm7xx_write_pen_release(int val)
{
	pen_release = val;
	/* write to pen_release must be visible to all observers. */
	smp_wmb();
	__cpuc_flush_dcache_area((void *) &pen_release, sizeof(pen_release));
	outer_clean_range(__pa(&pen_release), __pa(&pen_release + 1));
}

static DEFINE_SPINLOCK(boot_lock);

static void npcm7xx_smp_secondary_init(unsigned int cpu)
{
	/*
	 * let the primary processor know we're out of the
	 * pen, then head off into the C entry point
	 */
	npcm7xx_write_pen_release(-1);

	/*
	 * Synchronise with the boot thread.
	 */
	spin_lock(&boot_lock);
	spin_unlock(&boot_lock);
}

static int npcm7xx_smp_boot_secondary(unsigned int cpu, struct task_struct *idle)
{
	unsigned long timeout;

	if (!gcr_base)
		return -EIO;

	/*
	 * set synchronisation state between this boot processor
	 * and the secondary one
	 */
	spin_lock(&boot_lock);

	/*
	 * The secondary processor is waiting to be released from
	 * the holding pen - release it, then wait for it to flag
	 * that it has been released by resetting pen_release.
	 */
	npcm7xx_write_pen_release(cpu_logical_map(cpu));
	iowrite32(virt_to_phys(npcm7xx_secondary_startup),
		  gcr_base + NPCM7XX_SCRPAD_REG);
	/* make npcm7xx_secondary_startup visible to all observers. */
	smp_rmb();

	arch_send_wakeup_ipi_mask(cpumask_of(cpu));
	timeout  = jiffies + (HZ * 1);
	while (time_before(jiffies, timeout)) {
		/* make sure we see any writes to pen_release. */
		smp_rmb();
		if (pen_release == -1)
			break;

		udelay(10);
	}

	/*
	 * now the secondary core is starting up let it run its
	 * calibrations, then wait for it to finish
	 */
	spin_unlock(&boot_lock);

	return pen_release != -1 ? -EIO : 0;
}


static void __init npcm7xx_wakeup_secondary(void)
{
	/*
	 * write the address of secondary startup into the backup ram register
	 * at offset 0x1FF4, then write the magic number 0xA1FEED01 to the
	 * backup ram register at offset 0x1FF0, which is what boot rom code
	 * is waiting for. This would wake up the secondary core from WFE
	 */
	iowrite32(virt_to_phys(npcm7xx_secondary_startup), gcr_base +
		  NPCM7XX_SCRPAD_REG);
	/* make sure npcm7xx_secondary_startup is seen by all observers. */
	smp_wmb();
	dsb_sev();

	/* make sure write buffer is drained */
	mb();
}

static void __init npcm7xx_smp_prepare_cpus(unsigned int max_cpus)
{
	struct device_node *gcr_np, *scu_np;

	gcr_np = of_find_compatible_node(NULL, NULL, "nuvoton,npcm750-gcr");
	if (!gcr_np) {
		pr_err("no gcr device node\n");
		return;
	}
	gcr_base = of_iomap(gcr_np, 0);
	if (!gcr_base) {
		pr_err("could not iomap gcr at: 0x%llx\n", gcr_base);
		return;
	}

	scu_np = of_find_compatible_node(NULL, NULL, "arm,cortex-a9-scu");
	if (!scu_np) {
		pr_err("no scu device node\n");
		return;
	}
	scu_base = of_iomap(scu_np, 0);
	if (!scu_base) {
		pr_err("could not iomap gcr at: 0x%llx\n", scu_base);
		return;
	}

	scu_enable(scu_base);
	npcm7xx_wakeup_secondary();
}

static struct smp_operations npcm7xx_smp_ops __initdata = {
	.smp_prepare_cpus = npcm7xx_smp_prepare_cpus,
	.smp_boot_secondary = npcm7xx_smp_boot_secondary,
	.smp_secondary_init = npcm7xx_smp_secondary_init,
};

CPU_METHOD_OF_DECLARE(npcm7xx_smp, "nuvoton,npcm7xx-smp", &npcm7xx_smp_ops);
