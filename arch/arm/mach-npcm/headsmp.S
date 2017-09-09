/*
 * Copyright 2017 Google, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/linkage.h>
#include <linux/init.h>
#include <asm/assembler.h>

ENTRY(npcm7xx_secondary_startup)
	safe_svcmode_maskall r0

	b	secondary_startup
ENDPROC(npcm7xx_secondary_startup)
