/*
 *  drivers\watchdog\npcm750_wdt_fiq_handler.S
 *
 *  Based on  linux/arch/arm/lib/floppydma.S
 *  Renamed and modified to work with 2.6 kernel by Matt Callow
 *  Copyright (C) 1995, 1996 Russell King
 *  Copyright (C) 2004 Pete Trapps
 *  Copyright (C) 2006 Matt Callow
 *  Copyright (C) 2010 Janusz Krzysztofik
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 */

#include <linux/linkage.h>
#include <asm/assembler.h>


#define WDOG_REG_WTCR		 0x1C        /* Watchdog register offset */
#define WDOG_BIT_WTIF		(0x01 << 3)  /* WTCR interrupt flag*/
#define WDOG_BIT_WTIE	        (0x01 << 6)  /* WTCR enable interrupt*/


/*
 * Register usage

 * r12 - virt. base addr
 * r13 - reg_val
 */

	.text
	.global npcm750_wdt_fiq_end

ENTRY(npcm750_wdt_fiq_start)
	@ FIQ intrrupt handler
	MRC p15,0,r0,c0,c0,5@ read Multiprocessor ID register
	BIC r0, #0xFFFFFFF0
	CMP r0,#0x0
	BEQ THIS_IS_CPU0
	b .     @ loop - all other cpus stuck here
THIS_IS_CPU0:

        ldr r13, [r12, #WDOG_REG_WTCR]
        tst r13, #WDOG_BIT_WTIF
	beq exit				@ none - spurious FIQ? exit

	ldr r13, [r12, #WDOG_REG_WTCR]
	bic r13, r13, #WDOG_BIT_WTIE            @ Disable watchdog interrupt
	str r13, [r12, #WDOG_REG_WTCR]
	
@@@@@@@@@@@@@@@@@@@@@@@@@@
@
@ FIQ - Kernel Dump 
@ Add Assembler Code here ...


@
@
@@@@@@@@@@@@@@@@@@@@@@@@@@

        b .     @ CPU0 - loop until HW watchdog reset 


exit:
	subs	pc, lr, #4			@ return from FIQ


npcm750_wdt_fiq_end:

/*
 * Check the size of the FIQ,
 * it cannot go beyond 0xffff0200, and is copied to 0xffff001c
 */
.if (npcm750_wdt_fiq_end - npcm750_wdt_fiq_start) > (0x200 - 0x1c)
	.err
.endif
