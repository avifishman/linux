/*
 * drivers/clk/clk-npcm750.h
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


#ifndef NPCM750_npcm750_clk_REGS_H
#define NPCM750_npcm750_clk_REGS_H

// #include "../../../Chips/chip.h"

#define  POLEG_CHRID                     0xA92750
#define  POLEG_VERSION_Z1                0x00
#define  POLEG_VERSION_Z2                0x04
#define  POLEG_VERSION_A1                0x10





/*---------------------------------------------------------------------------------------------------------*/
/* GFXMSEL                                                                                                 */
/*---------------------------------------------------------------------------------------------------------*/
#define npcm750_clk_SEL_GFXMSEL_CLKREF             0x02          /*  1 0    : CLKREF clock (25 MHz, default).      */
#define npcm750_clk_SEL_GFXMSEL_PLL2               0x03          /*  1 1    : NPCM750_PLL2 clock, frequency divided by 3.  */

/*---------------------------------------------------------------------------------------------------------*/
/* CLKOUTSEL                                                                                               */
/*---------------------------------------------------------------------------------------------------------*/
#define npcm750_clk_SEL_CLKOUTSEL_PLL0             0x00          /*   0 0 0 : NPCM750_PLL0 clock.                            */
#define npcm750_clk_SEL_CLKOUTSEL_PLL1             0x01          /*   0 0 1 : NPCM750_PLL1 clock.                            */
#define npcm750_clk_SEL_CLKOUTSEL_CLKREF           0x02          /*   0 1 0 : CLKREF input (25 MHz, default).        */
#define npcm750_clk_SEL_CLKOUTSEL_PLLG             0x03          /*   0 1 1  Graphics PLL output clock, divided by 2 */
#define npcm750_clk_SEL_CLKOUTSEL_PLL2             0x04          /*   1 0 0 : NPCM750_PLL2 clock divided by 2.               */

/*---------------------------------------------------------------------------------------------------------*/
/* GFXCKSEL                                                                                                */
/*---------------------------------------------------------------------------------------------------------*/
#define npcm750_clk_SEL_GFXCKSEL_PLL0              0x00          /*   0 0   : NPCM750_PLL0 clock.                            */
#define npcm750_clk_SEL_GFXCKSEL_PLL1              0x01          /*   0 1   : NPCM750_PLL1 clock.                            */
#define npcm750_clk_SEL_GFXCKSEL_CLKREF            0x02          /*   1 0   : CLKREF clock (25 MHz, default).        */
#define npcm750_clk_SEL_GFXCKSEL_PLL2              0x03          /*   1 1   : NPCM750_PLL2 clock divided by 2.               */

/*---------------------------------------------------------------------------------------------------------*/
/* TIMCKSEL                                                                                                */
/*---------------------------------------------------------------------------------------------------------*/
#define npcm750_clk_SEL_TIMCKSEL_PLL0              0x00          /*   0 0   : NPCM750_PLL0 clock.                            */
#define npcm750_clk_SEL_TIMCKSEL_PLL1              0x01          /*   0 1   : NPCM750_PLL1 clock.                            */
#define npcm750_clk_SEL_TIMCKSEL_CLKREF            0x02          /*   1 0   : CLKREF clock (25 MHz, default).        */
#define npcm750_clk_SEL_TIMCKSEL_PLL2              0x03          /*   1 1   : NPCM750_PLL2 clock divided by 2.               */

/*---------------------------------------------------------------------------------------------------------*/
/* MCCKSEL                                                                                                 */
/*---------------------------------------------------------------------------------------------------------*/
#define npcm750_clk_SEL_MCCKSEL_PLL1               0x00          /*  0 0    : NPCM750_PLL1 clock.                            */
#define npcm750_clk_SEL_MCCKSEL_CLKREF             0x02          /*  1 0    : CLKREF clock (25 MHz, default).        */
#define npcm750_clk_SEL_MCCKSEL_MCBPCK             0x03          /*  1 1    : MCBPCK clock input.                    */

/*---------------------------------------------------------------------------------------------------------*/
/* SUCKSEL                                                                                                 */
/*---------------------------------------------------------------------------------------------------------*/
#define npcm750_clk_SEL_SUCKSEL_PLL0               0x00          /*  0 0    : NPCM750_PLL0 clock.                            */
#define npcm750_clk_SEL_SUCKSEL_PLL1               0x01          /*  0 1    : NPCM750_PLL1 clock.                            */
#define npcm750_clk_SEL_SUCKSEL_CLKREF             0x02          /*  1 0    : CLKREF clock (25 MHz, default).        */
#define npcm750_clk_SEL_SUCKSEL_PLL2               0x03          /*  1 1    : NPCM750_PLL2 clock divided by 2.               */

/*---------------------------------------------------------------------------------------------------------*/
/* UARTCKSEL                                                                                               */
/*---------------------------------------------------------------------------------------------------------*/
#define npcm750_clk_SEL_UARTCKSEL_PLL0             0x00          /*  0 0    : NPCM750_PLL0 clock.                          */
#define npcm750_clk_SEL_UARTCKSEL_PLL1             0x01          /*  0 1    : NPCM750_PLL1 clock.                          */
#define npcm750_clk_SEL_UARTCKSEL_CLKREF           0x02          /*  1 0    : CLKREF clock (25 MHz, default).      */
#define npcm750_clk_SEL_UARTCKSEL_PLL2             0x03          /*  1 1    : NPCM750_PLL2 clock divided by 2.             */

/*---------------------------------------------------------------------------------------------------------*/
/* SDCKSEL                                                                                                 */
/*---------------------------------------------------------------------------------------------------------*/
#define npcm750_clk_SEL_SDCKSEL_PLL0               0x00          /*   0 0   : NPCM750_PLL0 clock.                          */
#define npcm750_clk_SEL_SDCKSEL_PLL1               0x01          /*   0 1   : NPCM750_PLL1 clock.                          */
#define npcm750_clk_SEL_SDCKSEL_CLKREF             0x02          /*   1 0   : CLKREF clock (25 MHz, default).      */
#define npcm750_clk_SEL_SDCKSEL_PLL2               0x03          /*   1 1   : NPCM750_PLL2 clock divided by 2.             */

/*---------------------------------------------------------------------------------------------------------*/
/* PIXCKSEL                                                                                                */
/*---------------------------------------------------------------------------------------------------------*/
#define npcm750_clk_SEL_PIXCKSEL_PLLG              0x00          /*   0 0   : PLL GFX clock after divide to 2.     */
#define npcm750_clk_SEL_PIXCKSEL_CLKOUT            0x01          /*   0 1   : CLKOUT/GPIO160 pin as input (MFSEL1.21 and GPIO160 controls should be left at default state).*/
#define npcm750_clk_SEL_PIXCKSEL_CLKREF            0x02          /*   1 0   : CLKREF input. (default)              */

/*---------------------------------------------------------------------------------------------------------*/
/* GPRFSEL                                                                                                 */
/*---------------------------------------------------------------------------------------------------------*/
#define npcm750_clk_SEL_GPRFSEL_GFXBYPCK           0x00          /*   0 0   : GFXBYPCK pin.                        */
#define npcm750_clk_SEL_GPRFSEL_USB                0x01          /*   0 1   : USB OHCI Clock (48 MHz).             */
#define npcm750_clk_SEL_GPRFSEL_CLKREF             0x02          /*   1 0   : CLKREF input. (default)              */

/*---------------------------------------------------------------------------------------------------------*/
/* CPUCKSEL                                                                                                */
/*---------------------------------------------------------------------------------------------------------*/
#define npcm750_clk_SEL_CPUCKSEL_PLL0              0x00          /*   0 0   : NPCM750_PLL0 clock.                          */
#define npcm750_clk_SEL_CPUCKSEL_PLL1              0x01          /*   0 1   : NPCM750_PLL1 clock.                          */
#define npcm750_clk_SEL_CPUCKSEL_CLKREF            0x02          /*   1 0   : CLKREF input (25 MHz, default).      */
#define npcm750_clk_SEL_CPUCKSEL_SYSBPCK           0x03          /*   1 1   : Bypass clock from pin SYSBPCK.       */

/*---------------------------------------------------------------------------------------------------------*/
/* ADCCKDIV                                                                                                */
/*---------------------------------------------------------------------------------------------------------*/
#define npcm750_clk_SEL_ADCCKDIV1              0x00          /*    0 0 0: /1 (should not be used).             */
#define npcm750_clk_SEL_ADCCKDIV2              0x01          /*    0 0 1: /2.                                  */
#define npcm750_clk_SEL_ADCCKDIV4              0x02          /*    0 1 0: /4.                                  */
#define npcm750_clk_SEL_ADCCKDIV8              0x03          /*    0 1 1: /8.                                  */
#define npcm750_clk_SEL_ADCCKDIV16             0x04          /*    1 0 0: /16                                  */
#define npcm750_clk_SEL_ADCCKDIV32             0x05          /*    1 0 1: /32 (default)                        */

/*---------------------------------------------------------------------------------------------------------*/
/* APBxCKDIV                                                                                               */
/*---------------------------------------------------------------------------------------------------------*/
#define npcm750_clk_SEL_APB_DIV1                   0x00          /*     0 0 : AHB1CLK/1.                           */
#define npcm750_clk_SEL_APB_DIV2                   0x01          /*     0 1 : AHB1CLK/2. (default for NPCM750_APB5 )       */
#define npcm750_clk_SEL_APB_DIV4                   0x02          /*     1 0 : AHB1CLK/4. (default, except NPCM750_APB5)    */
#define npcm750_clk_SEL_APB_DIV8                   0x03          /*     1 1 : AHB1CLK/8.                           */



#define NPCMX50_CLKREF  25000000
//(25 * (_1MHz_))


/***********************************************************************/

/*---------------------------------------------------------------------------------------------------------*/
/* DVCSSEL (DVC System Clock Source Select Bit). Typically, a divided NPCM750_PLL2 (by 4): 240 MHz.                */
/*---------------------------------------------------------------------------------------------------------*/
#define   CLKSEL_DVCSSEL_CLKREF       0x02   /*   1 0: CLKREF clock (25 MHz, default).  */
#define   CLKSEL_DVCSSEL_PLL2         0x03   /*   1 1: NPCM750_PLL2 clock, frequency divided by 4.  */



/*---------------------------------------------------------------------------------------------------------*/
/* GFXMSEL (Graphics Memory Clock Source Select Bit). Typically it would be divided NPCM750_PLL2, 320MHz.          */
/*---------------------------------------------------------------------------------------------------------*/
#define   CLKSEL_GFXMSEL_CLKREF       0x02   /*   1 0: CLKREF clock (25 MHz, default).  */
#define   CLKSEL_GFXMSEL_PLL2         0x03   /*   1 1: NPCM750_PLL2    clock, frequency divided by 3.  */

/*---------------------------------------------------------------------------------------------------------*/
/* CLKOUTSEL (CLKOUT signal Clock Source Select Bit).                                                      */
/*---------------------------------------------------------------------------------------------------------*/
#define   CLKSEL_CLKOUTSEL_PLL0       0x00   /*  0 0 0: NPCM750_PLL0    clock.  */
#define   CLKSEL_CLKOUTSEL_PLL1       0x01   /*  0 0 1: NPCM750_PLL1    clock.  */
#define   CLKSEL_CLKOUTSEL_CLKREF     0x02   /*  0 1 0: CLKREF input (25 MHz, default).  */
#define   CLKSEL_CLKOUTSEL_PLLG       0x03   /*  0 1 1: Graphics PLL output clock, divided by 2 .  */
#define   CLKSEL_CLKOUTSEL_PLL2       0x04   /*  1 0 0: NPCM750_PLL2    clock divided by 2.  */

/*---------------------------------------------------------------------------------------------------------*/
/* GFXCKSEL (Graphics System Clock Source Select Bit). Typically it would be the same source of the CPU    */
/* clock.                                                                                                  */
/*---------------------------------------------------------------------------------------------------------*/
#define   CLKSEL_GFXCKSEL_PLL0       0x00   /*  0 0: NPCM750_PLL0    clock.  */
#define   CLKSEL_GFXCKSEL_PLL1       0x01   /*  0 1: NPCM750_PLL1    clock.  */
#define   CLKSEL_GFXCKSEL_CLKREF     0x02   /*  1 0: CLKREF clock (25 MHz, default).  */
#define   CLKSEL_GFXCKSEL_PLL2       0x03   /*  1 1: NPCM750_PLL2    clock divided by 2.  */
/* Note: Before changing this field, assure a delay of 200 clock cycles from last change of GSCKDIV and PCICKDIV fields in CLKDIVx register.  */

/*---------------------------------------------------------------------------------------------------------*/
/* TIMCKSEL (Timer Clock Source Select Bit).                                                               */
/*---------------------------------------------------------------------------------------------------------*/
#define   CLKSEL_TIMCKSEL_PLL0       0x00   /*  0 0: NPCM750_PLL0    clock.  */
#define   CLKSEL_TIMCKSEL_PLL1       0x01   /*  0 1: NPCM750_PLL1    clock.  */
#define   CLKSEL_TIMCKSEL_CLKREF     0x02   /*  1 0: CLKREF clock (25 MHz, default).  */
#define   CLKSEL_TIMCKSEL_PLL2       0x03   /*  1 1: NPCM750_PLL2    clock divided by 2.  */
/* Note: Before changing this field, assure a delay of 200 clock cycles from last change of TIMCKDIV field in CLKDIV register.  */

/*---------------------------------------------------------------------------------------------------------*/
/* MCCKSEL (Memory Controller Clock Source Select Bit).                                                    */
/*---------------------------------------------------------------------------------------------------------*/
#define   CLKSEL_MCCKSEL_PLL1        0x00   /*  0 0: NPCM750_PLL1    clock.  */
#define   CLKSEL_MCCKSEL_CLKREF      0x02   /*  1 0: CLKREF clock (25 MHz, default).  */
#define   CLKSEL_MCCKSEL_MCBPCK      0x03   /*  1 1: MCBPCK clock input.  */

/*---------------------------------------------------------------------------------------------------------*/
/* SUCKSEL (USB Serial Clock Source Select Bit). For UTMI2UTMI and OHCI logic. Should select a clock of 480*/
/* MHZ.                                                                                                    */
/*---------------------------------------------------------------------------------------------------------*/
#define   CLKSEL_SUCKSEL_PLL0        0x00   /*  0 0: NPCM750_PLL0    clock.  */
#define   CLKSEL_SUCKSEL_PLL1        0x01   /*  0 1: NPCM750_PLL1    clock.  */
#define   CLKSEL_SUCKSEL_CLKREF      0x02   /*  1 0: CLKREF clock (25 MHz, default).  */
#define   CLKSEL_SUCKSEL_PLL2        0x03   /*  1 1: NPCM750_PLL2    clock divided by 2.  */
/* Note: Before changing this field, assure a delay of 200 clock cycles from the last change of SUCKDIV field in CLKDIV register.  */

/*---------------------------------------------------------------------------------------------------------*/
/* UARTCKSEL (Core and Host UART Clock Source Select Bit).                                                 */
/*---------------------------------------------------------------------------------------------------------*/
#define   CLKSEL_UARTCKSEL_PLL0      0x00   /*  0 0: NPCM750_PLL0    clock.  */
#define   CLKSEL_UARTCKSEL_PLL1      0x01   /*  0 1: NPCM750_PLL1    clock.  */
#define   CLKSEL_UARTCKSEL_CLKREF    0x02   /*  1 0: CLKREF clock (25 MHz, default).  */
#define   CLKSEL_UARTCKSEL_PLL2      0x03   /*  1 1: NPCM750_PLL2    clock divided by 2.  */
/* Note: Before changing this field, assure a delay of 200 clock cycles from last change of UARTDIV field in CLKDIV register.  */


/*---------------------------------------------------------------------------------------------------------*/
/* SDCKSEL (SDHC Clock Source Select Bit).                                                                 */
/*---------------------------------------------------------------------------------------------------------*/
#define   CLKSEL_SDCKSEL_PLL0        0x00   /* 0 0: NPCM750_PLL0    clock.  */
#define   CLKSEL_SDCKSEL_PLL1        0x01   /* 0 1: NPCM750_PLL1    clock.  */
#define   CLKSEL_SDCKSEL_CLKREF      0x02   /* 1 0: CLKREF clock (25 MHz, default).  */
#define   CLKSEL_SDCKSEL_PLL2        0x03   /* 1 1: NPCM750_PLL2    clock divided by 2.  */
/* Note: Before changing this field, assure a delay of 200 clock cycles from last change of SDCKDIV field in CLKDIV register. */

/*---------------------------------------------------------------------------------------------------------*/
/* PIXCKSEL (Pixel Clock Source Select Bit).                                                               */
/*---------------------------------------------------------------------------------------------------------*/
#define   CLKSEL_PIXCKSEL_PLLG       0x00   /* 0 0: PLL GFX clock after divide to 2.  */
#define   CLKSEL_PIXCKSEL_CLKOUT     0x01   /* 0 1: CLKOUT/GPIO160 pin as input (MFSEL1.21 and GPIO160 controls should be left at default state).  */
#define   CLKSEL_PIXCKSEL_CLKREF     0x02   /* 1 0: CLKREF input. (default)  */


/*---------------------------------------------------------------------------------------------------------*/
/* GPRFSEL (Graphics PLL Reference Clock Source Select Bit).                                               */
/*---------------------------------------------------------------------------------------------------------*/
#define   CLKSEL_GPRFSEL_GFXBYPCK     0x00   /* 0 0: GFXBYPCK pin.  */
#define   CLKSEL_GPRFSEL_USB_OHCI     0x01   /* 0 1: USB OHCI Clock (48 MHz).  */
#define   CLKSEL_GPRFSEL_CLKREF       0x02   /* 1 0: CLKREF input. (default)  */

/*---------------------------------------------------------------------------------------------------------*/
/* CPUCKSEL (CPU/AMBA/MC Clock Source Select Bit).                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#define   CLKSEL_CPUCKSEL_PLL0        0x00   /* 0 0: NPCM750_PLL0    clock.  */
#define   CLKSEL_CPUCKSEL_PLL1        0x01   /* 0 1: NPCM750_PLL1    clock.  */
#define   CLKSEL_CPUCKSEL_CLKREF      0x02   /* 1 0: CLKREF input (25 MHz, default).  */
#define   CLKSEL_CPUCKSEL_SYSBPCK     0x03   /* 1 1: Bypass clock from pin SYSBPCK.  */
/* Note: Before changing this field, assure a delay of 200 (selected) clock cycles from last change of CLKDIV register. */






/*---------------------------------------------------------------------------------------------------------*/
/* clock division field values:                                                                            */
/*---------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------------------------*/
/* CLKDIV1_ADCCKDIV                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
#define  CLKDIV1_ADCCKDIV1     0 /* 0 0 0: /1 (should not be used). */
#define  CLKDIV1_ADCCKDIV2     1 /* 0 0 1: /2. */
#define  CLKDIV1_ADCCKDIV4     2 /* 0 1 0: /4. */
#define  CLKDIV1_ADCCKDIV8     3 /* 0 1 1: /8. */
#define  CLKDIV1_ADCCKDIV16    4 /* 1 0 0: /16 */
#define  CLKDIV1_ADCCKDIV32    5 /* 1 0 1: /32 (default) */


/*---------------------------------------------------------------------------------------------------------*/
/* CLKDIV1_CLK4DIV (AMBA AHB Clock Divider Control)                                                        */
/*---------------------------------------------------------------------------------------------------------*/
#define  CLKDIV1_CLK4DIV1    0 /* 0: CLK2 Clock is used for CLK4. */
#define  CLKDIV1_CLK4DIV2    1 /* 1: CLK2 Clock is divided by 2 for CLK4 (default). */
#define  CLKDIV1_CLK4DIV3    2 /* 2: CLK2 Clock is divided by 3 for CLK4. */
#define  CLKDIV1_CLK4DIV4    3 /* 3: CLK2 Clock is divided by 4 for CLK4. */
 /* After changing this field, assure a delay of 200 clock cycles before changing CPUCKSEL field in CLKSEL register. */

/*---------------------------------------------------------------------------------------------------------*/
/* CLKDIV1_TIMCKDIV (Timer Clock Source Divider Control). Default is divide by 20. The division factor is  */
/* (TIMCKDIV +1), where TIMCKDIV value is 0-31.                                                            */
/*---------------------------------------------------------------------------------------------------------*/
#define  CLKDIV1_TIMCK_DIV(n)   ( (n) - 1)     /* • TIMCKDIV value is 0-31. */
/* Note: After changing this field, assure a delay of 200 selected clock cycles before the timer is used. */


/*---------------------------------------------------------------------------------------------------------*/
/* CLKDIV1_UARTDIV (UART Clock Source Divider Control). This resulting clock must be 24 MHz for UARTs proper*/
/* operation                                                                                               */
/*---------------------------------------------------------------------------------------------------------*/
#define  CLKDIV1_UART_DIV(n)   ( (n) - 1)     /* Default is divide by 20. */

/*---------------------------------------------------------------------------------------------------------*/
/* CLKDIV1_MMCCKDIV (MMC Controller (SDHC2) Clock Divider Control). Sets the division factor from the clock*/
/* selected by SDCKSEL.                                                                                    */
/*---------------------------------------------------------------------------------------------------------*/
#define  CLKDIV1_MMCCK_DIV(n)   ( (n) - 1)     /* The division factor is (MMCCKDIV+1), where MMCCKDIV is 0 to 31. Default is to divide by 32. */

/*---------------------------------------------------------------------------------------------------------*/
/* CLKDIV1_AHB3CKDIV (NPCM750_SPI3 Clock Divider Control). Sets the division factor from AHB clock (CLK4) to AHB3  */
/* and NPCM750_SPI3                                                                                                */
/*---------------------------------------------------------------------------------------------------------*/
#define  CLKDIV1_AHB3CK_DIV(n)   ( (n) - 1)     /* clock. The division factor is (AHB3CKDIV+1), where AHB3CKDIV is 0 to 31. Default is to divide by 2. */

/*---------------------------------------------------------------------------------------------------------*/
/* CLKDIV1_PCICKDIV (Internal PCI Clock Divider Control). Sets the division factor from the clock selected */
/* by the CLKSEL                                                                                           */
/*---------------------------------------------------------------------------------------------------------*/
#define  CLKDIV1_PCICK_DIV(n)   ( (n) - 1)     /* GFXCKSEL field. The division factor is (PCICKDIV+1), where PCICKDIV is 0 to 15. */

/*---------------------------------------------------------------------------------------------------------*/
/* CLKDIV1_CLK2DIV (AMBA AXI Clock Divider Control).                                                       */
/*---------------------------------------------------------------------------------------------------------*/
#define  CLKDIV1_CLK2DIV1    0 /* 0: CPU Clock is used for AXI16 (CLK2). */
#define  CLKDIV1_CLK2DIV2    1 /* 1: CPU Clock is divided by 2 for AXI16 (CLK2)(default). */

/*---------------------------------------------------------------------------------------------------------*/
/* CLKDIV2_APBxCKDIV (AMBA APBx Clock Divider Control).                                                    */
/* NPCM750_APB1 clock must be at least 24 MHz, for UARTs to function. APB clock frequency is up to 67 MHz          */
/* NPCM750_APB2 clock frequency is up to 67 MHz.                                                                   */
/* NPCM750_APB3 clock frequency is up to 67 MHz                                                                    */
/* NPCM750_APB4 clock frequency is up to 67 MHz                                                                    */
/* NPCM750_APB5 clock must be high to enable PSPI1-2 high SPI clock rate. The preferred setting is divide by 2     */
/*---------------------------------------------------------------------------------------------------------*/
#define  CLKDIV2_APBxCKDIV1    0 /* 0 0: AHB1CLK/1. */
#define  CLKDIV2_APBxCKDIV2    1 /* 0 1: AHB1CLK/2. */
#define  CLKDIV2_APBxCKDIV4    2 /* 1 0: AHB1CLK/4 (default). */
#define  CLKDIV2_APBxCKDIV8    3 /* 1 1: AHB1CLK/8. */
 /* Note: After changing this field, assure a delay of 200 clock cycles before changing CPUCKSEL field in CLKSEL */


/*---------------------------------------------------------------------------------------------------------*/
/* GFXCKDIV (Graphics System Clock Divider Control). Sets the division factor from the clock selected by the*/
/* CLKSEL.GFXCKSEL field. The division factor is (GFXCKDIV+1), where GFXCKDIV is 0 to 7.                   */
/* Default is divide by 5.                                                                                 */
/*---------------------------------------------------------------------------------------------------------*/
#define  CLKDIV2_GFXCKDIV_DIV(n)   ( (n) - 1)     /* is (SPI0CKDIV+1), where SPI0CKDIV is 0 to 31. Default is to divide by 1. */
/* After changing this field, ensure a delay of 200 selected clock cycles before changing GFXCKSEL field in CLKSEL register. */



/*---------------------------------------------------------------------------------------------------------*/
/* CLKDIV3_SPI0CKDV (NPCM750_SPI0 Clock Divider Control). Sets the division factor from AHB clock to NPCM750_SPI0 clock. The*/
/* division factor   is (SPI0CKDIV+1)                                                                                      */
/*---------------------------------------------------------------------------------------------------------*/
#define  CLKDIV3_SPI0CKDV_DIV(n)   ( (n) - 1)     /* is (SPI0CKDIV+1), where SPI0CKDIV is 0 to 31. Default is to divide by 1. */

/*---------------------------------------------------------------------------------------------------------*/
/* CLKDIV3_SPIXCKDV (SPIX Clock Divider Control). Sets the division factor from AHB clock to SPIX clock. The*/
/* division factor    is (SPIXCKDIV+1)                                                                                      */
/*---------------------------------------------------------------------------------------------------------*/
#define  CLKDIV3_SPIXCKDV_DIV(n)   ( (n) - 1)     /* is (SPIXCKDIV+1), where SPIXCKDIV is 0 to 31. Default is to divide by 1. */


/*---------------------------------------------------------------------------------------------------------*/
/* PWDEN:  PLL power down values                                                                           */
/*---------------------------------------------------------------------------------------------------------*/
#define  PLLCONn_PWDEN_NORMAL            0
#define  PLLCONn_PWDEN_POWER_DOWN        1


typedef enum
{
    NPCM750_PLL0    = 0,
    NPCM750_PLL1    = 1,
    NPCM750_PLL2    = 2,
    NPCM750_PLL_GFX = 3,

} NPCM750_PLL_MODULE_T ;


typedef enum
{
    NPCM750_APB1  = 1,
    NPCM750_APB2  = 2,
    NPCM750_APB3  = 3,
    NPCM750_APB4  = 4,
    NPCM750_APB5  = 5,
	NPCM750_SPI0  = 6,
	NPCM750_SPI3  = 7,
	NPCM750_SPIX  = 8
} NPCM750_APB_CLK;


typedef enum
{
    NPCM750_CLK_SD  = 0,
    NPCM750_CLK_MMC = 1,
} NPCM750_SD_MMC_CLK_T;


typedef enum
{
    FIU_MODULE_0 = 0,
    FIU_MODULE_1 = 1,
    FIU_MODULE_2 = 2,
    FIU_MODULE_3 = 3,
    FIU_MODULE_X = 4
} FIU_MODULE_T;


#endif // NPCM750_npcm750_clk_REGS_H

