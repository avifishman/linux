/*
 * linux/arch/arm/mach-realview/headsmp.S
 *
 * Copyright (c) 2003 ARM Limited
 * Copyright 2017 Google, Inc.
 *  All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/linkage.h>
#include <linux/init.h>

.equ SVC_MODE, 0x13
.equ I_BIT, 0x80
.equ F_BIT, 0x40

ENTRY(npcm7xx_wakeup_z1)
	stmfd	sp!, {r0-r12, lr}
	ldr	r0, =0x01
	ldr	r1, =0x01
	ldr	r2, =0x01

	and	r3, r0, #0x0F /* Mask off unused bits of ID, and move to r3 */
	and	r1, r1, #0x0F /* Mask off unused bits of target_filter */
	and	r2, r2, #0x0F /* Mask off unused bits of filter_list */

	orr	r3, r3, r1, LSL #16 /* Combine ID and target_filter */
	orr	r3, r3, r2, LSL #24 /* and now the filter list */

	/* Get the address of the GIC */
	mrc	p15, 4, r0, c15, c0, 0 /* Read periph base address */
	add	r0, r0, #0x1F00 /* Add offset of the sgi_trigger reg */

	/* Write to the Software Generated Interrupt Register (ICDSGIR) */
	str	r3, [r0]

	ldmfd	sp!, {r0-r12, pc}
ENDPROC(npcm7xx_wakeup_z1)

ENTRY(v7_invalidate_l1_npcmX50)
	mov	r0, #0
	mcr	p15, 0, r0, c7, c5, 0 /* invalidate I cache */
	mcr	p15, 2, r0, c0, c0, 0
	mrc	p15, 1, r0, c0, c0, 0

	ldr	r1, =0x7fff
	and	r2, r1, r0, lsr #13

	ldr	r1, =0x3ff

	and	r3, r1, r0, lsr #3 /* NumWays - 1 */
	add	r2, r2, #1         /* NumSets */

	and	r0, r0, #0x7
	add	r0, r0, #4 /* SetShift */

	clz	r1, r3     /* WayShift */
	add	r4, r3, #1 /* NumWays */
1:	sub	r2, r2, #1 /* NumSets-- */
	mov	r3, r4     /* Temp = NumWays */
2:	subs	r3, r3, #1 /* Temp-- */
	mov	r5, r3, lsl r1
	mov	r6, r2, lsl r0
	/* Reg = (Temp << WayShift) | (NumSets << SetShift) */
	orr	r5, r5, r6
	mcr	p15, 0, r5, c7, c6, 2
	bgt	2b
	cmp	r2, #0
	bgt	1b
	dsb
	isb
	mov	pc, lr
ENDPROC(v7_invalidate_l1_npcmX50)

/*
 * MSM specific entry point for secondary CPUs.  This provides
 * a "holding pen" into which all secondary cores are held until we're
 * ready for them to initialise.
 */
ENTRY(npcm7xx_secondary_startup)
	msr	CPSR_c, #(SVC_MODE)

	bl	v7_invalidate_l1_npcmX50
	/* disable vector table remapping */
	mrc	p15, 0,r0, c1, c0, 0
	and	r0, #0xffffdfff
	mcr	p15, 0,r0, c1, c0, 0

#ifdef CONFIG_CACHE_L2X0
	/* Enable L1 & L2 prefetch + Zero line */
	mrc	p15, 0, r0, c1, c0, 1
	orr	r0, r0, #(7 << 1)
	mcr	p15, 0, r0, c1, c0, 1
#endif /* CONFIG_CACHE_L2X0 */

	mrc	p15, 0, r0, c0, c0, 5
	and	r0, r0, #15
	adr	r4, 1f
	ldmia	r4, {r5, r6}
	sub	r4, r4, r5
	add	r6, r6, r4
	str r3,[r2]

pen:	ldr	r7, [r6]
	cmp	r7, r0
	bne	pen

	/*
	 * we've been released from the holding pen: secondary_stack
	 * should now contain the SVC stack for this core
	 */
	b	secondary_startup
ENDPROC(npcm7xx_secondary_startup)

	.align
1:	.long	.
	.long	pen_release
