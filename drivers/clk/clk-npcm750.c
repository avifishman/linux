/*
 * drivers/clk/clk-npcm750.c
 *
 * Provides clock implementations for clk drivers.
 * all the clocks are set on boot, so this driver allow only reading of current clk settings directly from the module.
 *
 * Copyright (C) 2016 Nuvoton Technologies tali.perry@nuvoton.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/clk-provider.h>
#include <linux/regmap.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/clk/nuvoton.h>
#include <linux/mfd/syscon.h>
#include <linux/io.h>

#include <dt-bindings/clock/nuvoton,npcm7xx-clks.h>
#include "clk-npcm750.h"


// #include <mach/hal.h>
#define NPCMX50_SD_NUM_OF_MODULES               2
#define NPCMX50_EXT_CLOCK_FREQUENCY_MHZ     25
#define NPCMX50_EXT_CLOCK_FREQUENCY_KHZ     (NPCMX50_EXT_CLOCK_FREQUENCY_MHZ*_1KHz_)
#define NPCMX50_EXT_CLOCK_FREQUENCY_HZ      (NPCMX50_EXT_CLOCK_FREQUENCY_MHZ*_1MHz_)
#include <asm/cp15.h>
//#include <defs.h>

static void __iomem *clk_base;

#define _1Hz_           1UL
#define _1KHz_          (1000 * _1Hz_ )
#define _1MHz_          (1000 * _1KHz_)
#define _1GHz_          (1000 * _1MHz_)

#define ICACHE_SAVE_DISABLE(var)    {var = get_cr(); set_cr( var & (~0x1000)); }
#define ICACHE_RESTORE(var)         set_cr(var)

typedef struct bit_field {
	u32 offset;
	u32 size;
} bit_field_t;

#define  CLKEN1         (clk_base + 0x00)
#define  CLKEN2         (clk_base + 0x28)
#define  CLKEN3         (clk_base + 0x30)
#define  CLKSEL         (clk_base + 0x04)
#define  CLKDIV1        (clk_base + 0x08)
#define  CLKDIV2        (clk_base + 0x2C)
#define  CLKDIV3        (clk_base + 0x58)
#define  PLLCON0        (clk_base + 0x0C)
#define  PLLCON1        (clk_base + 0x10)
#define  PLLCON2        (clk_base + 0x54)
#define  SWRSTR         (clk_base + 0x14)
#define  IRQWAKECON     (clk_base + 0x18)
#define  IRQWAKEFLAG    (clk_base + 0x1C)
#define  IPSRST1        (clk_base + 0x20)
#define  IPSRST2        (clk_base + 0x24)
#define  IPSRST3        (clk_base + 0x34)
#define  WD0RCR         (clk_base + 0x38)
#define  WD1RCR         (clk_base + 0x3C)
#define  WD2RCR         (clk_base + 0x40)
#define  SWRSTC1        (clk_base + 0x44)
#define  SWRSTC2        (clk_base + 0x48)
#define  SWRSTC3        (clk_base + 0x4C)
#define  SWRSTC4        (clk_base + 0x50)
#define  CORSTC         (clk_base + 0x5C)
#define  PLLCONG        (clk_base + 0x60)
#define  AHBCKFI        (clk_base + 0x64)
#define  SECCNT         (clk_base + 0x68)
#define  CNTR25M        (clk_base + 0x6C)

#define  CLKEN1_OFFSET          0x00
#define  CLKEN2_OFFSET          0x28
#define  CLKEN3_OFFSET          0x30
#define  CLKSEL_OFFSET          0x04
#define  CLKDIV1_OFFSET         0x08
#define  CLKDIV2_OFFSET         0x2C
#define  CLKDIV3_OFFSET         0x58
#define  PLLCON0_OFFSET         0x0C
#define  PLLCON1_OFFSET         0x10
#define  PLLCON2_OFFSET         0x54
#define  SWRSTR_OFFSET          0x14
#define  IRQWAKECON_OFFSET      0x18
#define  IRQWAKEFLAG_OFFSET     0x1C
#define  IPSRST1_OFFSET         0x20
#define  IPSRST2_OFFSET         0x24
#define  IPSRST3_OFFSET         0x34
#define  WD0RCR_OFFSET          0x38
#define  WD1RCR_OFFSET          0x3C
#define  WD2RCR_OFFSET          0x40
#define  SWRSTC1_OFFSET         0x44
#define  SWRSTC2_OFFSET         0x48
#define  SWRSTC3_OFFSET         0x4C
#define  SWRSTC4_OFFSET         0x50
#define  CORSTC_OFFSET          0x5C
#define  PLLCONG_OFFSET         0x60
#define  AHBCKFI_OFFSET         0x64
#define  SECCNT_OFFSET          0x68
#define  CNTR25M_OFFSET         0x6C

/******************************************************/
/*   Clock Enable 1 Register (CLKEN1)                 */
/******************************************************/
static const bit_field_t   CLKEN1_SMB1     = {  31, 1 };
static const bit_field_t   CLKEN1_SMB0     = {  30, 1 };
static const bit_field_t   CLKEN1_SMB7     = {  29, 1 };
static const bit_field_t   CLKEN1_SMB6     = {  28, 1 };
static const bit_field_t   CLKEN1_ADC      = {  27, 1 };
static const bit_field_t   CLKEN1_WDT      = {  26, 1 };
static const bit_field_t   CLKEN1_USBDEV3  = {  25, 1 };
static const bit_field_t   CLKEN1_USBDEV6  = {  24, 1 };
static const bit_field_t   CLKEN1_USBDEV5  = {  23, 1 };
static const bit_field_t   CLKEN1_USBDEV4  = {  22, 1 };
static const bit_field_t   CLKEN1_EMC2     = {  21, 1 };
static const bit_field_t   CLKEN1_TIMER5_9 = {  20, 1 };
static const bit_field_t   CLKEN1_TIMER0_4 = {  19, 1 };
static const bit_field_t   CLKEN1_PWM0     = {  18, 1 };
static const bit_field_t   CLKEN1_HUART    = {  17, 1 };
static const bit_field_t   CLKEN1_SMB5     = {  16, 1 };
static const bit_field_t   CLKEN1_SMB4     = {  15, 1 };
static const bit_field_t   CLKEN1_SMB3     = {  14, 1 };
static const bit_field_t   CLKEN1_SMB2     = {  13, 1 };
static const bit_field_t   CLKEN1_MC       = {  12, 1 };
static const bit_field_t   CLKEN1_UART01   = {  11, 1 };
static const bit_field_t   CLKEN1_AES      = {  10, 1 };
static const bit_field_t   CLKEN1_PECI     = {  9, 1  };
static const bit_field_t   CLKEN1_USBDEV2  = {  8, 1  };
static const bit_field_t   CLKEN1_UART23   = {  7, 1  };
static const bit_field_t   CLKEN1_EMC1     = {  6, 1  };
static const bit_field_t   CLKEN1_USBDEV1  = {  5, 1  };
static const bit_field_t   CLKEN1_SHM      = {  4, 1  };
static const bit_field_t   CLKEN1_GDMA0    = {  3, 1  };
static const bit_field_t   CLKEN1_KCS      = {  2, 1  };
static const bit_field_t   CLKEN1_SPI3     = {  1, 1  };
static const bit_field_t   CLKEN1_SPI0     = {  0, 1  };

/******************************************************/
/*   Clock Enable 2 Register (CLKEN2)                 */
/******************************************************/
static const bit_field_t   CLKEN2_CP        = {  31, 1  };
static const bit_field_t   CLKEN2_TOCK      = {  30, 1  };
static const bit_field_t   CLKEN2_GMAC1     = {  28, 1  };
static const bit_field_t   CLKEN2_USBIF     = {  27, 1  };
static const bit_field_t   CLKEN2_USBHOST   = {  26, 1  };
static const bit_field_t   CLKEN2_GMAC2     = {  25, 1  };
static const bit_field_t   CLKEN2_PSPI2     = {  23, 1  };
static const bit_field_t   CLKEN2_PSPI1     = {  22, 1  };
static const bit_field_t   CLKEN2_SIOX2     = {  19, 1  };
static const bit_field_t   CLKEN2_SIOX1     = {  18, 1  };
static const bit_field_t   CLKEN2_FUSE      = {  16, 1  };
static const bit_field_t   CLKEN2_VCD       = {  14, 1  };
static const bit_field_t   CLKEN2_ECE       = {  13, 1  };
static const bit_field_t   CLKEN2_VDMA      = {  12, 1  };
static const bit_field_t   CLKEN2_AHBPCIBRG = {  11, 1  };
static const bit_field_t   CLKEN2_GFXSYS    = {  10, 1  };
static const bit_field_t   CLKEN2_SDHC      = {  9, 1   };
static const bit_field_t   CLKEN2_MMC       = {  8, 1   };
static const bit_field_t   CLKEN2_MFT7_0    = {  0, 8   };

/******************************************************/
/*   Clock Enable 3 Register (CLKEN3)                 */
/******************************************************/
static const bit_field_t   CLKEN3_GPIOM7     = {  31, 1  };
static const bit_field_t   CLKEN3_GPIOM6     = {  30, 1  };
static const bit_field_t   CLKEN3_GPIOM5     = {  29, 1  };
static const bit_field_t   CLKEN3_GPIOM4     = {  28, 1  };
static const bit_field_t   CLKEN3_GPIOM3     = {  27, 1  };
static const bit_field_t   CLKEN3_GPIOM2     = {  26, 1  };
static const bit_field_t   CLKEN3_GPIOM1     = {  25, 1  };
static const bit_field_t   CLKEN3_GPIOM0     = {  24, 1  };
static const bit_field_t   CLKEN3_ESPI       = {  23, 1  };
static const bit_field_t   CLKEN3_SMB11      = {  22, 1  };
static const bit_field_t   CLKEN3_SMB10      = {  21, 1  };
static const bit_field_t   CLKEN3_SMB9       = {  20, 1  };
static const bit_field_t   CLKEN3_SMB8       = {  19, 1  };
static const bit_field_t   CLKEN3_SMB15      = {  18, 1  };
static const bit_field_t   CLKEN3_RNG        = {  17, 1  };
static const bit_field_t   CLKEN3_TIMER10_14 = {  16, 1  };
static const bit_field_t   CLKEN3_PCIERC     = {  15, 1  };
static const bit_field_t   CLKEN3_SECECC     = {  14, 1  };
static const bit_field_t   CLKEN3_SHA        = {  13, 1  };
static const bit_field_t   CLKEN3_SMB14      = {  12, 1  };
static const bit_field_t   CLKEN3_GDMA2      = {  11, 1  };
static const bit_field_t   CLKEN3_GDMA1      = {  10, 1  };
static const bit_field_t   CLKEN3_PCIMBX     = {  9, 1   };
static const bit_field_t   CLKEN3_USBDEV9    = {  7, 1   };
static const bit_field_t   CLKEN3_USBDEV8    = {  6, 1   };
static const bit_field_t   CLKEN3_USBDEV7    = {  5, 1   };
static const bit_field_t   CLKEN3_USBDEV0    = {  4, 1   };
static const bit_field_t   CLKEN3_SMB13      = {  3, 1   };
static const bit_field_t   CLKEN3_SPIX       = {  2, 1   };
static const bit_field_t   CLKEN3_SMB12      = {  1, 1   };
static const bit_field_t   CLKEN3_PWM1       = {  0, 1   };

/******************************************************/
/*   Clock Select Register (CLKSEL)                   */
/******************************************************/
static const bit_field_t   CLKSEL_DVCSSEL   = {  23, 2  };
static const bit_field_t   CLKSEL_GFXMSEL   = {  21, 2  };
static const bit_field_t   CLKSEL_CLKOUTSEL = {  18, 3  };
static const bit_field_t   CLKSEL_GFXCKSEL  = {  16, 2  };
static const bit_field_t   CLKSEL_TIMCKSEL  = {  14, 2  };
static const bit_field_t   CLKSEL_MCCKSEL   = {  12, 2  };
static const bit_field_t   CLKSEL_SUCKSEL   = {  10, 2  };
static const bit_field_t   CLKSEL_UARTCKSEL = {  8, 2   };
static const bit_field_t   CLKSEL_SDCKSEL   = {  6, 2   };
static const bit_field_t   CLKSEL_PIXCKSEL  = {  4, 2   };
static const bit_field_t   CLKSEL_GPRFSEL   = {  2, 2   };
static const bit_field_t   CLKSEL_CPUCKSEL  = {  0, 2   };

/******************************************************/
/*   Clock Divider Control Register 1 (CLKDIV1)       */
/******************************************************/
static const bit_field_t   CLKDIV1_ADCCKDIV  = {  28, 3  };
static const bit_field_t   CLKDIV1_CLK4DIV   = {  26, 2  };
static const bit_field_t   CLKDIV1_TIMCKDIV  = {  21, 5  };
static const bit_field_t   CLKDIV1_UARTDIV   = {  16, 5  };
static const bit_field_t   CLKDIV1_MMCCKDIV  = {  11, 5  };
static const bit_field_t   CLKDIV1_AHB3CKDIV = {  6, 5   };
static const bit_field_t   CLKDIV1_PCICKDIV  = {  2, 4   };
static const bit_field_t   CLKDIV1_CLK2DIV   = {  0, 1   };

/******************************************************/
/*   Clock Divider Control Register 2 (CLKDIV2)       */
/******************************************************/
static const bit_field_t   CLKDIV2_APB4CKDIV = {  30, 2  };
static const bit_field_t   CLKDIV2_APB3CKDIV = {  28, 2  };
static const bit_field_t   CLKDIV2_APB2CKDIV = {  26, 2  };
static const bit_field_t   CLKDIV2_APB1CKDIV = {  24, 2  };
static const bit_field_t   CLKDIV2_APB5CKDIV = {  22, 2  };
static const bit_field_t   CLKDIV2_CLKOUTDIV = {  16, 5  };
static const bit_field_t   CLKDIV2_GFXCKDIV  = {  13, 3  };
static const bit_field_t   CLKDIV2_SUCKDIV   = {  8, 5   };
static const bit_field_t   CLKDIV2_SU48CKDIV = {  4, 4   };
static const bit_field_t   CLKDIV2_SD1CKDIV  = {  0, 4   };

/******************************************************/
/*   Clock Divider Control Register 3 (CLKDIV3)       */
/******************************************************/

static const bit_field_t   CLKDIV3_SPI0CKDV = {  6, 5  };
static const bit_field_t   CLKDIV3_SPIXCKDV = {  1, 5  };

/******************************************************/
/*   PLL Control Register 2 (PLLCON2)                 */
/******************************************************/
static const bit_field_t   PLLCONn_LOKI   = {   31, 1  };
static const bit_field_t   PLLCONn_LOKS   = {   30, 1  };
static const bit_field_t   PLLCONn_FBDV   = {   16, 12 };
static const bit_field_t   PLLCONn_OTDV2  = {   13, 3  };
static const bit_field_t   PLLCONn_PWDEN  = {   12, 1  };
static const bit_field_t   PLLCONn_OTDV1  = {   8, 3   };
static const bit_field_t   PLLCONn_INDV   = {   0, 6   };

/******************************************************/
/*   Software Reset Register (SWRSTR)                 */
/******************************************************/
static const bit_field_t   SWRSTR_SWRST4 = {  6, 1  };
static const bit_field_t   SWRSTR_SWRST3 = {  5, 1  };
static const bit_field_t   SWRSTR_SWRST2 = {  4, 1  };
static const bit_field_t   SWRSTR_SWRST1 = {  3, 1  };

/******************************************************/
/*   IRQ Wake-Up Control Register (IRQWAKECON)        */
/******************************************************/
static const bit_field_t   IRQWAKECON_IRQWAKEUPPOL = {  16, 16  };
static const bit_field_t   IRQWAKECON_IRQWAKEUPEN  = {  0, 16   };

/******************************************************/
/*   IRQ Wake-Up Flag Register (IRQWAKEFLAG)          */
/******************************************************/
static const bit_field_t   IRQWAKEFLAG_IRQWAKEFLAG = {  0, 16  };

/******************************************************/
/*   IP Software Reset Register 1 (IPSRST1)           */
/******************************************************/
static const bit_field_t   IPSRST1_SMB1    = {  31, 1  };
static const bit_field_t   IPSRST1_SMB0    = {  30, 1  };
static const bit_field_t   IPSRST1_SMB7    = {  29, 1  };
static const bit_field_t   IPSRST1_SMB6    = {  28, 1  };
static const bit_field_t   IPSRST1_ADC     = {  27, 1  };
static const bit_field_t   IPSRST1_USBDEV3 = {  25, 1  };
static const bit_field_t   IPSRST1_USBDEV6 = {  24, 1  };
static const bit_field_t   IPSRST1_USBDEV5 = {  23, 1  };
static const bit_field_t   IPSRST1_USBDEV4 = {  22, 1  };
static const bit_field_t   IPSRST1_EMC2    = {  21, 1  };
static const bit_field_t   IPSRST1_TIM5_9  = {  20, 1  };
static const bit_field_t   IPSRST1_TIM0_4  = {  19, 1  };
static const bit_field_t   IPSRST1_PWM     = {  18, 1  };
static const bit_field_t   IPSRST1_SMB5    = {  16, 1  };
static const bit_field_t   IPSRST1_SMB4    = {  15, 1  };
static const bit_field_t   IPSRST1_SMB3    = {  14, 1  };
static const bit_field_t   IPSRST1_SMB2    = {  13, 1  };
static const bit_field_t   IPSRST1_MC      = {  12, 1  };
static const bit_field_t   IPSRST1_UART01  = {  11, 1  };
static const bit_field_t   IPSRST1_AES     = {  10, 1  };
static const bit_field_t   IPSRST1_PECI    = {  9, 1   };
static const bit_field_t   IPSRST1_USBDEV2 = {  8, 1   };
static const bit_field_t   IPSRST1_UART23  = {  7, 1   };
static const bit_field_t   IPSRST1_EMC1    = {  6, 1   };
static const bit_field_t   IPSRST1_USBDEV1 = {  5, 1   };
static const bit_field_t   IPSRST1_GDMA0   = {  3, 1   };
static const bit_field_t   IPSRST1_SPI3    = {  1, 1   };
static const bit_field_t   IPSRST1_SPI0    = {  0, 1   };

/******************************************************/
/*   IP Software Reset Register 2 (IPSRST2)           */
/******************************************************/
static const bit_field_t   IPSRST2_CP        = {  31, 1  };
static const bit_field_t   IPSRST2_GMAC1     = {  28, 1  };
static const bit_field_t   IPSRST2_USBHOST   = {  26, 1  };
static const bit_field_t   IPSRST2_GMAC2     = {  25, 1  };
static const bit_field_t   IPSRST2_PSPI2     = {  23, 1  };
static const bit_field_t   IPSRST2_PSPI1     = {  22, 1  };
static const bit_field_t   IPSRST2_SIOX2     = {  19, 1  };
static const bit_field_t   IPSRST2_SIOX1     = {  18, 1  };
static const bit_field_t   IPSRST2_OTP       = {  16, 1  };
static const bit_field_t   IPSRST2_VCD       = {  14, 1  };
static const bit_field_t   IPSRST2_ECE       = {  13, 1  };
static const bit_field_t   IPSRST2_VDMA      = {  12, 1  };
static const bit_field_t   IPSRST2_AHBPCIBRG = {  11, 1  };
static const bit_field_t   IPSRST2_GFXSYS    = {  10, 1  };
static const bit_field_t   IPSRST2_SDHC      = {  9, 1   };
static const bit_field_t   IPSRST2_MMC       = {  8, 1   };
static const bit_field_t   IPSRST2_MFT7_0    = {  0, 8   };

/******************************************************/
/*   IP Software Reset Register 3 (IPSRST3)           */
/******************************************************/
static const bit_field_t   IPSRST3_USBPHY2    = {  25, 1  };
static const bit_field_t   IPSRST3_USBPHY1    = {  24, 1  };
static const bit_field_t   IPSRST3_ESPI       = {  23, 1  };
static const bit_field_t   IPSRST3_SMB11      = {  22, 1  };
static const bit_field_t   IPSRST3_SMB10      = {  21, 1  };
static const bit_field_t   IPSRST3_SMB9       = {  20, 1  };
static const bit_field_t   IPSRST3_SMB8       = {  19, 1  };
static const bit_field_t   IPSRST3_SMB15      = {  18, 1  };
static const bit_field_t   IPSRST3_RNG        = {  17, 1  };
static const bit_field_t   IPSRST3_TIMER10_14 = {  16, 1  };
static const bit_field_t   IPSRST3_PCIERC     = {  15, 1  };
static const bit_field_t   IPSRST3_SECECC     = {  14, 1  };
static const bit_field_t   IPSRST3_SHA        = {  13, 1  };
static const bit_field_t   IPSRST3_SMB14      = {  12, 1  };
static const bit_field_t   IPSRST3_GDMA2      = {  11, 1  };
static const bit_field_t   IPSRST3_GDMA1      = {  10, 1  };
static const bit_field_t   IPSRST3_SPCIMBX    = {  9, 1   };
static const bit_field_t   IPSRST3_USBHUB     = {  8, 1   };
static const bit_field_t   IPSRST3_USBDEV9    = {  7, 1   };
static const bit_field_t   IPSRST3_USBDEV8    = {  6, 1   };
static const bit_field_t   IPSRST3_USBDEV7    = {  5, 1   };
static const bit_field_t   IPSRST3_USBDEV0    = {  4, 1   };
static const bit_field_t   IPSRST3_SMB13      = {  3, 1   };
static const bit_field_t   IPSRST3_SPIX       = {  2, 1   };
static const bit_field_t   IPSRST3_SMB12      = {  1, 1   };
static const bit_field_t   IPSRST3_PWM1       = {  0, 1   };

/******************************************************/
/*   Watchdog 0 Reset Control Register (WD0RCR)       */
/******************************************************/
static const bit_field_t   WD0RCR_LPCESPI  = {  31, 1  };
static const bit_field_t   WD0RCR_PCIE     = {  30, 1  };
static const bit_field_t   WD0RCR_SHMKCS   = {  29, 1  };
static const bit_field_t   WD0RCR_PWM      = {  28, 1  };
static const bit_field_t   WD0RCR_SPER     = {  27, 1  };
static const bit_field_t   WD0RCR_SPI      = {  26, 1  };
static const bit_field_t   WD0RCR_SIOX2    = {  25, 1  };
static const bit_field_t   WD0RCR_SIOX1    = {  24, 1  };
static const bit_field_t   WD0RCR_GPIOM7   = {  23, 1  };
static const bit_field_t   WD0RCR_GPIOM6   = {  22, 1  };
static const bit_field_t   WD0RCR_GPIOM5   = {  21, 1  };
static const bit_field_t   WD0RCR_GPIOM4   = {  20, 1  };
static const bit_field_t   WD0RCR_GPIOM3   = {  19, 1  };
static const bit_field_t   WD0RCR_GPIOM2   = {  18, 1  };
static const bit_field_t   WD0RCR_GPIOM1   = {  17, 1  };
static const bit_field_t   WD0RCR_GPIOM0   = {  16, 1  };
static const bit_field_t   WD0RCR_TIMER    = {  15, 1  };
static const bit_field_t   WD0RCR_PCIMBX   = {  14, 1  };
static const bit_field_t   WD0RCR_AHB2PCI  = {  13, 1  };
static const bit_field_t   WD0RCR_SD       = {  12, 1  };
static const bit_field_t   WD0RCR_MMC      = {  11, 1  };
static const bit_field_t   WD0RCR_DMA      = {  10, 1  };
static const bit_field_t   WD0RCR_USBHST   = {  9, 1   };
static const bit_field_t   WD0RCR_USBDEV   = {  8, 1   };
static const bit_field_t   WD0RCR_ETH      = {  7, 1   };
static const bit_field_t   WD0RCR_CLKS     = {  6, 1   };
static const bit_field_t   WD0RCR_MC       = {  5, 1   };
static const bit_field_t   WD0RCR_RV       = {  4, 1   };
static const bit_field_t   WD0RCR_SEC      = {  3, 1   };
static const bit_field_t   WD0RCR_CP       = {  2, 1   };
static const bit_field_t   WD0RCR_A9DBG    = {  1, 1   };
static const bit_field_t   WD0RCR_CA9C     = {  0, 1   };

static const bit_field_t   PLLCONG_LOKI    = {  31, 1  };
static const bit_field_t   PLLCONG_LOKS    = {  30, 1  };
static const bit_field_t   PLLCONG_GPLLTST = {  29, 1  };
static const bit_field_t   PLLCONG_FBDV    = {  16, 12 };
static const bit_field_t   PLLCONG_OTDV2   = {  13, 3  };
static const bit_field_t   PLLCONG_PWDEN   = {  12, 1  };
static const bit_field_t   PLLCONG_OTDV1   = {  8, 3   };
static const bit_field_t   PLLCONG_INDV    = {  0, 6   };

/********************************************************/
/*   AHB Clock Frequency Information Register (AHBCKFI) */
/********************************************************/
static const bit_field_t   AHBCKFI_TST1S       = { 31, 1 };
static const bit_field_t   AHBCKFI_AHB_CLK_FRQ = { 0, 8  };

/******************************************************/
/*   Seconds Counter Register (SECCNT)                */
/******************************************************/
static const bit_field_t   SECCNT_SEC_CNT = {  0, 32 };

/******************************************************/
/*   25M Counter Register (CNTR25M)                   */
/******************************************************/
static const bit_field_t   CNTR25M_COUNT = { 0, 25 };


#define CNTR25M_ACCURECY  NPCMX50_EXT_CLOCK_FREQUENCY_MHZ


#ifdef SET_REG_FIELD
#undef SET_REG_FIELD
#endif
/*---------------------------------------------------------------------------*/
/* Set field of a register / variable according to the field offset and size */
/*---------------------------------------------------------------------------*/
static inline void SET_REG_FIELD(unsigned char __iomem *mem,
        bit_field_t bit_field, u32 val)
{
	u32 tmp = ioread32(mem);
	tmp &= ~(((1 << bit_field.size) - 1) << bit_field.offset);
	tmp |= val << bit_field.offset;
	iowrite32(tmp, mem);
}

#ifdef SET_VAR_FIELD
#undef SET_VAR_FIELD
#endif
// bit_field should be of bit_field_t type
#define SET_VAR_FIELD(var, bit_field, value) {                 \
    typeof(var) tmp = var;                 		       \
    tmp &= ~(((1 << bit_field.size) - 1) << bit_field.offset); \
    tmp |= value << bit_field.offset; 			       \
    var = tmp;                                                 \
}

#ifdef READ_REG_FIELD
#undef READ_REG_FIELD
#endif
/*---------------------------------------------------------------------------*/
/* Get field of a register / variable according to the field offset and size */
/*---------------------------------------------------------------------------*/
static inline u32 READ_REG_FIELD(unsigned char __iomem *mem, bit_field_t bit_field)
{
	u32 tmp = ioread32(mem);
	tmp = tmp >> bit_field.offset;     // shift right the offset
	tmp &= (1 << bit_field.size) - 1;  // mask the size
	return tmp;
}

#ifdef READ_VAR_FIELD
#undef READ_VAR_FIELD
#endif
// bit_field should be of bit_field_t type
#define READ_VAR_FIELD(var, bit_field) ({ \
    typeof(var) tmp = var;           \
    tmp = tmp >> bit_field.offset;     /* shift right the offset */ \
    tmp &= (1 << bit_field.size) - 1;  /* mask the size */          \
    tmp;                                                     \
})

#ifdef MASK_FIELD
#undef MASK_FIELD
#endif
/*----------------------------------------------*/
/* Build a mask of a register / variable field  */
/*----------------------------------------------*/
// bit_field should be of bit_field_t type
#define MASK_FIELD(bit_field) \
    (((1 << bit_field.size) - 1) << bit_field.offset) /* mask the field size */

#ifdef BUILD_FIELD_VAL
#undef BUILD_FIELD_VAL
#endif
/*-----------------------------------------------------------------*/
/* Expand the value of the given field into its correct position   */
/*-----------------------------------------------------------------*/
// bit_field should be of bit_field_t type
#define BUILD_FIELD_VAL(bit_field, value)  \
    ((((1 << bit_field.size) - 1) & (value)) << bit_field.offset)


#ifdef SET_REG_MASK
#undef SET_REG_MASK
#endif
/*---------------------------------------------------------------------------*/
/* Set field of a register / variable according to the field offset and size */
/*---------------------------------------------------------------------------*/
static inline void SET_REG_MASK(unsigned char __iomem *mem, u32 val)
{
	iowrite32(ioread32(mem) | val, mem);
}

// #define CLK_DEBUG_MODULE 1
#ifdef CLK_DEBUG_MODULE
#define CLOCK_DEBUG(fmt,args...)   printk(fmt ,##args)
#else
#define CLOCK_DEBUG(fmt,args...)
#endif

#undef  CLK_VIRT_BASE_ADDR
#define CLK_VIRT_BASE_ADDR                  clk_base

extern void npcmx50_atomic_io_modify(void __iomem *reg, u32 mask, u32 set);

static struct regmap *clk_rst_regmap;

/*-------------------------*/
/* Spec predefined values  */
/*-------------------------*/

/*---------------------------*/
/* PLLCON 0 possible values: */
/*---------------------------*/
#define npcm750_clk_333MHZ_PLLCON0_REG_CFG    0x00A02403
#define npcm750_clk_500MHZ_PLLCON0_REG_CFG    0x00282201
#define npcm750_clk_600MHZ_PLLCON0_REG_CFG    0x00302201
#define npcm750_clk_666MHZ_PLLCON0_REG_CFG    0x00A02203
#define npcm750_clk_700MHZ_PLLCON0_REG_CFG    0x001C2101
#define npcm750_clk_720MHZ_PLLCON0_REG_CFG    0x00902105
#define npcm750_clk_750MHZ_PLLCON0_REG_CFG    0x001E2101
#define npcm750_clk_800MHZ_PLLCON0_REG_CFG    0x00202101
#define npcm750_clk_825MHZ_PLLCON0_REG_CFG    0x00212101
#define npcm750_clk_850MHZ_PLLCON0_REG_CFG    0x00222101
#define npcm750_clk_888MHZ_PLLCON0_REG_CFG    0x03782119
#define npcm750_clk_900MHZ_PLLCON0_REG_CFG    0x00242101
#define npcm750_clk_950MHZ_PLLCON0_REG_CFG    0x00262101
#define npcm750_clk_1000MHZ_PLLCON0_REG_CFG   0x00282101
#define npcm750_clk_1066MHZ_PLLCON0_REG_CFG   0x00802103

/*---------------------------------------------------------------------------*/
/* PLLCON 1 possible values (notice that NPCM750_PLL1 in Z2 has a divider /2 */
/* , so OTDV1 is smaller in half          				     */
/*---------------------------------------------------------------------------*/
#define npcm750_clk_333MHZ_PLLCON1_REG_CFG    0x00A02203


#define npcm750_clk_444MHZ_PLLCON1_REG_CFG       0x00A02303
#define npcm750_clk_500MHZ_PLLCON1_REG_CFG       0x00282101
#define npcm750_clk_600MHZ_PLLCON1_REG_CFG       0x00302101
#define npcm750_clk_666MHZ_PLLCON1_REG_CFG_Z1    0x00A02203
#define npcm750_clk_666MHZ_PLLCON1_REG_CFG       0x00A02103

#define npcm750_clk_700MHZ_PLLCON1_REG_CFG    0x00382101
#define npcm750_clk_720MHZ_PLLCON1_REG_CFG    0x01202105
#define npcm750_clk_750MHZ_PLLCON1_REG_CFG    0x003C2101
#define npcm750_clk_800MHZ_PLLCON1_REG_CFG    0x00402101
#define npcm750_clk_825MHZ_PLLCON1_REG_CFG    0x00422101
#define npcm750_clk_850MHZ_PLLCON1_REG_CFG    0x00442101
#define npcm750_clk_900MHZ_PLLCON1_REG_CFG    0x00482101
#define npcm750_clk_950MHZ_PLLCON1_REG_CFG    0x004C2101
#define npcm750_clk_1000MHZ_PLLCON1_REG_CFG   0x00502101
#define npcm750_clk_1066MHZ_PLLCON1_REG_CFG   0x01002103

#define npcm750_clk_800MHZ_PLLCON0_REG_CFG_BB  0x00402201
/*--------------------------*/
/* PLLCON 2 possible values */
/*--------------------------*/
#define npcm750_clk_960MHZ_PLLCON2_REG_CFG    0x00C02105


#define LOK_TIMEOUT  100000  /* 4ms if 25 MHz */


/*-------------------*/
/* Local definitions */
/*-------------------*/
/* SD Clock Target frequency */
#define SU60_DESIRED_FREQUENCY      60  // MHz (dont use _1MHz_)
#define SU_DESIRED_FREQUENCY        30  // MHz (dont use _1MHz_)


static const struct of_device_id npcm750clk_match_table[];
struct clk_core;


/*-----------------*/
/* Local Functions */
/*-----------------*/
static void npcm750_clk_EnableGMACClock(u32 ethNum, bool bEN);
static u32 npcm750_clk_EnableSDClock(u32 sdNum, bool bEN);
static u32 npcm750_clk_CalculatePLLFrequency(u32 pllVal);
static u32 npcm750_clk_GetPll0Freq(void);
static u32 npcm750_clk_GetPll1Freq(void);
static u32 npcm750_clk_GetPll2Freq(void);
static u32 npcm750_clk_GetMemoryFreq(void);
static u32 npcm750_clk_GetCPUFreq(void);
static u32 npcm750_clk_GetSPIFreq(u32 spi);
static u32 npcm750_clk_GetAPBFreq(NPCM750_APB_CLK apb);
static u32 npcm750_clk_Get_AHB_Freq(void);
static u32 npcm750_clk_Get_AXI_Freq(void);
static u32 npcm750_clk_GetSDFreq(u32 sdNum);
static u32 npcm750_clk_GetUartFreq(void);
static u32 npcm750_clk_GetPLL0toAPBdivisor(NPCM750_APB_CLK apb);
static u32 npcm750_clk_Delay_Cycles(u32 cycles);
static int npcm750_clk_GetUSBClock(void);
static u32 npcm750_clk_GetTimerFreq(void);
static unsigned long npcm750_clk_recalc(struct clk_hw *hw, unsigned long parent_rate);
static int npcm750_clk_enable_disable(struct clk_hw *hw, bool bEN);
static int npcm750_clk_enable(struct clk_hw *hw);
static void npcm750_clk_disable(struct clk_hw *hw);
static const struct clk_ops npcm750_clk_ops;
static struct npcm750_clk *npcm750clk_clocks[];
static void npcm750clk_setup(struct device_node *np);

/*--------------------------*/
/* Functions Implementation */
/*--------------------------*/
u32 npcm750_clk_ConfigureUartClock(void)
{
	u32 uart_clk; //Hz
	u32 uartDesiredFreq  = 24 * _1MHz_; //Hz

	u32 pllFreq = npcm750_clk_GetPll2Freq();
	u32 uartDiv = pllFreq / uartDesiredFreq;
	uart_clk = pllFreq / uartDiv;

	regmap_update_bits(clk_rst_regmap, CLKDIV1_OFFSET, (0x1F << 16),
		CLKDIV1_UART_DIV(uartDiv));
	regmap_update_bits(clk_rst_regmap, CLKSEL_OFFSET, (0x3 << 8),
		CLKSEL_UARTCKSEL_PLL2);

	npcm750_clk_Delay_Cycles(200);

	CLOCK_DEBUG("\t\tuartfreq is %d\n", uart_clk);

	return uart_clk;

}

static  u32  npcm750_clk_GetPLL0toAPBdivisor(NPCM750_APB_CLK apb)
{
	volatile u32 apb_divisor = 1;

	apb_divisor = apb_divisor * (READ_REG_FIELD(CLKDIV1,
		 CLKDIV1_CLK2DIV) + 1);
	apb_divisor = apb_divisor * (READ_REG_FIELD(CLKDIV1,
		 CLKDIV1_CLK4DIV) + 1);

	switch (apb) {
	case NPCM750_APB1:
		apb_divisor = apb_divisor * (1 << READ_REG_FIELD(CLKDIV2,
			 CLKDIV2_APB1CKDIV));
		break;
	case NPCM750_APB2:
		apb_divisor = apb_divisor * (1 << READ_REG_FIELD(CLKDIV2,
			 CLKDIV2_APB2CKDIV));
		break;
	case NPCM750_APB3:
		apb_divisor = apb_divisor * (1 << READ_REG_FIELD(CLKDIV2,
			 CLKDIV2_APB3CKDIV));
		break;
	case NPCM750_APB4:
		apb_divisor = apb_divisor * (1 << READ_REG_FIELD(CLKDIV2,
			 CLKDIV2_APB4CKDIV));
		break;
	case NPCM750_APB5:
		apb_divisor = apb_divisor * (1 << READ_REG_FIELD(CLKDIV2,
			 CLKDIV2_APB5CKDIV));
		break;
	case NPCM750_SPI0:
		apb_divisor = apb_divisor * (1 + READ_REG_FIELD(CLKDIV3,
			 CLKDIV3_SPI0CKDV));
		break;
	case NPCM750_SPI3:
		apb_divisor = apb_divisor * (1 + READ_REG_FIELD(CLKDIV1,
			 CLKDIV1_AHB3CKDIV));
		break;
	case NPCM750_SPIX:
		apb_divisor = apb_divisor * (1 + READ_REG_FIELD(CLKDIV1,
			 CLKDIV3_SPIXCKDV));
		break;
	default:
		apb_divisor = 0xFFFFFFFF;
		break;
	}

	return apb_divisor;
}

static u32 npcm750_clk_Delay_Cycles(u32 cycles)
{
	unsigned int            cacheState = 0;
	volatile unsigned int   i          = 0;
	volatile u32 iterations = 0;
	const unsigned int CYCLES_IN_ONE_ITERATION_CACHE_DISABLED  =   145;

	iterations = cycles / CYCLES_IN_ONE_ITERATION_CACHE_DISABLED + 1;

	/*-----------------------*/
	/* The actual wait loop: */
	/*-----------------------*/
	ICACHE_SAVE_DISABLE(cacheState);
	for (i = 0; i < iterations; i++);
	ICACHE_RESTORE(cacheState);

	return iterations;
}

void  npcm750_clk_GetTimeStamp(u32 time_quad[2])
{
	u32 Seconds;
	u32 RefClocks;

	do {
		Seconds = ioread32(clk_base + SECCNT_OFFSET);
		RefClocks = ioread32(clk_base + CNTR25M_OFFSET);
	} while (ioread32(clk_base + SECCNT_OFFSET) != Seconds);

	time_quad[0] = RefClocks;
	time_quad[1] = Seconds;
}
EXPORT_SYMBOL(npcm750_clk_GetTimeStamp);

static u32 npcm750_clk_Delay_MicroSec(u32 microSec)
{
	u32 iUsCnt1[2], iUsCnt2[2];
	u32 delay;  // Acctual delay generated by FW
	u32 minimum_delay = (microSec * NPCMX50_EXT_CLOCK_FREQUENCY_MHZ)
				+ CNTR25M_ACCURECY;

	npcm750_clk_GetTimeStamp(iUsCnt1);

	do {
		npcm750_clk_GetTimeStamp(iUsCnt2);
		delay =  ((NPCMX50_EXT_CLOCK_FREQUENCY_MHZ * _1MHz_) * (iUsCnt2[1] - iUsCnt1[1]))
			 + (iUsCnt2[0] - iUsCnt1[0]);
	}while (delay < minimum_delay);

	return (u32)(delay / NPCMX50_EXT_CLOCK_FREQUENCY_MHZ);
}

u32 npcm750_clk_Delay_Since(u32 microSecDelay, u32 t0_time[2])
{

	u32 iUsCnt2[2];
	u32 timeElapsedSince;
	u32 minimum_delay = (microSecDelay * NPCMX50_EXT_CLOCK_FREQUENCY_MHZ)
				+ CNTR25M_ACCURECY;

	do {
		npcm750_clk_GetTimeStamp(iUsCnt2);
		timeElapsedSince =  ((NPCMX50_EXT_CLOCK_FREQUENCY_MHZ * _1MHz_) * (iUsCnt2[1] - t0_time[1]))
				    + (iUsCnt2[0] - t0_time[0]);
	}while (timeElapsedSince < minimum_delay);

	return (u32)(timeElapsedSince / NPCMX50_EXT_CLOCK_FREQUENCY_MHZ);
}

static  void npcm750_clk_EnableEMCClock(u32 ethNum, bool bEN)
{
	if (ethNum == 0) {
		regmap_update_bits(clk_rst_regmap, CLKEN1_OFFSET,
			 (0x1 << 6), bEN);

	} else if (ethNum == 1) {
		regmap_update_bits(clk_rst_regmap, CLKEN1_OFFSET,
			 (0x1 << 21), bEN);

	}
}

static  void npcm750_clk_EnableGMACClock(u32 ethNum, bool bEN)
{
	if (ethNum == 2) {
		regmap_update_bits(clk_rst_regmap, CLKEN2_OFFSET,
			(0x1 << 28), bEN);
	} else if (ethNum == 3) {
		regmap_update_bits(clk_rst_regmap, CLKEN2_OFFSET,
			(0x1 << 25), bEN);
	}
}

static  u32 npcm750_clk_EnableSDClock(u32 sdNum, bool bEN)
{
	if (sdNum >= NPCMX50_SD_NUM_OF_MODULES) {
		return -EINVAL;
	}

	if (sdNum == NPCM750_CLK_SD) {
		regmap_update_bits(clk_rst_regmap, CLKEN2_OFFSET,
			 (0x1 << 9), bEN);
	} else if (sdNum == NPCM750_CLK_MMC) {
		regmap_update_bits(clk_rst_regmap, CLKEN2_OFFSET,
			 (0x1 << 8), bEN);
	}

	return 0;
}

#ifdef _RESET_DEVICE_TREE_SUPPORT_TODO_

static void npcm750_clk_ResetEMC(u32 deviceNum)
{
	if (deviceNum == 0) {
		regmap_update_bits(clk_rst_regmap, IPSRST1_OFFSET,
			 (0x1 << 6), 1);
		regmap_update_bits(clk_rst_regmap, IPSRST1_OFFSET,
			 (0x1 << 6), 0);
	} else if (deviceNum == 1) {
		regmap_update_bits(clk_rst_regmap, IPSRST1_OFFSET,
			 (0x1 << 21), 1);
		regmap_update_bits(clk_rst_regmap, IPSRST1_OFFSET,
			 (0x1 << 21), 0);
	}
}

static void npcm750_clk_ResetFIU(void)
{
	regmap_update_bits(clk_rst_regmap, IPSRST1_OFFSET, (0x1 << 0), 1);
	regmap_update_bits(clk_rst_regmap, IPSRST1_OFFSET, (0x1 << 0), 0);
	regmap_update_bits(clk_rst_regmap, IPSRST1_OFFSET, (0x1 << 1), 1);
	regmap_update_bits(clk_rst_regmap, IPSRST1_OFFSET, (0x1 << 1), 0);
	regmap_update_bits(clk_rst_regmap, IPSRST1_OFFSET, (0x1 << 2), 1);
	regmap_update_bits(clk_rst_regmap, IPSRST1_OFFSET, (0x1 << 2), 0);
}

static void npcm750_clk_ResetUART(u32 deviceNum)
{
	regmap_update_bits(clk_rst_regmap, IPSRST1_OFFSET, (0x1 << 11), 1);
	regmap_update_bits(clk_rst_regmap, IPSRST1_OFFSET, (0x1 << 11), 0);
	regmap_update_bits(clk_rst_regmap, IPSRST1_OFFSET, (0x1 << 7), 1);
	regmap_update_bits(clk_rst_regmap, IPSRST1_OFFSET, (0x1 << 7), 0);
}

static void npcm750_clk_ResetAES(void)
{
	regmap_update_bits(clk_rst_regmap, IPSRST1_OFFSET, (0x1 << 10), 1);
	regmap_update_bits(clk_rst_regmap, IPSRST1_OFFSET, (0x1 << 10), 0);
}

static void npcm750_clk_ResetMC(void)
{
	regmap_update_bits(clk_rst_regmap, IPSRST1_OFFSET, (0x1 << 12), 1);
	npcm750_clk_Delay_MicroSec(100);

	regmap_update_bits(clk_rst_regmap, IPSRST1_OFFSET, (0x1 << 12), 0);
	npcm750_clk_Delay_MicroSec(100);

	regmap_update_bits(clk_rst_regmap, IPSRST2_OFFSET, (0x1 << 19), 0);
}

static void npcm750_clk_ResetTIMER(u32 deviceNum)
{
	if (deviceNum <= 4) {
		regmap_update_bits(clk_rst_regmap, IPSRST1_OFFSET,
			 (0x1 << 19), 1);
		regmap_update_bits(clk_rst_regmap, IPSRST1_OFFSET,
			 (0x1 << 19), 0);
	} else if (deviceNum <= 9) {
		regmap_update_bits(clk_rst_regmap, IPSRST1_OFFSET,
			 (0x1 << 20), 1);
		regmap_update_bits(clk_rst_regmap, IPSRST1_OFFSET,
			 (0x1 << 20), 0);
	} else {
		regmap_update_bits(clk_rst_regmap, IPSRST3_OFFSET,
			 (0x1 << 16), 1);
		regmap_update_bits(clk_rst_regmap, IPSRST3_OFFSET,
			 (0x1 << 16), 0);
		SET_REG_FIELD(IPSRST3, IPSRST3_TIMER10_14, 1);
		SET_REG_FIELD(IPSRST3, IPSRST3_TIMER10_14, 0);
	}
}

static int npcm750_clk_ResetSD(u32 sdNum)
{
	if (sdNum >= NPCMX50_SD_NUM_OF_MODULES) {
		return -2;
	}

	if (sdNum == NPCM750_CLK_SD) {
		regmap_update_bits(clk_rst_regmap, IPSRST2_OFFSET,
			 (0x1 << 9), 1);
		regmap_update_bits(clk_rst_regmap, IPSRST2_OFFSET,
			 (0x1 << 9), 0);
	} else if (sdNum == NPCM750_CLK_MMC) {
		regmap_update_bits(clk_rst_regmap, IPSRST2_OFFSET,
			 (0x1 << 8), 1);
		regmap_update_bits(clk_rst_regmap, IPSRST2_OFFSET,
			 (0x1 << 8), 0);
	}

	return 0;
}

static void npcm750_clk_ResetGMAC(u32 deviceNum)
{
	if (deviceNum == 2) {
		regmap_update_bits(clk_rst_regmap, IPSRST2_OFFSET,
			 (0x1 << 28), 1);
		regmap_update_bits(clk_rst_regmap, IPSRST2_OFFSET,
			 (0x1 << 928), 0);
	} else if (deviceNum == 3) {
		regmap_update_bits(clk_rst_regmap, IPSRST2_OFFSET,
			 (0x1 << 25), 1);
		regmap_update_bits(clk_rst_regmap, IPSRST2_OFFSET,
			 (0x1 << 25), 0);
	}
}

static int npcm750_clk_ResetPSPI(u32 deviceNum)
{
	if (deviceNum >= PSPI_NUM_OF_MODULES) {
		return -2;
	}

	if (deviceNum == PSPI1_DEV) {
		regmap_update_bits(clk_rst_regmap, IPSRST2_OFFSET,
			 (0x1 << 22), 1);
		regmap_update_bits(clk_rst_regmap, IPSRST2_OFFSET,
			 (0x1 << 22), 0);
	} else if (deviceNum == PSPI2_DEV) {
		regmap_update_bits(clk_rst_regmap, IPSRST2_OFFSET,
			 (0x1 << 23), 1);
		regmap_update_bits(clk_rst_regmap, IPSRST2_OFFSET,
			 (0x1 << 923), 0);
	}

	return 0;
}
#endif // _RESET_DEVICE_TREE_SUPPORT_TODO_

static u32  npcm750_clk_CalculatePLLFrequency(u32 pllVal)
{
	u32  FIN         = NPCMX50_EXT_CLOCK_FREQUENCY_KHZ;
	u32  FOUT        = 0;
	u32  NR          = 0;
	u32  NF          = 0;
	u32  NO          = 0;

	NR = READ_VAR_FIELD(pllVal, PLLCONn_INDV);
	NF = READ_VAR_FIELD(pllVal, PLLCONn_FBDV);
	NO = (READ_VAR_FIELD(pllVal, PLLCONn_OTDV1)) *
		(READ_VAR_FIELD(pllVal, PLLCONn_OTDV2));

	FOUT = ((10 * FIN * NF) / (NO * NR));

	/* Returning value in Hertz */
	return  FOUT * 100;

}

static u32  npcm750_clk_GetPll0Freq(void)
{
	u32  pllVal      = 0;

	pllVal = ioread32(clk_base + PLLCON0_OFFSET);
	return npcm750_clk_CalculatePLLFrequency(pllVal);
}

static u32  npcm750_clk_GetPll1Freq(void)
{
	u32  pllVal      = 0;

	pllVal = ioread32(clk_base + PLLCON1_OFFSET);
	return (npcm750_clk_CalculatePLLFrequency(pllVal) / 2);
}

static u32  npcm750_clk_GetPll2Freq(void)
{
	u32  pllVal      = 0;

	pllVal = ioread32(clk_base + PLLCON2_OFFSET);
	return (npcm750_clk_CalculatePLLFrequency(pllVal) / 2);
}

static u32  npcm750_clk_GetMemoryFreq(void)
{
	u32  FOUT        = 0;

	/* Calculate CPU clock */
	if (READ_REG_FIELD(CLKSEL, CLKSEL_MCCKSEL) == CLKSEL_MCCKSEL_PLL1) {
		FOUT = npcm750_clk_GetPll1Freq();
	} else if (READ_REG_FIELD(CLKSEL, CLKSEL_MCCKSEL) ==
		   CLKSEL_MCCKSEL_CLKREF) {
		FOUT = NPCMX50_EXT_CLOCK_FREQUENCY_MHZ;
	} else {
		FOUT = NPCMX50_EXT_CLOCK_FREQUENCY_MHZ;
	}

	return FOUT;
}

static u32  npcm750_clk_GetCPUFreq(void)
{
	u32  FOUT        = 0;

	if (READ_REG_FIELD(CLKSEL, CLKSEL_CPUCKSEL) == CLKSEL_CPUCKSEL_PLL0) {
		FOUT = npcm750_clk_GetPll0Freq();
	} else if (READ_REG_FIELD(CLKSEL, CLKSEL_CPUCKSEL) ==
		   CLKSEL_CPUCKSEL_PLL1) {
		FOUT = npcm750_clk_GetPll1Freq();
	} else if (READ_REG_FIELD(CLKSEL, CLKSEL_CPUCKSEL) ==
		   CLKSEL_CPUCKSEL_CLKREF) {
		FOUT = NPCMX50_EXT_CLOCK_FREQUENCY_MHZ;
	} else {
		FOUT = NPCMX50_EXT_CLOCK_FREQUENCY_MHZ;
	}

	return FOUT;
}

static u32 npcm750_clk_GetSPIFreq(u32 spi)
{
	return  npcm750_clk_GetCPUFreq() / npcm750_clk_GetPLL0toAPBdivisor(spi);

}

static u32 npcm750_clk_GetAPBFreq(NPCM750_APB_CLK apb)
{
	if ((apb > NPCM750_APB5) && (apb < NPCM750_APB1))
		return -1;

	return  npcm750_clk_GetCPUFreq() / npcm750_clk_GetPLL0toAPBdivisor(apb);
}

static u32 npcm750_clk_Get_AHB_Freq(void)
{
	u32  clk2Div = 0;
	u32  clk4Div = 0;

	clk2Div = (READ_REG_FIELD(CLKDIV1, CLKDIV1_CLK2DIV) + 1);
	clk4Div = (READ_REG_FIELD(CLKDIV1, CLKDIV1_CLK4DIV) + 1);

	return  (npcm750_clk_GetCPUFreq() / (clk2Div * clk4Div));
}

int npcm750_clk_Set_AHB_Freq(u32  cpFreq)
{
	u32  clkDiv = npcm750_clk_GetCPUFreq() / cpFreq;

	switch (clkDiv) {
	case 1:
		regmap_update_bits(clk_rst_regmap, CLKDIV1_OFFSET,
			 (0x3 << 26), CLKDIV1_CLK4DIV1);
		regmap_update_bits(clk_rst_regmap, CLKDIV1_OFFSET,
			 (0x1 << 0), CLKDIV1_CLK2DIV1);
		break;
	case 2:
		regmap_update_bits(clk_rst_regmap, CLKDIV1_OFFSET,
			 (0x3 << 26), CLKDIV1_CLK4DIV2);
		regmap_update_bits(clk_rst_regmap, CLKDIV1_OFFSET,
			 (0x1 << 0), CLKDIV1_CLK2DIV1);
		break;
	case 3:
		regmap_update_bits(clk_rst_regmap, CLKDIV1_OFFSET,
			 (0x3 << 26), CLKDIV1_CLK4DIV3);
		regmap_update_bits(clk_rst_regmap, CLKDIV1_OFFSET,
			 (0x1 << 0), CLKDIV1_CLK2DIV1);
		break;
	case 4:
		regmap_update_bits(clk_rst_regmap, CLKDIV1_OFFSET,
			 (0x3 << 26), CLKDIV1_CLK4DIV2);
		regmap_update_bits(clk_rst_regmap, CLKDIV1_OFFSET,
			 (0x1 << 0), CLKDIV1_CLK2DIV2);
		break;
	case 6:
		regmap_update_bits(clk_rst_regmap, CLKDIV1_OFFSET,
			 (0x3 << 26), CLKDIV1_CLK4DIV3);
		regmap_update_bits(clk_rst_regmap, CLKDIV1_OFFSET,
			 (0x1 << 0), CLKDIV1_CLK2DIV2);
		break;
	case 8:
		regmap_update_bits(clk_rst_regmap, CLKDIV1_OFFSET,
			 (0x3 << 26), CLKDIV1_CLK4DIV4);
		regmap_update_bits(clk_rst_regmap, CLKDIV1_OFFSET,
			 (0x1 << 0), CLKDIV1_CLK2DIV2);
		break;
	default:
		{
			return -1;
		}
	}

	npcm750_clk_Delay_MicroSec(20);

	return  0;
}

static u32 npcm750_clk_Get_AXI_Freq(void)
{
	u32  clk2Div = (READ_REG_FIELD(CLKDIV1, CLKDIV1_CLK2DIV) + 1);

	return  (npcm750_clk_GetCPUFreq() / (clk2Div));
}

static u32 npcm750_clk_GetSDFreq(u32 sdNum)
{
	u32  divider;
	u32  pll0Freq;   //In Hz

	pll0Freq = npcm750_clk_GetPll0Freq();
	if (sdNum == NPCM750_CLK_SD) {
		divider = 1 + READ_REG_FIELD(CLKDIV2, CLKDIV2_SD1CKDIV);
	} else if (sdNum == NPCM750_CLK_MMC) {
		divider = 1 + READ_REG_FIELD(CLKDIV1, CLKDIV1_MMCCKDIV);
	} else {
		return -2;
	}

	return (pll0Freq / divider);
}

static u32 npcm750_clk_GetUartFreq(void)
{
	return (npcm750_clk_GetPll2Freq() /
		(u32)(READ_REG_FIELD(CLKDIV1, CLKDIV1_UARTDIV) + 1));
}

void npcm750_clk_ConfigurePCIClock(void)
{
	u32 PLLCON1_l = 0;

	PLLCON1_l  = npcm750_clk_333MHZ_PLLCON1_REG_CFG;
	SET_VAR_FIELD(PLLCON1_l, PLLCONn_PWDEN, PLLCONn_PWDEN_NORMAL);
	iowrite32(PLLCON1_l, clk_base + PLLCON1_OFFSET);

	npcm750_clk_Delay_MicroSec(2);

	regmap_update_bits(clk_rst_regmap, CLKSEL_OFFSET,
		 (0x3 << 16), CLKSEL_GFXCKSEL_PLL2);

	npcm750_clk_Delay_MicroSec(2);

	regmap_update_bits(clk_rst_regmap, CLKDIV1_OFFSET,
		 (0x3 << 4), CLKDIV1_PCICK_DIV(5));

	npcm750_clk_Delay_MicroSec(2);
}

int npcm750_clk_ConfigureFIUClock(u8  fiu, u8 clkDiv)
{
	u32  ratio = 0;

	if ((clkDiv == 0) || (clkDiv == 0xFF))
		return -1;

	/* set SPIn clk div */
	switch (fiu) {
	case FIU_MODULE_0:
		regmap_update_bits(clk_rst_regmap, CLKDIV3_OFFSET,
			 (0x1F << 6), (CLKDIV3_SPI0CKDV_DIV(clkDiv) & 0x1F));
		break;
	case  FIU_MODULE_3:
		regmap_update_bits(clk_rst_regmap, CLKDIV1_OFFSET,
			 (0x1F << 6), (CLKDIV1_AHB3CK_DIV(clkDiv) & 0x1F));
		break;
	case  FIU_MODULE_X:
		regmap_update_bits(clk_rst_regmap, CLKDIV3_OFFSET,
			 (0x1F << 1), (CLKDIV3_SPIXCKDV_DIV(clkDiv) & 0x1F));
		break;
	default:
		return -2;
	}

	ratio = READ_REG_FIELD(CLKDIV1, CLKDIV1_CLK2DIV)
	 * READ_REG_FIELD(CLKDIV1, CLKDIV1_CLK4DIV) * clkDiv;

	/* delay is according to ratio. Take some buffer too */
	npcm750_clk_Delay_Cycles(50 * ratio);

	return 0;
}

u8 npcm750_clk_GetFIUClockDiv(u8  fiu)
{
	switch (fiu) {
	case FIU_MODULE_0:
		return READ_REG_FIELD(CLKDIV3, CLKDIV3_SPI0CKDV) + 1;
	case  FIU_MODULE_3:
		return READ_REG_FIELD(CLKDIV1, CLKDIV1_AHB3CKDIV) + 1;
	case  FIU_MODULE_X:
		return READ_REG_FIELD(CLKDIV3, CLKDIV3_SPIXCKDV) + 1;
	default:
		return 0xFF;
	}
}

static int  npcm750_clk_GetUSBClock(void)
{

	u32  choosenPllFreq;

	if (READ_REG_FIELD(CLKSEL, CLKSEL_SUCKSEL) == CLKSEL_SUCKSEL_PLL0) {
		choosenPllFreq = npcm750_clk_GetPll0Freq();
	} else if (READ_REG_FIELD(CLKSEL, CLKSEL_SUCKSEL)
		   == CLKSEL_SUCKSEL_PLL1) {
		choosenPllFreq = npcm750_clk_GetPll1Freq();
	} else if (READ_REG_FIELD(CLKSEL, CLKSEL_SUCKSEL)
		   == CLKSEL_SUCKSEL_PLL2) {
		choosenPllFreq = npcm750_clk_GetPll2Freq();
	} else {
		return -EINVAL;
	}

	return (choosenPllFreq /
		(READ_REG_FIELD(CLKDIV2, CLKDIV2_SU48CKDIV) + 1));
}

static u32  npcm750_clk_GetTimerFreq(void)
{

	u32  choosenPllFreq;

	if (READ_REG_FIELD(CLKSEL, CLKSEL_TIMCKSEL) == CLKSEL_TIMCKSEL_PLL0) {
		choosenPllFreq = npcm750_clk_GetPll0Freq();
	} else if (READ_REG_FIELD(CLKSEL, CLKSEL_TIMCKSEL) == CLKSEL_TIMCKSEL_PLL1) {
		choosenPllFreq = npcm750_clk_GetPll1Freq();
	} else if (READ_REG_FIELD(CLKSEL, CLKSEL_TIMCKSEL) == CLKSEL_TIMCKSEL_PLL2) {
		choosenPllFreq = npcm750_clk_GetPll2Freq();
	} else if (READ_REG_FIELD(CLKSEL, CLKSEL_TIMCKSEL) == CLKSEL_TIMCKSEL_CLKREF) {
		choosenPllFreq = NPCMX50_CLKREF;
	} else {
		return -EINVAL;
	}

	return (choosenPllFreq /  (READ_REG_FIELD(CLKDIV1, CLKDIV1_TIMCKDIV) + 1));
}

/**
 * struct npcm750_clk - Common struct to all npcm750 clocks.
 * @hw: clk_hw for the common clk framework
 * @regmap: Regmap for the clock control registers
 */
struct npcm750_clk {
	struct clk_hw hw;
};
#define to_npcm750_clk(_hw) container_of(_hw, struct npcm750_clk, hw)

/**
 * npcm750_clk_recalc - Calculate the PLL generated clock rate given the
 * parent clock rate.
 */
static unsigned long npcm750_clk_recalc(struct clk_hw *hw, unsigned long parent_rate)
{
	unsigned long ret_val = -ENOKEY;

	/* setting ADC clock divider to 2*/
	SET_REG_FIELD(CLKDIV1, CLKDIV1_ADCCKDIV, 0);

	if (0 == strcmp(__clk_get_name(hw->clk), "clk_pll0"))
		ret_val =  npcm750_clk_GetPll0Freq();
	if (0 == strcmp(__clk_get_name(hw->clk), "clk_pll1"))
		ret_val =  npcm750_clk_GetPll1Freq();
	if (0 == strcmp(__clk_get_name(hw->clk), "clk_pll2"))
		ret_val =  npcm750_clk_GetPll2Freq();
	if (0 == strcmp(__clk_get_name(hw->clk), "clk_gfx"))
		ret_val =  2 * npcm750_clk_GetPll2Freq();
	if (0 == strcmp(__clk_get_name(hw->clk), "clk_apb1"))
		ret_val =  npcm750_clk_GetAPBFreq(NPCM750_APB1);
	if (0 == strcmp(__clk_get_name(hw->clk), "clk_apb2"))
		ret_val =  npcm750_clk_GetAPBFreq(NPCM750_APB2);
	if (0 == strcmp(__clk_get_name(hw->clk), "clk_apb3"))
		ret_val =  npcm750_clk_GetAPBFreq(NPCM750_APB3);
	if (0 == strcmp(__clk_get_name(hw->clk), "clk_apb4"))
		ret_val =  npcm750_clk_GetAPBFreq(NPCM750_APB4);
	if (0 == strcmp(__clk_get_name(hw->clk), "clk_apb5"))
		ret_val =  npcm750_clk_GetAPBFreq(NPCM750_APB5);
	if (0 == strcmp(__clk_get_name(hw->clk), "clk_mc"))
		ret_val =  npcm750_clk_GetMemoryFreq();
	if (0 == strcmp(__clk_get_name(hw->clk), "clk_cpu"))
		ret_val =  npcm750_clk_GetCPUFreq();
	if (0 == strcmp(__clk_get_name(hw->clk), "clk_spi0"))
		ret_val =  npcm750_clk_GetSPIFreq(NPCM750_SPI0);
	if (0 == strcmp(__clk_get_name(hw->clk), "clk_spi3"))
		ret_val =  npcm750_clk_GetSPIFreq(NPCM750_SPI3);
	if (0 == strcmp(__clk_get_name(hw->clk), "clk_spix"))
		ret_val =  npcm750_clk_GetSPIFreq(NPCM750_SPIX);
	if (0 == strcmp(__clk_get_name(hw->clk), "clk_uart_core"))
		ret_val =  npcm750_clk_GetUartFreq();
	if (0 == strcmp(__clk_get_name(hw->clk), "clk_timer"))
		ret_val =  npcm750_clk_GetTimerFreq();
	if (0 == strcmp(__clk_get_name(hw->clk), "clk_host_uart"))
		ret_val =  npcm750_clk_GetUartFreq();
	if (0 == strcmp(__clk_get_name(hw->clk), "clk_mmc"))
		ret_val =  npcm750_clk_GetSDFreq(NPCM750_CLK_SD);
	if (0 == strcmp(__clk_get_name(hw->clk), "clk_sdhc"))
		ret_val =  npcm750_clk_GetSDFreq(NPCM750_CLK_MMC);
	if (0 == strcmp(__clk_get_name(hw->clk), "clk_adc"))
		ret_val =  npcm750_clk_GetTimerFreq() /
		(1 << READ_REG_FIELD(CLKDIV1, CLKDIV1_ADCCKDIV));
	if (0 == strcmp(__clk_get_name(hw->clk), "clk_gfx_mem"))
		ret_val =  npcm750_clk_GetPll0Freq();
	if (0 == strcmp(__clk_get_name(hw->clk), "clk_usb_bridge"))
		ret_val =  npcm750_clk_GetUSBClock();
	if (0 == strcmp(__clk_get_name(hw->clk), "clk_axi"))
		ret_val =  npcm750_clk_Get_AXI_Freq();
	if ((0 == strcmp(__clk_get_name(hw->clk), "clk_ahb")) ||
		(0 == strcmp(__clk_get_name(hw->clk), "clk_emc")) ||
		(0 == strcmp(__clk_get_name(hw->clk), "clk_gmac"))) {
		ret_val =  npcm750_clk_Get_AHB_Freq();
	}

	CLOCK_DEBUG("\t npcm750_clk_recalc for clock %s => %ld\n"
		, hw->init->name, ret_val);

	return ret_val;
}


int npcm750_clk_set_rate(struct clk_hw *hw, unsigned long rate,
	unsigned long parent_rate)
{
	CLOCK_DEBUG("\t npcm750_clk_set_rate for clock %s => %ld\n"
		, hw->init->name, rate);
	return 0;
}

static int npcm750_clk_enable_disable(struct clk_hw *hw, bool bEN)
{
	unsigned long ret_val = 0;

	if (0 == strcmp(__clk_get_name(hw->clk), "clk_emc")) {
		npcm750_clk_EnableEMCClock(0, bEN);
		npcm750_clk_EnableEMCClock(1, bEN);
		ret_val = 0;
	}
	if (0 == strcmp(__clk_get_name(hw->clk), "clk_gmac")) {
		npcm750_clk_EnableGMACClock(0, bEN);
		npcm750_clk_EnableGMACClock(1, bEN);
		ret_val = 0;
	}
	if (0 == strcmp(__clk_get_name(hw->clk), "clk_sdhc")) {
		npcm750_clk_EnableSDClock(NPCM750_CLK_SD, bEN);
		ret_val = 0;
	}
	if (0 == strcmp(__clk_get_name(hw->clk), "clk_mmc")) {
		npcm750_clk_EnableSDClock(NPCM750_CLK_MMC, bEN);
		ret_val = 0;
	}

	return ret_val;
}

static int npcm750_clk_enable(struct clk_hw *hw)
{
	CLOCK_DEBUG("\tnpcm750_clk_enable for clock %s\n"
		, __clk_get_name(hw->clk));
	return npcm750_clk_enable_disable(hw, true);
}


static void npcm750_clk_disable(struct clk_hw *hw)
{
	CLOCK_DEBUG("\tnpcm750_clk_disable for clock %s\n"
		, __clk_get_name(hw->clk));
	npcm750_clk_enable_disable(hw, false);
	return;
}


static const struct clk_ops npcm750_clk_ops = {
	.recalc_rate = npcm750_clk_recalc,
	//.set_rate    = npcm750_clk_set_rate,// not needed. setting done by BB.
	.enable      = npcm750_clk_enable,
	.disable     = npcm750_clk_disable,
};

/*
 * CLK definitions
 */

#define DECLARE_NUVOTON_CLK(_clk_name, _clk_name_str) \
	static struct npcm750_clk _clk_name = { \
	.hw.init = &(struct clk_init_data){ \
	.name = _clk_name_str, \
	.parent_names = (const char *[]){}, \
	.num_parents = 0, \
	.ops = &npcm750_clk_ops,	},};

DECLARE_NUVOTON_CLK(clk_pll0,         "clk_pll0");
DECLARE_NUVOTON_CLK(clk_pll1,         "clk_pll1");
DECLARE_NUVOTON_CLK(clk_pll2,         "clk_pll2");
DECLARE_NUVOTON_CLK(clk_gfx,         "clk_gfx");
DECLARE_NUVOTON_CLK(clk_apb1,         "clk_apb1");
DECLARE_NUVOTON_CLK(clk_apb2,         "clk_apb2");
DECLARE_NUVOTON_CLK(clk_apb3,         "clk_apb3");
DECLARE_NUVOTON_CLK(clk_apb4,         "clk_apb4");
DECLARE_NUVOTON_CLK(clk_apb5,         "clk_apb5");
DECLARE_NUVOTON_CLK(clk_mc,         "clk_mc");
DECLARE_NUVOTON_CLK(clk_cpu,         "clk_cpu");
DECLARE_NUVOTON_CLK(clk_spi0,         "clk_spi0");
DECLARE_NUVOTON_CLK(clk_spi3,	      "clk_spi3");
DECLARE_NUVOTON_CLK(clk_spix,	      "clk_spix");
DECLARE_NUVOTON_CLK(clk_uart_core,	  "clk_uart_core");
DECLARE_NUVOTON_CLK(clk_timer,		  "clk_timer");
DECLARE_NUVOTON_CLK(clk_host_uart,	  "clk_host_uart");
DECLARE_NUVOTON_CLK(clk_mmc,		  "clk_mmc");
DECLARE_NUVOTON_CLK(clk_sdhc,		  "clk_sdhc");
DECLARE_NUVOTON_CLK(clk_adc, 	      "clk_adc");
DECLARE_NUVOTON_CLK(clk_gfx_mem,	  "clk_gfx_mem");
DECLARE_NUVOTON_CLK(clk_usb_bridge,   "clk_usb_bridge");
DECLARE_NUVOTON_CLK(clk_axi,	      "clk_axi");
DECLARE_NUVOTON_CLK(clk_ahb,	      "clk_ahb");
DECLARE_NUVOTON_CLK(clk_emc,	      "clk_emc");
DECLARE_NUVOTON_CLK(clk_gmac,	      "clk_gmac");



/* Table of all supported clocks indexed by the clock identifiers from the
 * device tree binding
 */
static struct npcm750_clk *npcm750clk_clocks[] = {
	/*[npcm750_CLK_FAB_PLL]  = &clk_pll0.sclk,*/

	[NPCM7XX_CLK_PLL0	    ] = &clk_pll0,
	[NPCM7XX_CLK_PLL1	    ] = &clk_pll1,
	[NPCM7XX_CLK_PLL2	    ] = &clk_pll2,
	[NPCM7XX_CLK_GFX  	    ] = &clk_gfx,
	[NPCM7XX_CLK_APB1	    ] = &clk_apb1,
	[NPCM7XX_CLK_APB2	    ] = &clk_apb2,
	[NPCM7XX_CLK_APB3	    ] = &clk_apb3,
	[NPCM7XX_CLK_APB4	    ] = &clk_apb4,
	[NPCM7XX_CLK_APB5	    ] = &clk_apb5,
	[NPCM7XX_CLK_MC	        ] = &clk_mc,
	[NPCM7XX_CLK_CPU	    ] = &clk_cpu,
	[NPCM7XX_CLK_SPI0	    ] = &clk_spi0,
	[NPCM7XX_CLK_SPI3	    ] = &clk_spi3,
	[NPCM7XX_CLK_SPIX	    ] = &clk_spix,
	[NPCM7XX_CLK_UART_CORE	] = &clk_uart_core,
	[NPCM7XX_CLK_TIMER		] = &clk_timer,
	[NPCM7XX_CLK_HOST_UART	] = &clk_host_uart,
	[NPCM7XX_CLK_MMC		] = &clk_mmc,
	[NPCM7XX_CLK_SDHC		] = &clk_sdhc,
	[NPCM7XX_CLK_ADC 	    ] = &clk_adc,
	[NPCM7XX_CLK_GFX_MEM 	] = &clk_gfx_mem,
	[NPCM7XX_CLK_USB_BRIDGE ] = &clk_usb_bridge,
	[NPCM7XX_CLK_AXI	    ] = &clk_axi,
	[NPCM7XX_CLK_AHB	    ] = &clk_ahb,
	[NPCM7XX_CLK_EMC	    ] = &clk_emc,
	[NPCM7XX_CLK_GMAC	    ] = &clk_gmac,
};

struct npcm750clk_priv {
	struct clk_onecell_data onecell;
	struct clk *clks[];
};

static const struct regmap_config npcm750_clk_config = {
	.reg_bits = 32,
	.val_bits = 32,
	.reg_stride = 4,
};

static void __init npcm750clk_setup(struct device_node *np)
{
	int i, ret;
	struct clk *clk;
	size_t num_clks;
	struct npcm750clk_priv *priv;

	CLOCK_DEBUG("npcm750clk_setup\n");

	clk_base = of_iomap(np, 0);
	if (IS_ERR(clk_base)) {
		printk("\tnuvoton_npcm750_clock_init: resource error\n");
		goto setup_err;
	}

	clk_rst_regmap = regmap_init_mmio(NULL, clk_base, &npcm750_clk_config);

	if (IS_ERR(clk_rst_regmap)) {
		pr_err("%s: failed to find nuvoton,npcm750-clk_rst\n"
			, __func__);
		goto setup_err;
	}

	num_clks = ARRAY_SIZE(npcm750clk_clocks);
	pr_info("\tnpcm750 clk: supporting %u clocks\n", num_clks);
	priv = kzalloc(sizeof(*priv) + sizeof(*priv->clks) * num_clks,
		GFP_KERNEL);
	if (!priv) {
		printk(" npcm750clk_setup: alloc error\n");
		ret = -ENOMEM;
		goto setup_err;
	}

	priv->onecell.clks = priv->clks;
	priv->onecell.clk_num = num_clks;

	/* Update each entry with the allocated regmap and register the clock
	* with the common clock framework
	*/
	for (i = 0; i < num_clks; i++) {
		CLOCK_DEBUG("\t register clk%d..(%s)\t"
			, i, npcm750clk_clocks[i]->hw.init->name);
		clk = clk_register(NULL, &npcm750clk_clocks[i]->hw);
		if (IS_ERR(clk)) {
			printk("\tnpcm750clk_setup: register clk error\n");
			ret = PTR_ERR(clk);
			goto setup_err;
		}
		priv->clks[i] = clk;
	}

	ret = of_clk_add_provider(np, of_clk_src_onecell_get, &priv->onecell);

	printk("npcm750 clk setup done\n");

	return;

setup_err:
	printk("npcm750clk_setup: failed to start. errno %d\n", ret);
	of_node_put(np);
	BUG();
}

CLK_OF_DECLARE(npcm750_clk, "nuvoton,npcm750-clk", npcm750clk_setup);

MODULE_DESCRIPTION("npcm750 clock driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:npcm750-clk");
