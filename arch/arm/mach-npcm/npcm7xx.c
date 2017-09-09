/*
 * Copyright (c) 2017 Nuvoton Technology corporation.
 * Copyright 2017 Google, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <asm/mach/arch.h>
#include <asm/mach-types.h>
#include <asm/mach/map.h>
#include <asm/hardware/cache-l2x0.h>

#define NPCM7XX_AUX_VAL (L310_AUX_CTRL_INSTR_PREFETCH |			       \
			 L310_AUX_CTRL_DATA_PREFETCH |			       \
			 L310_AUX_CTRL_NS_LOCKDOWN |			       \
			 L310_AUX_CTRL_CACHE_REPLACE_RR |		       \
			 L2C_AUX_CTRL_SHARED_OVERRIDE |			       \
			 L310_AUX_CTRL_FULL_LINE_ZERO)

static const char *const npcm7xx_dt_match[] = {
	"nuvoton,npcm750",
	NULL
};

DT_MACHINE_START(NPCM7XX_DT, "NPCMX50 Chip family")
	.atag_offset	= 0x100,
	.dt_compat	= npcm7xx_dt_match,
	.l2c_aux_val	= NPCM7XX_AUX_VAL,
	.l2c_aux_mask	= ~NPCM7XX_AUX_VAL,
MACHINE_END
