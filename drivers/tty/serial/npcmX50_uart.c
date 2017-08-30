/*
 * UART API
 *
 * Support for Nuvoton UART HW.
 *
 * Copyright (c) 2016 Nuvoton Technologies
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 */


#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/sysrq.h>
#include <linux/console.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <generated/autoconf.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/spinlock_types.h>
#include <linux/regmap.h>
#include <linux/mfd/syscon.h>

#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>


#include <asm/io.h>
#include <asm/irq.h>

//#include <mach/hardware.h>
//#include <mach/hal.h>

static struct regmap *gcr_regmap;

#define  MFSEL1_OFFSET 0x00C
#define  MFSEL4_OFFSET 0x0B0
#define  SPSWC_OFFSET  0x038

#define MSN(parm8)        ((u8)((u8)(parm8) >> 4))
#define LSN(parm8)        ((u8)((u8)parm8 & 0x0F))
#define MSB(parm16)        ((u8)((u16)(parm16) >> 8))
#define LSB(parm16)        ((u8)(parm16))
#define MSW(parm32)        ((u16)((u32)(parm32) >> 16))
#define LSW(parm32)        ((u16)(parm32))
#define MSD(parm64)        ((u32)((u64)(parm64) >> 32))
#define LSD(parm64)        ((u32)(parm64))
#define PORT_NPCMX50    101

/*---------------------------------------------------------------------------------------------------------*/
/* Debug messages                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
#if 0
#define NPCMX50_SERIAL_MSG(format, arg...) printk(KERN_NOTICE format , ## arg)
#else
#define NPCMX50_SERIAL_MSG(format, arg...)  do { } while(0)
#endif

#define NPCMX50_UART_NUM_OF_MODULES             4

#define _1Hz_           1UL
#define _1KHz_          (1000 * _1Hz_ )
#define _1MHz_          (1000 * _1KHz_)
#define _1GHz_          (1000 * _1MHz_)

typedef struct bit_field {
    u8 offset;
    u8 size;
} bit_field_t;

/*---------------------------------------------------------------------------------------------------------*/
/* UART REGS                                                                                               */
/*---------------------------------------------------------------------------------------------------------*/

/**************************************************************************************************************************/
/*   Receive Buffer Register (RBR)                                                                                        */
/**************************************************************************************************************************/
#define  UART_RBR(n)                            ( npcmX50_serial_ports[n].port.membase + 0x0000)        /* Offset: UART_BA + 0000h */
static const bit_field_t RBR_Transmit =   { 0 , 8 };  /* Transmit Holding Register (THR)                                                                                       */

/**************************************************************************************************************************/
/*   Transmit Holding Register (THR)                                                                                      */
/**************************************************************************************************************************/
#define  UART_THR(n)                            ( npcmX50_serial_ports[n].port.membase + 0x0000)        /* Offset: UART_BA + 0000h */

/**************************************************************************************************************************/
/*   Interrupt Enable Register (IER)                                                                                      */
/**************************************************************************************************************************/
#define  UART_IER(n)                            ( npcmX50_serial_ports[n].port.membase + 0x0004)        /* Offset: UART_BA + 0004h */
static const bit_field_t IER_nDBGACK_EN = { 4 , 1 };  /* 4 nDBGACK_EN (ICE Debug Mode Acknowledge Enable).                                                                     */
static const bit_field_t IER_MSIE =       { 3 , 1 };  /* 3 MSIE (Modem Status Interrupt (Irpt_MOS) Enable).                                                                    */
static const bit_field_t IER_RLSIE =      { 2 , 1 };  /* 2 RLSIE (Receive Line Status Interrupt (Irpt_RLS) Enable).                                                            */
static const bit_field_t IER_THREIE =     { 1 , 1 };  /* 1 THREIE (Transmit Holding Register Empty Interrupt (Irpt_THRE) Enable).                                              */
static const bit_field_t IER_RDAIE =      { 0 , 1 };  /* 0 RDAIE (Receive Data Available Interrupt (Irpt_RDA) Enable and Timeout Interrupt (Irpt_TOUT) Enable).                */

/**************************************************************************************************************************/
/*   Divisor Latch (Low Byte) Register (DLL)                                                                              */
/**************************************************************************************************************************/
#define  UART_DLL(n)                            ( npcmX50_serial_ports[n].port.membase + 0x0000)        /* Offset: UART_BA + 0000h */

/**************************************************************************************************************************/
/*   Divisor Latch (High Byte) Register (DLM)                                                                             */
/**************************************************************************************************************************/
#define  UART_DLM(n)                            ( npcmX50_serial_ports[n].port.membase + 0x0004)         /* Offset: UART_BA + 0004h */
static const bit_field_t DLM_Baud =  { 0 , 8 };  /* 7-0 Baud Rate Divisor (High Byte). The high byte of the baud rate divisor.                                            */

/**************************************************************************************************************************/
/*   Interrupt Identification Register (IIR)                                                                              */
/**************************************************************************************************************************/
#define  UART_IIR(n)                            ( npcmX50_serial_ports[n].port.membase + 0x0008)        /* Offset: UART_BA + 0008h */
static const bit_field_t IIR_FMES =       { 7 , 1 };  /* 7 FMES (FIFO Mode Enable Status). Indicates whether FIFO mode is enabled or not. Since FIFO mode is                   */
static const bit_field_t IIR_RFTLS =      { 5 , 2 };  /* 6-5 RFTLS (RxFIFO Threshold Level Status). Shows the current setting of the receiver FIFO threshold level             */
static const bit_field_t IIR_DMS =        { 4 , 1 };  /* 4 DMS (DMA Mode Select). The DMA function is not implemented in this version. When reading IIR, the DMS               */
static const bit_field_t IIR_IID =        { 1 , 3 };  /* 3-1 IID (Interrupt Identification). IID together with NIP indicates the current interrupt request from the UART.      */
static const bit_field_t IIR_NIP =        { 0 , 1 };  /* 0 NIP (No Interrupt Pending).                                                                                         */

/**************************************************************************************************************************/
/*   FIFO Control Register (FCR)                                                                                          */
/**************************************************************************************************************************/
#define  UART_FCR(n)                            ( npcmX50_serial_ports[n].port.membase + 0x0008)        /* Offset: UART_BA + 0008h */
static const bit_field_t FCR_RFITL =      { 4 , 4 };  /* 7-4 RFITL (RxFIFO Interrupt (Irpt_RDA) Trigger Level).                                                                */
static const bit_field_t FCR_DMS =        { 3 , 1 };  /* 3 DMS (DMA Mode Select). The DMA function is not implemented in this version.                                         */
static const bit_field_t FCR_TFR =        { 2 , 1 };  /* 2 TFR (TxFIFO Reset). Setting this bit generates a reset to the TxFIFO. The TxFIFO becomes empty (Tx pointer          */
static const bit_field_t FCR_RFR =        { 1 , 1 };  /* 1 RFR (RxFIFO Reset). Setting this bit generates an OSC cycle reset pulse to reset the RxFIFO. The RxFIFO             */
static const bit_field_t FCR_FME =        { 0 , 1 };  /* 0 FME (FIFO Mode Enable). The UART always operates in FIFO mode; therefore, writing this bit has no effect            */

/**************************************************************************************************************************/
/*   Line Control Register (LCR)                                                                                          */
/**************************************************************************************************************************/
#define  UART_LCR(n)                            ( npcmX50_serial_ports[n].port.membase + 0x000C)        /* Offset: UART_BA + 000Ch */
static const bit_field_t LCR_DLAB =       { 7 , 1 };  /* 7 DLAB (Divisor Latch Access Bit).                                                                                    */
static const bit_field_t LCR_BCB =        { 6 , 1 };  /* 6 BCB (Break Control Bit). When this bit is set to logic 1, the serial data output (SOUT) is forced to the Spacing    */
static const bit_field_t LCR_SPE =        { 5 , 1 };  /* 5 SPE (Stick Parity Enable).                                                                                          */
static const bit_field_t LCR_EPE =        { 4 , 1 };  /* 4 EPE (Even Parity Enable).                                                                                           */
static const bit_field_t LCR_PBE =        { 3 , 1 };  /* 3 PBE (Parity Bit Enable).                                                                                            */
static const bit_field_t LCR_NSB =        { 2 , 1 };  /* 2 NSB (Number of "STOP Bits").                                                                                        */
static const bit_field_t LCR_WLS =        { 0 , 2 };  /* 1-0 WLS (Word Length Select).                                                                                         */
static const bit_field_t LCR_Word =       { 10 , 1 };  /* 10 Word Length                                                                                                        */

/**************************************************************************************************************************/
/*   Modem Control Register (MCR)                                                                                         */
/**************************************************************************************************************************/
#define  UART_MCR(n)                            ( npcmX50_serial_ports[n].port.membase + 0x0010)        /* Offset: UART_BA + 0010h */
static const bit_field_t MCR_LBME =       { 4 , 1 };  /* 4 LBME (Loopback Mode Enable).                                                                                        */
static const bit_field_t MCR_OUT2 =       { 3 , 1 };  /* 3 OUT2. Used in loopback mode to drive DCD input.                                                                     */
static const bit_field_t MCR_RTS =        { 1 , 1 };  /* 1 RTS (Request to Send Signal). Complement version of Request to Send (RTS) signal.                                   */
static const bit_field_t MCR_DTR =        { 0 , 1 };  /* 0 DTR (Data Terminal Ready Signal). Complement version of Data Terminal Ready (DTR) signal.                           */

/**************************************************************************************************************************/
/*   Line Status Control Register (LSR)                                                                                   */
/**************************************************************************************************************************/
#define  UART_LSR(n)                            ( npcmX50_serial_ports[n].port.membase + 0x0014)        /* Offset: UART_BA + 0014h */
static const bit_field_t LSR_ERR_Rx =     { 7 , 1 };  /* 7 ERR_Rx (RxFIFO Error).                                                                                              */
static const bit_field_t LSR_TE =         { 6 , 1 };  /* 6 TE (Transmitter Empty).                                                                                             */
static const bit_field_t LSR_THRE =       { 5 , 1 };  /* 5 THRE (Transmitter Holding Register Empty).                                                                          */
static const bit_field_t LSR_BII =        { 4 , 1 };  /* 4 BII (Break Interrupt Indicator). Is set to a logic 1 when the received data input is held in the "spacing state"    */
static const bit_field_t LSR_FEI =        { 3 , 1 };  /* 3 FEI (Framing Error Indicator). Is set to logic 1 when the received character does not have a valid "stop bit"       */
static const bit_field_t LSR_PEI =        { 2 , 1 };  /* 2 PEI (Parity Error Indicator). This bit is set to logic 1 when the received character does not have a valid "parity  */
static const bit_field_t LSR_OEI =        { 1 , 1 };  /* 1 OEI (Overrun Error Indicator). An overrun error occurs only after the RxFIFO is full and the next character has     */
static const bit_field_t LSR_RFDR =       { 0 , 1 };  /* 0 RFDR (RxFIFO Data Ready).                                                                                           */

/**************************************************************************************************************************/
/*   Modem Status Register (MSR)                                                                                          */
/**************************************************************************************************************************/
#define  UART_MSR(n)                            ( npcmX50_serial_ports[n].port.membase , NPCMX50_UART_ACCESS, 8        /* Offset: UART_BA + 0018h */
static const bit_field_t MSR_DCD =        { 7 , 1 };  /* 7 DCD. (Data Carrier Detect). Complement version of Data Carrier Detect (DCD) input.                                  */
static const bit_field_t MSR_RI =         { 6 , 1 };  /* 6 RI. (Ring Indicator) Complement version of Ring Indicator (RI) input.                                               */
static const bit_field_t MSR_DSR =        { 5 , 1 };  /* 5 DSR (Data Set Ready). Complement version of Data Set Ready (DSR) input.                                             */
static const bit_field_t MSR_CTS =        { 4 , 1 };  /* 4 CTS (Clear to Send). Complement version of Clear To Send (CTS) input).                                              */
static const bit_field_t MSR_DDCD =       { 3 , 1 };  /* 3 DDCD (DCD State Change). Is set when DCD input changes state; it is reset if the CPU reads the MSR. When            */
static const bit_field_t MSR_DRI =        { 2 , 1 };  /* 2 DRI (RI State Change). It is set when RI input changes state to asserted; it is reset if the CPU reads the MSR.     */
static const bit_field_t MSR_DDSR =       { 1 , 1 };  /* 1 DDSR (DSR State Change). It is set when DSR input changes state; it is reset if the CPU reads the MSR.              */
static const bit_field_t MSR_DCTS =       { 0 , 1 };  /* 0 DCTS (CTS State Change). It is set when CTS input changes state; it is reset if the CPU reads the MSR.              */

/**************************************************************************************************************************/
/*   Timeout Register (TOR)                                                                                               */
/**************************************************************************************************************************/
#define  UART_TOR(n)                            ( npcmX50_serial_ports[n].port.membase + 0x001C)        /* Offset: UART_BA + 001Ch */
static const bit_field_t TOR_TOIE =       { 7 , 1 };  /* 7 TOIE (Timeout Interrupt Enable). Enabled only when this bit is set and IER register bit 0 is set.                   */
static const bit_field_t TOR_TOIC =       { 0 , 7 };  /* 6-0 TOIC (Timeout Interrupt Comparator). The timeout counter resets and starts counting (the counting clock =         */


enum FCR_RFITL_type
{
    FCR_RFITL_1B    = 0x0,
    FCR_RFITL_4B    = 0x4,
    FCR_RFITL_8B    = 0x8,
    FCR_RFITL_14B   = 0xC,
};



enum LCR_WLS_type
{
    LCR_WLS_5bit    = 0x0,
    LCR_WLS_6bit    = 0x1,
    LCR_WLS_7bit    = 0x2,
    LCR_WLS_8bit    = 0x3,
};


enum IIR_IID_type
{
  IIR_IID_MODEM = 0x0,
  IIR_IID_THRE  = 0x1,
  IIR_IID_TOUT  = 0x5,
  IIR_IID_RDA   = 0x2,
  IIR_IID_RLS   = 0x3,
};



/*---------------------------------------------------------------------------------------------------------*/
/* UART ports definition                                                                                   */
/*---------------------------------------------------------------------------------------------------------*/
typedef enum
{
    NPCMX50_UART0_DEV = 0,  // UART0 is a general UART block without modem-I/O-control connection to external signals.
    NPCMX50_UART1_DEV = 1,  // UART1-3 are each a general UART with modem-I/O-control connection to external signals.
    NPCMX50_UART2_DEV = 2,
    NPCMX50_UART3_DEV = 3,
} NPCMX50_UART_DEV_T;

#ifdef CONFIG_NACH_NPCM650
/*---------------------------------------------------------------------------------------------------------*/
/* Uart Mux modes definitions                                                                              */
/*---------------------------------------------------------------------------------------------------------*/

typedef enum
{
    NPCMX50_UART_MUX_CORE_SNOOP        = 0,
    NPCMX50_UART_MUX_CORE_TAKEOVER     = 1,
    NPCMX50_UART_MUX_CORE_SP2__SP1_SI1 = 2,
    NPCMX50_UART_MUX_CORE_SP2__SP1_SI2 = 3,
	NPCMX50_UART_MUX_SKIP_CONFIG       = 4,
} NPCMX50_UART_MUX_T;
#endif

#ifdef CONFIG_MACH_NPCM750
/*---------------------------------------------------------------------------------------------------------*/
/* Uart Mux modes definitions. These numbers match the register field value. Do not change !               */
/* s == snoop                                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
typedef enum NPCMX50_UART_MUX_T_
{
    NPCMX50_UART_MUX_MODE1_HSP1_SI2____HSP2_UART2__UART1_s_HSP1__UART3_s_SI2                = 0,       // 0 0 0: Mode 1 - HSP1 connected to SI2  , HSP2 connected to UART2 ,UART1 snoops HSP1, UART3 snoops SI2
    NPCMX50_UART_MUX_MODE2_HSP1_UART1__HSP2_SI2____UART2_s_HSP2__UART3_s_SI2                = 1,       // 0 0 1: Mode 2 - HSP1 connected to UART1, HSP2 connected to SI2   ,UART2 snoops HSP2, UART3 snoops SI2
    NPCMX50_UART_MUX_MODE3_HSP1_UART1__HSP2_UART2__UART3_SI2                                = 2,       // 0 1 0: Mode 3 - HSP1 connected to UART1, HSP2 connected to UART2 ,UART3 connected to SI2
    NPCMX50_UART_MUX_MODE4_HSP1_SI1____HSP2_SI2____UART1_s_SI1___UART3_s_SI2__UART2_s_HSP1  = 3,       // 0 1 1: Mode 4 - HSP1 connected to SI1  , HSP2 connected to SI2   ,UART1 snoops SI1,  UART3 snoops SI2,   UART2 snoops HSP1 (default)
    NPCMX50_UART_MUX_MODE5_HSP1_SI1____HSP2_UART2__UART1_s_HSP1__UART3_s_SI1                = 4,       // 1 0 0: Mode 5 - HSP1 connected to SI1  , HSP2 connected to UART2 ,UART1 snoops HSP1, UART3 snoops SI1
    NPCMX50_UART_MUX_MODE6_HSP1_SI1____HSP2_SI2____UART1_s_SI1___UART3_s_SI2__UART2_s_HSP2  = 5,       // 1 0 1: Mode 6 - HSP1 connected to SI1  , HSP2 connected to SI2   ,UART1 snoops SI1,  UART3 snoops SI2,   UART2 snoops HSP2
    NPCMX50_UART_MUX_MODE7_HSP1_SI1____HSP2_UART2__UART1_s_HSP1__UART3_SI2                  = 6,       // 1 1 0: Mode 7 - HSP1 connected to SI1  , HSP2 connected to UART2 ,UART1 snoops HSP1, UART3 connected to SI2
    NPCMX50_UART_MUX_RESERVED                                                               = 7,       // skip uart mode configuration.
    NPCMX50_UART_MUX_SKIP_CONFIG                                                            = 8        // this is a SW option to allow config of UART without touching the UART mux.
} NPCMX50_UART_MUX_T;
#endif


/*---------------------------------------------------------------------------------------------------------*/
/* Common baudrate definitions                                                                             */
/*---------------------------------------------------------------------------------------------------------*/
typedef enum
{
    NPCMX50_UART_BAUDRATE_110       = 110,
    NPCMX50_UART_BAUDRATE_300       = 300,
    NPCMX50_UART_BAUDRATE_600       = 600,
    NPCMX50_UART_BAUDRATE_1200      = 1200,
    NPCMX50_UART_BAUDRATE_2400      = 2400,
    NPCMX50_UART_BAUDRATE_4800      = 4800,
    NPCMX50_UART_BAUDRATE_9600      = 9600,
    NPCMX50_UART_BAUDRATE_14400     = 14400,
    NPCMX50_UART_BAUDRATE_19200     = 19200,
    NPCMX50_UART_BAUDRATE_38400     = 38400,
    NPCMX50_UART_BAUDRATE_57600     = 57600,
    NPCMX50_UART_BAUDRATE_115200    = 115200,
    NPCMX50_UART_BAUDRATE_230400    = 230400,
    NPCMX50_UART_BAUDRATE_380400    = 380400,
    NPCMX50_UART_BAUDRATE_460800    = 460800,
} NPCMX50_UART_BAUDRATE_T;



/*---------------------------------------------------------------------------------------------------------*/
/* Uart Rx Fifo Trigger level definitions                                                                  */
/*---------------------------------------------------------------------------------------------------------*/
typedef enum
{
    NPCMX50_UART_RXFIFO_TRIGGER_1B      = 0x0,
    NPCMX50_UART_RXFIFO_TRIGGER_4B      = 0x1,
    NPCMX50_UART_RXFIFO_TRIGGER_8B      = 0x2,
    NPCMX50_UART_RXFIFO_TRIGGER_14B     = 0x3,
} NPCMX50_UART_RXFIFO_TRIGGER_T;


/*---------------------------------------------------------------------------------------------------------*/
/* UART parity types                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
typedef enum
{
    NPCMX50_UART_PARITY_NONE    = 0x00,
    NPCMX50_UART_PARITY_EVEN    = 0x01,
    NPCMX50_UART_PARITY_ODD     = 0x02,
} NPCMX50_UART_PARITY_T;


/*---------------------------------------------------------------------------------------------------------*/
/* Uart stop bits                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
typedef enum
{
    NPCMX50_UART_STOPBIT_1          = 0x00,
    NPCMX50_UART_STOPBIT_DYNAMIC    = 0x01,
} NPCMX50_UART_STOPBIT_T;


/*---------------------------------------------------------------------------------------------------------*/
/* Callback functions for UART IRQ handler                                                                 */
/*---------------------------------------------------------------------------------------------------------*/
typedef void (*UART_irq_callback_t)(u8 devNum, void *args);



/*---------------------------------------------------------------------------------------------------------*/
/* Default UART PORT configurations                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
#ifdef _PALLADIUM_
#define NPCMX50_SERIAL_BAUD     NPCMX50_UART_BAUDRATE_600
#else
#define NPCMX50_SERIAL_BAUD     NPCMX50_UART_BAUDRATE_115200
#endif
#define NPCMX50_SERIAL_BITS     8
#define NPCMX50_SERIAL_PARITY   'n'

/*---------------------------------------------------------------------------------------------------------*/
/* UART driver name and definitions                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
#define NPCMX50_SERIAL_NAME     "ttyS"
#define NPCMX50_SERIAL_MAJOR    4
#define NPCMX50_SERIAL_MINOR    64

/*---------------------------------------------------------------------------------------------------------*/
/* Default configurations for Rx and Tx FIFOs                                                              */
/*---------------------------------------------------------------------------------------------------------*/
#define NPCMX50_SERIAL_RX_TIMEOUT       0x20
#define NPCMX50_SERIAL_RX_THRES         NPCMX50_UART_RXFIFO_TRIGGER_1B
#define NPCMX50_SERIAL_TX_FIFO_SIZE     16

/*---------------------------------------------------------------------------------------------------------*/
/* Default configurations for Console                                                                      */
/*---------------------------------------------------------------------------------------------------------*/
#define NPCMX50_SERIAL_CONSOLE_PORT     NPCMX50_UART3_DEV




/*---------------------------------------------------------------------------------------------------------*/
/* Our port definitions and structures                                                                     */
/*---------------------------------------------------------------------------------------------------------*/
struct npcmX50_uart_port
{
    unsigned char      rx_claimed;
    unsigned char      tx_claimed;

    unsigned int       type;
    struct uart_port   port;
};



extern void npcmx50_gcr_mux_uart(unsigned int redirection_mode, bool CoreSP, 
bool sp1, bool sp2);



static int npcmx50_uart_init(NPCMX50_UART_DEV_T devNum, NPCMX50_UART_MUX_T muxMode, NPCMX50_UART_BAUDRATE_T baudRate);
static int npcmx50_uart_putc(NPCMX50_UART_DEV_T devNum, const u8 c );
#if 0
static u8 npcmx50_uart_getc( NPCMX50_UART_DEV_T devNum );
#endif
static int npcmx50_uart_putc_NB(NPCMX50_UART_DEV_T devNum, const u8 c );
static int npcmx50_uart_getc_NB( NPCMX50_UART_DEV_T devNum, u8* c );
static bool npcmx50_uart_test_rx( NPCMX50_UART_DEV_T devNum );
static bool npcmx50_uart_test_tx( NPCMX50_UART_DEV_T devNum );
       int npcmx50_uart_reset_fifo(NPCMX50_UART_DEV_T devNum, bool txFifo, bool rxFifo);
static int npcmx50_uart_set_rx_irq_state(NPCMX50_UART_DEV_T devNum, bool On);
// static int npcmx50_uart_set_tx_config(NPCMX50_UART_DEV_T devNum, u8 timeout, NPCMX50_UART_RXFIFO_TRIGGER_T triggerLevel);
static int npcmx50_uart_set_rx_config(NPCMX50_UART_DEV_T devNum, u8 timeout, NPCMX50_UART_RXFIFO_TRIGGER_T triggerLevel);
static int npcmx50_uart_set_parity(NPCMX50_UART_DEV_T devNum, NPCMX50_UART_PARITY_T parity);
static int npcmx50_uart_set_bits_per_char(NPCMX50_UART_DEV_T devNum, u32 bits);
static int npcmx50_uart_set_baud_rate(NPCMX50_UART_DEV_T devNum, NPCMX50_UART_BAUDRATE_T baudrate);
static int npcmx50_uart_set_stop_bit(NPCMX50_UART_DEV_T devNum, NPCMX50_UART_STOPBIT_T stopbit);
static int npcmx50_uart_set_break(NPCMX50_UART_DEV_T devNum, bool state);
static int npcmx50_uart_isr(NPCMX50_UART_DEV_T devNum,  UART_irq_callback_t rxCallback, void* rxParam, UART_irq_callback_t txCallback, void* txParam);
static void npcmX50_serial_stop_tx(struct uart_port *port);
static void npcmX50_serial_start_tx(struct uart_port *port);
static void npcmX50_serial_stop_rx(struct uart_port *port);
static void npcmX50_serial_tx_irq(u8 devNum, void* args);
static void npcmX50_serial_rx_irq(u8 devNum, void *args);
static irqreturn_t npcmX50_serial_irq(int irq, void *dev_id);
static unsigned int npcmX50_serial_tx_empty(struct uart_port *port);
static void npcmX50_serial_break_ctl(struct uart_port *port, int break_state);
static void npcmX50_serial_shutdown(struct uart_port *port);
static int npcmX50_serial_startup(struct uart_port *port);
static void npcmX50_serial_set_termios(struct uart_port * port, struct ktermios *new, struct ktermios *old);
static const char *npcmX50_serial_type(struct uart_port *port);
static void npcmX50_serial_config_port(struct uart_port *port, int flags);
static int npcmX50_serial_verify_port(struct uart_port *port, struct serial_struct *ser);
static void npcmX50_serial_set_mctrl(struct uart_port * port, unsigned int mctrl);
static unsigned int npcmX50_serial_get_mctrl(struct uart_port * port);
static void npcmX50_serial_flush_buffer(struct uart_port * port);
static void npcmX50_serial_set_ldisc(struct uart_port * port, struct ktermios * new);
static void npcmX50_serial_pm(struct uart_port * port, unsigned int state, unsigned int oldstate);
#if 0
static int npcmX50_serial_set_wake(struct uart_port * port, unsigned int state);
#endif
static void npcmX50_serial_release_port(struct uart_port * port);
static int npcmX50_serial_request_port(struct uart_port * port);
static int npcmX50_serial_init_port(struct npcmX50_uart_port *ourport, struct platform_device *platdev);
static int npcmX50_serial_probe(struct platform_device *dev);
static int npcmX50_serial_remove(struct platform_device *dev);
static int __init npcmX50_serial_modinit(void);
static void __exit npcmX50_serial_modexit(void);
static void npcmX50_console_putchar(struct uart_port *port, int ch);
static int __init npcmX50_console_setup(struct console *co, char *options);
static void npcmX50_console_write(struct console *co, const char *s, unsigned int count);
static int npcmX50_console_init(void);

#define regwrite8(mem, val) iowrite8(val, mem)
static inline void set_reg_field8(unsigned char __iomem *mem, bit_field_t bit_field, u8 val) {
    u8 tmp = ioread8(mem);
    tmp &= ~(((1 << bit_field.size) - 1) << bit_field.offset); // mask the field size
    tmp |= val << bit_field.offset;  // or with the requested value
    iowrite8(tmp, mem);
}
#define set_var_field(var, bit_field, value) { \
    typeof(var) tmp = var;                 \
    tmp &= ~(((1 << bit_field.size) - 1) << bit_field.offset); /* mask the field size */ \
    tmp |= value << bit_field.offset;  /* or with the requested value */               \
    var = tmp;                                                                         \
}
static inline u8 read_reg_field8(unsigned char __iomem *mem, bit_field_t bit_field) {
    u8 tmp = ioread8(mem);
    tmp = tmp >> bit_field.offset;     // shift right the offset
    tmp &= (1 << bit_field.size) - 1;  // mask the size
    return tmp;
}
#define read_var_field(var, bit_field) ({ \
    typeof(var) tmp = var;           \
    tmp = tmp >> bit_field.offset;     /* shift right the offset */ \
    tmp &= (1 << bit_field.size) - 1;  /* mask the size */          \
    tmp;                                                     \
})
#ifdef CONFIG_OF
static const struct of_device_id uart_dt_id[];
#endif

static struct platform_driver npcmX50_serial_drv;
static struct uart_port*      npcmX50_console_port;





/*---------------------------------------------------------------------------------------------------------*/
/* Forward declaration of driver structs                                                                   */
/*---------------------------------------------------------------------------------------------------------*/
static struct uart_ops          npcmX50_serial_ops;
static struct platform_driver   npcmX50_serial_drv;
static struct console           npcmX50_serial_console;



static struct npcmX50_uart_port npcmX50_serial_ports[NPCMX50_UART_NUM_OF_MODULES] =
{
    /*-----------------------------------------------------------------------------------------------------*/
    /* Uart0 port                                                                                          */
    /*-----------------------------------------------------------------------------------------------------*/
    {
        .port =
        {
            .lock       = __SPIN_LOCK_UNLOCKED(npcmX50_serial_ports[0].port.lock),
            .iotype     = UPIO_MEM,
            .fifosize   = NPCMX50_SERIAL_TX_FIFO_SIZE,
            .ops        = &npcmX50_serial_ops,
            .flags      = UPF_SPD_VHI | UPF_BOOT_AUTOCONF,
            .line       = 0,
        },
        .type = PORT_NPCMX50,
    },

    /*-----------------------------------------------------------------------------------------------------*/
    /* Uart1 port                                                                                          */
    /*-----------------------------------------------------------------------------------------------------*/
    {
        .port =
        {
            .lock       = __SPIN_LOCK_UNLOCKED(npcmX50_serial_ports[1].port.lock),
            .iotype     = UPIO_MEM,
            .fifosize   = NPCMX50_SERIAL_TX_FIFO_SIZE,
            .ops        = &npcmX50_serial_ops,
            .flags      = UPF_SPD_VHI | UPF_BOOT_AUTOCONF,
            .line       = 1,
        },
        .type = PORT_NPCMX50,
    },
    
    /*-----------------------------------------------------------------------------------------------------*/
    /* Uart2 port                                                                                          */
    /*-----------------------------------------------------------------------------------------------------*/
    {
        .port =
        {
            .lock       = __SPIN_LOCK_UNLOCKED(npcmX50_serial_ports[2].port.lock),
            .iotype     = UPIO_MEM,
            .fifosize   = NPCMX50_SERIAL_TX_FIFO_SIZE,
            .ops        = &npcmX50_serial_ops,
            .flags      = UPF_SPD_VHI | UPF_BOOT_AUTOCONF,
            .line       = 2,
        },
        .type = PORT_NPCMX50,
    },
    
        /*-----------------------------------------------------------------------------------------------------*/
    /* Uart3 port                                                                                          */
    /*-----------------------------------------------------------------------------------------------------*/
    {
        .port =
        {
            .lock       = __SPIN_LOCK_UNLOCKED(npcmX50_serial_ports[3].port.lock),
            .iotype     = UPIO_MEM,
            .fifosize   = NPCMX50_SERIAL_TX_FIFO_SIZE,
            .ops        = &npcmX50_serial_ops,
            .flags      = UPF_SPD_VHI | UPF_BOOT_AUTOCONF,
            .line       = 3,
        },
        .type = PORT_NPCMX50,
    },

    
};



/*---------------------------------------------------------------------------------------------------------*/
/*                                                 Macros                                                  */
/*---------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------------------------*/
/* Interrupt handling                                                                                      */
/*---------------------------------------------------------------------------------------------------------*/
#define tx_enabled(port)    ((port)->unused[0])
#define rx_enabled(port)    ((port)->unused[1])

#define port_lock(port)     { unsigned long flags; spin_lock_irqsave(&port->lock, flags);
#define port_unlock(port)     spin_unlock_irqrestore(&port->lock, flags); }


/*---------------------------------------------------------------------------------------------------------*/
/* conversion functions between various structures                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#define npcmX50_convert_dev_to_port(__dev)      (struct uart_port *)dev_get_drvdata(__dev)
#define npcmX50_convert_port_to_ourport(port)   container_of(port, struct npcmX50_uart_port, port)
#define npcmX50_convert_port_to_portname(port)  to_platform_device(port->dev)->name


/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                          UART PORT driver code                                          */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/





/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmx50_uart_init                                                                      */
/*                                                                                                         */
/* Parameters:      devNum - uart module number                                                            */
/*                  muxMode - configuration mode (last setting is the one that is active)                  */
/*                  baudRate - BAUD for the UART module                                                    */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine performs UART initialization                                              */
/*---------------------------------------------------------------------------------------------------------*/
static int npcmx50_uart_init(NPCMX50_UART_DEV_T devNum, NPCMX50_UART_MUX_T muxMode, NPCMX50_UART_BAUDRATE_T baudRate)
{
    u32 FCR_Val      = 0;

    bool CoreSP  = false;
    bool sp1     = false;
    bool sp2     = false;
    u32  ret     = 0;

    /*-----------------------------------------------------------------------------------------------------*/
    /* Parameters check                                                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
    if (devNum >= NPCMX50_UART_NUM_OF_MODULES)
    {
        return -EINVAL;
    }

    /*-----------------------------------------------------------------------------------------------------*/
    /* Reseting the module                                                                                 */
    /*-----------------------------------------------------------------------------------------------------*/
    // removed since resets all UARTS
    //CLK_ResetUART(devNum); 

    /*-----------------------------------------------------------------------------------------------------*/
    /* Muxing for UART0                                                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
    if (devNum == NPCMX50_UART0_DEV)
    {
        CoreSP = true;
    }

#if defined NPCM650
    /*-----------------------------------------------------------------------------------------------------*/
    /* Muxing for UART1                                                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
    else if (devNum == NPCMX50_UART1_DEV)
    {
        CoreSP = false;

        switch (muxMode)
        {
            case NPCMX50_UART_MUX_CORE_SNOOP:
            case NPCMX50_UART_MUX_CORE_TAKEOVER:
                {
                    sp1 = true;
                    sp2 = true;
                    break;
                }
            case NPCMX50_UART_MUX_CORE_SP2__SP1_SI1:
                {
                    sp1 = true;
                    break;
                }
            case NPCMX50_UART_MUX_CORE_SP2__SP1_SI2:
                {
                    sp2= true;
                    break;
                }

	        case NPCMX50_UART_MUX_SKIP_CONFIG:
	            {
	                /* Do nothing. Don't call CHIP_Mux_UART. Assuming it was called before */
	                break;
	            }

            /*---------------------------------------------------------------------------------------------*/
            /* Illegal mux mode                                                                            */
            /*---------------------------------------------------------------------------------------------*/
            default: return HAL_ERROR_BAD_PARAM;
        }
    }
#elif (defined NPCM750_CHIP || defined NPCM750_CP)
    /*-----------------------------------------------------------------------------------------------------*/
    /* Enable serial interfaces according to mux mode                                                      */
    /*-----------------------------------------------------------------------------------------------------*/
    switch (muxMode)
    {
        case NPCMX50_UART_MUX_MODE4_HSP1_SI1____HSP2_SI2____UART1_s_SI1___UART3_s_SI2__UART2_s_HSP1:
        case NPCMX50_UART_MUX_MODE6_HSP1_SI1____HSP2_SI2____UART1_s_SI1___UART3_s_SI2__UART2_s_HSP2:
        case NPCMX50_UART_MUX_MODE7_HSP1_SI1____HSP2_UART2__UART1_s_HSP1__UART3_SI2:
            {
                sp1 = true;
                sp2 = true;
                break;
            }
        case NPCMX50_UART_MUX_MODE5_HSP1_SI1____HSP2_UART2__UART1_s_HSP1__UART3_s_SI1:
            {
                sp1 = true;
                break;
            }
        case NPCMX50_UART_MUX_MODE1_HSP1_SI2____HSP2_UART2__UART1_s_HSP1__UART3_s_SI2:
        case NPCMX50_UART_MUX_MODE2_HSP1_UART1__HSP2_SI2____UART2_s_HSP2__UART3_s_SI2:
        case NPCMX50_UART_MUX_MODE3_HSP1_UART1__HSP2_UART2__UART3_SI2:
            {
                sp2= true;
                break;
            }

        case NPCMX50_UART_MUX_SKIP_CONFIG:
            {
                /* Do nothing. Don't call CHIP_Mux_UART. Assuming it was called before */
                break;
            }

        /*---------------------------------------------------------------------------------------------*/
        /* Illegal mux mode                                                                            */
        /*---------------------------------------------------------------------------------------------*/
        default: return -1;
    }
#endif

    if (muxMode != NPCMX50_UART_MUX_SKIP_CONFIG)
    {
        if (muxMode < 7)
        {      
            /*regmap_update_bits(gcr_regmap, SPSWC_OFFSET, (0x7 << 0)
                    , muxMode & 0x7);

            if (CoreSP)
            {
                regmap_update_bits(gcr_regmap, MFSEL1_OFFSET, (0x1 << 9), 1);
                regmap_update_bits(gcr_regmap, MFSEL4_OFFSET, (0x1 << 1), 1);
            }
            if (sp1)
            {
                regmap_update_bits(gcr_regmap, MFSEL1_OFFSET, (0x1 << 10), 1);
                regmap_update_bits(gcr_regmap, MFSEL4_OFFSET, (0x1 << 1), 0);
            }
            if (sp2)
            {
                regmap_update_bits(gcr_regmap, MFSEL1_OFFSET, (0x1 << 11), 1);
                regmap_update_bits(gcr_regmap, MFSEL4_OFFSET, (0x1 << 1), 0);
            }*/
        }
    }


    /*-----------------------------------------------------------------------------------------------------*/
    /* Disable interrupts                                                                                  */
    /*-----------------------------------------------------------------------------------------------------*/
    regwrite8(UART_LCR(devNum), 0);            // prepare to Init UART
    regwrite8(UART_IER(devNum), 0x0);          // Disable all UART interrupt

    /*-----------------------------------------------------------------------------------------------------*/
    /* Set baudrate                                                                                        */
    /*-----------------------------------------------------------------------------------------------------*/
    ret += npcmx50_uart_set_baud_rate(devNum, baudRate);

    /*-----------------------------------------------------------------------------------------------------*/
    /* Set port for 8 bit, 1 stop, no parity                                                               */
    /*-----------------------------------------------------------------------------------------------------*/
    ret += npcmx50_uart_set_bits_per_char(devNum, 8);
    ret += npcmx50_uart_set_stop_bit(devNum, NPCMX50_UART_STOPBIT_1);
    ret += npcmx50_uart_set_parity(devNum, NPCMX50_UART_PARITY_NONE);

    /*-----------------------------------------------------------------------------------------------------*/
    /* Set the RX FIFO trigger level, reset RX, TX FIFO                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
    FCR_Val = 0;
    set_var_field(FCR_Val, FCR_RFITL, FCR_RFITL_4B);
    set_var_field(FCR_Val, FCR_TFR, 1);
    set_var_field(FCR_Val, FCR_RFR, 1);
    set_var_field(FCR_Val, FCR_FME, 1);

    regwrite8(UART_FCR(devNum), FCR_Val);
    regwrite8(UART_TOR(devNum), 0x0);

    if (ret > 0)
        return -1;
    else
        return 0;

}




/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmx50_uart_putc                                                                              */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  c - char to write to UART                                                              */
/*                                                                                                         */
/* Returns:         none                                                                                   */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine write single char to UART                                                 */
/*                  Note that the function is blocking till char can be send                               */
/*---------------------------------------------------------------------------------------------------------*/
static int npcmx50_uart_putc(NPCMX50_UART_DEV_T devNum, const u8 c )
{
    /*-----------------------------------------------------------------------------------------------------*/
    /* Parameters check                                                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
    if (devNum >= NPCMX50_UART_NUM_OF_MODULES)
    {
        return -EINVAL;
    }

    /*-----------------------------------------------------------------------------------------------------*/
    /* wait until Tx ready                                                                                 */
    /*-----------------------------------------------------------------------------------------------------*/
    while (!read_reg_field8(UART_LSR(devNum), LSR_THRE));


    /*-----------------------------------------------------------------------------------------------------*/
    /* Put the char                                                                                        */
    /*-----------------------------------------------------------------------------------------------------*/
    regwrite8(UART_THR(devNum), (c & 0xFF));

    return 0;
}




#if 0
/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmx50_uart_getc                                                                      */
/*                                                                                                         */
/* Parameters:      none                                                                                   */
/* Returns:                                                                                                */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine reads char from UART                                                      */
/*                  Note that the function is blocking till char is available                              */
/*---------------------------------------------------------------------------------------------------------*/
static u8 npcmx50_uart_getc( NPCMX50_UART_DEV_T devNum )
{
    u8 Ch;

    /*-----------------------------------------------------------------------------------------------------*/
    /* Parameters check                                                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
    if (devNum >= NPCMX50_UART_NUM_OF_MODULES)
    {
        return 0;
    }

    /*-----------------------------------------------------------------------------------------------------*/
    /* wait until char is available                                                                        */
    /*-----------------------------------------------------------------------------------------------------*/
    while (!npcmx50_uart_test_rx(devNum));

    /*-----------------------------------------------------------------------------------------------------*/
    /* Reading the char                                                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
    Ch = ioread8(UART_RBR(devNum)) & 0xFF;

    return Ch;
}
#endif


/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmx50_uart_putc_NB                                                                           */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  c - char to write to UART                                                              */
/*                                                                                                         */
/* Returns:         0 if the char was written or error if it couldn't be written                      */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine write single char to UART in NON-Blocking manner                          */
/*---------------------------------------------------------------------------------------------------------*/
static int npcmx50_uart_putc_NB(NPCMX50_UART_DEV_T devNum, const u8 c )
{
    /*-----------------------------------------------------------------------------------------------------*/
    /* Parameters check                                                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
    if (devNum >= NPCMX50_UART_NUM_OF_MODULES)
    {
        return -EINVAL;
    }

    /*-----------------------------------------------------------------------------------------------------*/
    /* Put the char                                                                                        */
    /*-----------------------------------------------------------------------------------------------------*/
    regwrite8(UART_THR(devNum), (c & 0xFF));

    return 0;
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmx50_uart_getc_NB                                                                           */
/*                                                                                                         */
/* Parameters:      none                                                                                   */
/* Returns:         0 if char was read or error if it no char was available                           */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine reads char from UART in NON-Blocking manner                               */
/*---------------------------------------------------------------------------------------------------------*/
static int npcmx50_uart_getc_NB( NPCMX50_UART_DEV_T devNum, u8* c )
{
    /*-----------------------------------------------------------------------------------------------------*/
    /* Parameters check                                                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
    if (devNum >= NPCMX50_UART_NUM_OF_MODULES)
    {
        return -EINVAL;
    }

    /*-----------------------------------------------------------------------------------------------------*/
    /* wait until char is available                                                                        */
    /*-----------------------------------------------------------------------------------------------------*/
    if (!npcmx50_uart_test_rx(devNum))
    {
        return -1 ; // HAL_ERROR_QUEUE_EMPTY;
    }

    /*-----------------------------------------------------------------------------------------------------*/
    /* Reading the char                                                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
    *c = (ioread8(UART_RBR(devNum)) & 0xFF);

    return 0;
}




/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmx50_uart_test_rx                                                                            */
/*                                                                                                         */
/* Parameters:      none                                                                                   */
/* Returns:                                                                                                */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine test if there is a char in RX fifo                                        */
/*---------------------------------------------------------------------------------------------------------*/
static bool npcmx50_uart_test_rx( NPCMX50_UART_DEV_T devNum )
{

    if (read_reg_field8(UART_LSR(devNum), LSR_RFDR))
        return true;
    else
        return false;
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmx50_uart_test_tx                                                                            */
/*                                                                                                         */
/* Parameters:      none                                                                                   */
/* Returns:                                                                                                */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine test if there is a char in TX fifo                                        */
/*---------------------------------------------------------------------------------------------------------*/
static bool npcmx50_uart_test_tx( NPCMX50_UART_DEV_T devNum )
{
    if (!read_reg_field8(UART_LSR(devNum), LSR_THRE))
        return true;
    else
        return false;
}



/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmx50_uart_reset_fifo                                                                        */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  devNum -                                                                               */
/*                  rxFifo - if true RX fifo is reseted                                                    */
/*                  txFifo - if true TX fifo is reseted                                                    */
/*                                                                                                         */
/* Returns:         none                                                                                   */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine performs FIFO reset                                                       */
/*---------------------------------------------------------------------------------------------------------*/
int npcmx50_uart_reset_fifo(NPCMX50_UART_DEV_T devNum, bool txFifo, bool rxFifo)
{
    /*-----------------------------------------------------------------------------------------------------*/
    /* Parameters check                                                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
    if (devNum >= NPCMX50_UART_NUM_OF_MODULES)
    {
        return -EINVAL;
    }

    /*-----------------------------------------------------------------------------------------------------*/
    /* Reseting fifos                                                                                      */
    /*-----------------------------------------------------------------------------------------------------*/
    if (txFifo)
    {
        set_reg_field8(UART_FCR(devNum), FCR_TFR, 1);
    }

    if (rxFifo)
    {
        set_reg_field8(UART_FCR(devNum), FCR_RFR, 1);
    }

    return 0;
}






/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmx50_uart_set_tx_irq_state                                                                     */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  devNum -                                                                               */
/*                  On -                                                                                   */
/*                                                                                                         */
/* Returns:         none                                                                                   */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine enables/disables Tx Interrupt                                             */
/*---------------------------------------------------------------------------------------------------------*/
static int npcmx50_uart_set_tx_irq_state(NPCMX50_UART_DEV_T devNum, bool On)
{
    /*-----------------------------------------------------------------------------------------------------*/
    /* Parameters check                                                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
    if (devNum >= NPCMX50_UART_NUM_OF_MODULES)
    {
        return -EINVAL;
    }

    /*-----------------------------------------------------------------------------------------------------*/
    /* Setting Tx State                                                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
    if (On)
    {
        set_reg_field8(UART_IER(devNum), IER_THREIE, 1);
    }
    else
    {
        set_reg_field8(UART_IER(devNum), IER_THREIE, 0);
    }

    return 0;
}



/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmx50_uart_set_rx_irq_state                                                                     */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  devNum -                                                                               */
/*                  On -                                                                                   */
/*                  timeout -                                                                              */
/*                  triggerLevel -                                                                         */
/*                                                                                                         */
/* Returns:         none                                                                                   */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine performs Rx interrupt enable/disable and configuration                    */
/*---------------------------------------------------------------------------------------------------------*/
static int npcmx50_uart_set_rx_irq_state(NPCMX50_UART_DEV_T devNum, bool On)
{
    /*-----------------------------------------------------------------------------------------------------*/
    /* Parameters check                                                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
    if (devNum >= NPCMX50_UART_NUM_OF_MODULES)
    {
        return -EINVAL;
    }

    /*-----------------------------------------------------------------------------------------------------*/
    /* Setting Rx state                                                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
    if (On)
    {
        set_reg_field8(UART_IER(devNum), IER_RDAIE, 1);
        set_reg_field8(UART_TOR(devNum), TOR_TOIE, 1);
    }
    else
    {
        set_reg_field8(UART_IER(devNum), IER_RDAIE, 0);
        set_reg_field8(UART_TOR(devNum), TOR_TOIE, 0);
    }

    return 0;
}



/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmx50_uart_set_rx_config                                                                       */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  timeout -                                                                              */
/*                  triggerLevel -                                                                         */
/*                                                                                                         */
/* Returns:         none                                                                                   */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine performs Rx irq configurations                                            */
/*---------------------------------------------------------------------------------------------------------*/
static int npcmx50_uart_set_rx_config(NPCMX50_UART_DEV_T devNum, u8 timeout, NPCMX50_UART_RXFIFO_TRIGGER_T triggerLevel)
{
    /*-----------------------------------------------------------------------------------------------------*/
    /* Parameters check                                                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
    if (devNum >= NPCMX50_UART_NUM_OF_MODULES)
    {
        return -EINVAL;
    }

    /*-----------------------------------------------------------------------------------------------------*/
    /* Setting Rx interrupt timeout                                                                        */
    /*-----------------------------------------------------------------------------------------------------*/
    set_reg_field8(UART_TOR(devNum), TOR_TOIC, (timeout & 0x7F));

    /*-----------------------------------------------------------------------------------------------------*/
    /* Setting Rx interrupt FIFO trigger level                                                             */
    /*-----------------------------------------------------------------------------------------------------*/
    set_reg_field8(UART_FCR(devNum), FCR_RFITL, (triggerLevel<<2));

    return 0;
}




/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmx50_uart_set_parity                                                                         */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  devNum -                                                                               */
/*                  parity -                                                                               */
/*                                                                                                         */
/* Returns:         none                                                                                   */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine sets parity configuration                                                 */
/*---------------------------------------------------------------------------------------------------------*/
static int npcmx50_uart_set_parity(NPCMX50_UART_DEV_T devNum, NPCMX50_UART_PARITY_T parity)
{
    /*-----------------------------------------------------------------------------------------------------*/
    /* Parameters check                                                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
    if (devNum >= NPCMX50_UART_NUM_OF_MODULES)
    {
        return -EINVAL;
    }

    if (parity != NPCMX50_UART_PARITY_NONE)
    {
        /*-------------------------------------------------------------------------------------------------*/
        /* Parity enable, choosing type                                                                    */
        /*-------------------------------------------------------------------------------------------------*/
        set_reg_field8(UART_LCR(devNum), LCR_PBE, 1);

        if (parity == NPCMX50_UART_PARITY_EVEN)
        {
            set_reg_field8(UART_LCR(devNum), LCR_EPE, 1);

        }
        else if (parity == NPCMX50_UART_PARITY_ODD)
        {
            set_reg_field8(UART_LCR(devNum), LCR_EPE, 0);
        }
        else
        {
            /*---------------------------------------------------------------------------------------------*/
            /* Unknown parity type                                                                         */
            /*---------------------------------------------------------------------------------------------*/
            return -EINVAL;
        }

    }

    /*-----------------------------------------------------------------------------------------------------*/
    /* No parity                                                                                           */
    /*-----------------------------------------------------------------------------------------------------*/
    else
    {
        set_reg_field8(UART_LCR(devNum), LCR_PBE, 0);
    }

    return 0;
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmx50_uart_set_bits_per_char                                                                    */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  bits -                                                                                 */
/*                  devNum -                                                                               */
/*                                                                                                         */
/* Returns:         none                                                                                   */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine set bits per char                                                         */
/*---------------------------------------------------------------------------------------------------------*/
static int npcmx50_uart_set_bits_per_char(NPCMX50_UART_DEV_T devNum, u32 bits)
{
    /*-----------------------------------------------------------------------------------------------------*/
    /* Parameters check                                                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
    if (devNum >= NPCMX50_UART_NUM_OF_MODULES)
    {
        return -EINVAL;
    }

    switch (bits)
    {
        case 5:   set_reg_field8(UART_LCR(devNum), LCR_WLS, LCR_WLS_5bit);   break;
        case 6:   set_reg_field8(UART_LCR(devNum), LCR_WLS, LCR_WLS_6bit);   break;
        case 7:   set_reg_field8(UART_LCR(devNum), LCR_WLS, LCR_WLS_7bit);   break;
        default:
        case 8:   set_reg_field8(UART_LCR(devNum), LCR_WLS, LCR_WLS_8bit);   break;
    }

    return 0;
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmx50_uart_set_baud_rate                                                                       */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  baudrate -                                                                             */
/*                  devNum -                                                                               */
/*                                                                                                         */
/* Returns:         none                                                                                   */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine sets new baudrate                                                         */
/*---------------------------------------------------------------------------------------------------------*/
static int npcmx50_uart_set_baud_rate(NPCMX50_UART_DEV_T devNum, NPCMX50_UART_BAUDRATE_T baudrate)
{
    int               divisor     = 0;
    u32               uart_clock  = 0;
    int               ret         = 0;

    /*-----------------------------------------------------------------------------------------------------*/
    /* Parameters check                                                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
    if (devNum >= NPCMX50_UART_NUM_OF_MODULES)
    {
        return -EINVAL;
    }

    /*-----------------------------------------------------------------------------------------------------*/
    /* Configuring UART clock                                                                              */
    /*-----------------------------------------------------------------------------------------------------*/
    if (npcmX50_serial_ports[devNum].port.uartclk != 0 )
        uart_clock =  npcmX50_serial_ports[devNum].port.uartclk   ;      // CLK_ConfigureUartClock();
    else{
		NPCMX50_SERIAL_MSG("%s: warning, uart clock unknown!\n", __FUNCTION__);
		uart_clock = 24*_1MHz_;  
	}		

    // uart_clock = CLK_ConfigureUartClock();
    /*-----------------------------------------------------------------------------------------------------*/
    /* Computing the divisor for the given baudrate.                                                       */
    /*-----------------------------------------------------------------------------------------------------*/
    divisor = ((int)uart_clock / ((int)baudrate * 16)) - 2;

    // since divisor is rounded down check if it is better when rounded up
    if ( ((int)uart_clock / (16 * (divisor + 2)) - (int)baudrate) > 
         ((int)baudrate - (int)uart_clock / (16 * ((divisor+1) + 2))) )
    {
        divisor++;
    }

    if (divisor < 0 )
    {
        divisor = 0;
        ret = EINVAL;
    }

    /*-----------------------------------------------------------------------------------------------------*/
    /* Set baud rate to baudRate bps                                                                       */
    /*-----------------------------------------------------------------------------------------------------*/
    set_reg_field8(UART_LCR(devNum), LCR_DLAB, 1);    // prepare to access Divisor
    regwrite8(UART_DLL(devNum), LSB(divisor));
    regwrite8(UART_DLM(devNum), MSB(divisor));
    set_reg_field8(UART_LCR(devNum), LCR_DLAB, 0);   // prepare to access RBR, THR, IER

    return ret;


}




/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmx50_uart_set_stop_bit                                                                        */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  devNum -                                                                               */
/*                  stopbit -                                                                              */
/*                                                                                                         */
/* Returns:         none                                                                                   */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine sets number of stopbits                                                   */
/*---------------------------------------------------------------------------------------------------------*/
static int npcmx50_uart_set_stop_bit(NPCMX50_UART_DEV_T devNum, NPCMX50_UART_STOPBIT_T stopbit)
{
    /*-----------------------------------------------------------------------------------------------------*/
    /* Parameters check                                                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
    if (devNum >= NPCMX50_UART_NUM_OF_MODULES)
    {
        return -EINVAL;
    }

    if (stopbit == NPCMX50_UART_STOPBIT_1)
    {
        set_reg_field8(UART_LCR(devNum), LCR_NSB, 0);
    }
    else if (stopbit == NPCMX50_UART_STOPBIT_DYNAMIC)
    {
        set_reg_field8(UART_LCR(devNum), LCR_NSB, 1);
    }
    else
    {
        /*-------------------------------------------------------------------------------------------------*/
        /* Unknown stopbits configuration                                                                  */
        /*------------------------------------------------------------------------------------------------*/
        return -EINVAL;
    }

    return 0;
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmx50_uart_set_break                                                                          */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  devNum -                                                                               */
/*                                                                                                         */
/* Returns:         none                                                                                   */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine sets break on the given UART                                              */
/*---------------------------------------------------------------------------------------------------------*/
static int npcmx50_uart_set_break(NPCMX50_UART_DEV_T devNum, bool state)
{
    /*-----------------------------------------------------------------------------------------------------*/
    /* Parameters check                                                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
    if (devNum >= NPCMX50_UART_NUM_OF_MODULES)
    {
        return -EINVAL;
    }

    if (state)
    {
        set_reg_field8(UART_LCR(devNum), LCR_BCB, 1);
    }
    else
    {
        set_reg_field8(UART_LCR(devNum), LCR_BCB, 0);
    }

    return 0;
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmx50_uart_isr                                                                               */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  devNum -                                                                               */
/*                  rxCallback -                                                                           */
/*                  rxParam -                                                                              */
/*                  txCallback -                                                                           */
/*                  txParam -                                                                              */
/*                                                                                                         */
/* Returns:                                                                                                */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine performs IRQ handling                                                     */
/*---------------------------------------------------------------------------------------------------------*/
static int npcmx50_uart_isr(NPCMX50_UART_DEV_T devNum,  UART_irq_callback_t rxCallback, void* rxParam,
                                        UART_irq_callback_t txCallback, void* txParam)
{
    int  ret = 0;
    u32      iir = ioread8(UART_IIR(devNum)) & 0xF;

    /*-----------------------------------------------------------------------------------------------------*/
    /* Checking if we got any interrupts at all                                                            */
    /*-----------------------------------------------------------------------------------------------------*/
    if (read_var_field(iir, IIR_NIP))
    {
        /*-------------------------------------------------------------------------------------------------*/
        /* if no interrupts actually occurred, we return "not handled"                                     */
        /*-------------------------------------------------------------------------------------------------*/
        ret =  -ENXIO;  // IRQ_NONE;
    }
    else
    {
        switch (read_var_field(iir, IIR_IID))
        {
            /*---------------------------------------------------------------------------------------------*/
            /* We don't support modem interrups yet                                                        */
            /*---------------------------------------------------------------------------------------------*/
            case IIR_IID_MODEM:                                     break;

            /*---------------------------------------------------------------------------------------------*/
            /* Tx Interrupt                                                                                */
            /*---------------------------------------------------------------------------------------------*/
            case IIR_IID_THRE:      txCallback(devNum, txParam);    break;


            /*---------------------------------------------------------------------------------------------*/
            /* Rx Interrupts                                                                               */
            /*---------------------------------------------------------------------------------------------*/
            case IIR_IID_TOUT:
            case IIR_IID_RDA:       rxCallback(devNum, rxParam);    break;


            /*---------------------------------------------------------------------------------------------*/
            /* WE should never get here                                                                    */
            /*---------------------------------------------------------------------------------------------*/
            default:                                                break;

            /*---------------------------------------------------------------------------------------------*/
            /* Error interrupts                                                                            */
            /*---------------------------------------------------------------------------------------------*/
            case IIR_IID_RLS:
            {
                u32 lsr = ioread8(UART_LSR(devNum));
                if      (read_var_field(lsr, LSR_OEI))
                    ret = -EOVERFLOW;
                else if (read_var_field(lsr, LSR_PEI))
                    ret = -EILSEQ          ; // HAL_ERROR_BAD_PARITY;
                else if (read_var_field(lsr, LSR_FEI))
                    ret = -EMSGSIZE        ; // HAL_ERROR_BAD_FRAME;
                else if (read_var_field(lsr, LSR_BII))
                    ret = -EIO; // break
                else
                    ret = -1;

                break;
            }
        }
    }

    return ret;
}
















/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmX50_serial_stop_tx                                                                 */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  port -                                                                                 */
/*                                                                                                         */
/* Returns:         none                                                                                   */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine stops TX IRQ                                                              */
/*---------------------------------------------------------------------------------------------------------*/
static void npcmX50_serial_stop_tx(struct uart_port *port)
{

    if (tx_enabled(port))
    {
        npcmx50_uart_set_tx_irq_state((NPCMX50_UART_DEV_T)port->line, false);
        tx_enabled(port) = 0;
    }

}



/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmX50_serial_start_tx                                                                */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  port -                                                                                 */
/*                                                                                                         */
/* Returns:         none                                                                                   */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine starts TX IRQ                                                             */
/*---------------------------------------------------------------------------------------------------------*/
static void npcmX50_serial_start_tx(struct uart_port *port)
{
	struct circ_buf *xmit = &port->state->xmit;
	
    if ((!tx_enabled(port))&&(!uart_circ_empty(xmit)))
    {
        npcmx50_uart_set_tx_irq_state((NPCMX50_UART_DEV_T)port->line, true);
        tx_enabled(port) = 1;
    }

}


/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmX50_serial_stop_rx                                                                 */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  port -                                                                                 */
/*                                                                                                         */
/* Returns:         none                                                                                   */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine stop RX IRQ                                                               */
/*---------------------------------------------------------------------------------------------------------*/
static void npcmX50_serial_stop_rx(struct uart_port *port)
{

    if (rx_enabled(port))
    {
        npcmx50_uart_set_rx_irq_state((NPCMX50_UART_DEV_T)port->line, false);
        rx_enabled(port) = 0;
    }

}


/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmX50_serial_tx_irq                                                                  */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  args -                                                                                 */
/*                  devNum -                                                                               */
/*                                                                                                         */
/* Returns:         none                                                                                   */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine handles TX IRQ                                                            */
/*---------------------------------------------------------------------------------------------------------*/
static void npcmX50_serial_tx_irq(u8 devNum, void* args)
{
    struct npcmX50_uart_port *ourport = (struct npcmX50_uart_port *)args;
    struct uart_port *port = &ourport->port;
    struct circ_buf *xmit = &port->state->xmit;

    int count = NPCMX50_SERIAL_TX_FIFO_SIZE;

    /*-----------------------------------------------------------------------------------------------------*/
    /* Handle X-Char                                                                                       */
    /*-----------------------------------------------------------------------------------------------------*/
    if (port->x_char)
    {
        if (!npcmx50_uart_test_tx((NPCMX50_UART_DEV_T)port->line))
        {
            npcmx50_uart_putc_NB((NPCMX50_UART_DEV_T)port->line, port->x_char);
            port->icount.tx++;
            port->x_char = 0;
        }

        return;
    }

    /*-----------------------------------------------------------------------------------------------------*/
    /* if there isnt anything more to transmit, or the uart is now                                         */
    /* stopped, disable the uart and exit                                                                  */
    /*-----------------------------------------------------------------------------------------------------*/
    if (uart_circ_empty(xmit) || uart_tx_stopped(port))
    {
        npcmX50_serial_stop_tx(port);
        return;
    }

    /*-----------------------------------------------------------------------------------------------------*/
    /* try and drain the buffer                                                                            */
    /*-----------------------------------------------------------------------------------------------------*/
    while (!uart_circ_empty(xmit) && count-- > 0)
    {
//        if (!npcmx50_uart_test_tx(port->line))
        {
            npcmx50_uart_putc_NB((NPCMX50_UART_DEV_T)port->line, xmit->buf[xmit->tail]);
            xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
            port->icount.tx++;
        }
    }

    /*-----------------------------------------------------------------------------------------------------*/
    /* if the queue is not empty, we schedule late processing after the interrupt                          */
    /*-----------------------------------------------------------------------------------------------------*/
    if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
    {
        uart_write_wakeup(port);
    }

    /*-----------------------------------------------------------------------------------------------------*/
    /* if the queue is empty we stop TX                                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
    if (uart_circ_empty(xmit))
    {
        npcmX50_serial_stop_tx(port);
    }
}





/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmX50_serial_rx_irq                                                                  */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  args -                                                                                 */
/*                  devNum -                                                                               */
/*                                                                                                         */
/* Returns:         none                                                                                   */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine handles RX IRQ                                                            */
/*---------------------------------------------------------------------------------------------------------*/
static void npcmX50_serial_rx_irq(u8 devNum, void *args)
{
    struct npcmX50_uart_port *ourport   = (struct npcmX50_uart_port *)args;
    struct uart_port *port              = &ourport->port;
//    struct tty_struct *tty              = port->state->port.tty;

    unsigned char   ch;
    int             status;
    int             max_count = NPCMX50_SERIAL_TX_FIFO_SIZE;

    /*-----------------------------------------------------------------------------------------------------*/
    /* We read from uart as long as we have chars in RX                                                    */
    /* and we don't exceed the maximum count (to prevent starvation)                                       */
    /*-----------------------------------------------------------------------------------------------------*/
    while ((max_count-- > 0) && npcmx50_uart_test_rx(devNum))
    {
        /*-------------------------------------------------------------------------------------------------*/
        /* Reading the CHAR                                                                                */
        /*-------------------------------------------------------------------------------------------------*/
        status = npcmx50_uart_getc_NB(devNum, &ch);

        /*-------------------------------------------------------------------------------------------------*/
        /* insert the character into the buffer                                                            */
        /*-------------------------------------------------------------------------------------------------*/
        port->icount.rx++;
        if (!uart_handle_sysrq_char(port, ch) && (status == 0))
        {
            uart_insert_char(port, 0, 0, ch, TTY_NORMAL);
        }
    }

    tty_flip_buffer_push(&port->state->port);
}




/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmX50_serial_irq                                                                     */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  dev_id -                                                                               */
/*                  irq -                                                                                  */
/*                                                                                                         */
/* Returns:                                                                                                */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine is main UART IRQ Handler                                                  */
/*---------------------------------------------------------------------------------------------------------*/
static irqreturn_t npcmX50_serial_irq(int irq, void *dev_id)
{
    struct npcmX50_uart_port*   ourport = (struct npcmX50_uart_port *)dev_id;
    struct uart_port*           port    = &ourport->port;
    int                         ret;
//    int flag = TTY_NORMAL;
//    unsigned long irq_flags;

    /*-----------------------------------------------------------------------------------------------------*/
    /* Using UART module IRQ hadler to call the correct Rx/Tx handler                                      */
    /*-----------------------------------------------------------------------------------------------------*/
    ret = npcmx50_uart_isr((NPCMX50_UART_DEV_T)port->line, npcmX50_serial_rx_irq, dev_id,       \
                               npcmX50_serial_tx_irq, dev_id );


    /*-----------------------------------------------------------------------------------------------------*/
    /* If IRQ was not handled we were called by mistake                                                    */
    /* (In case of multiple modules on same interrupt line, we must mark that this is not our interrupt by */
    /* returning IRQ_NONE)                                                                                 */
    /*-----------------------------------------------------------------------------------------------------*/
    if (ret == (-ENXIO))
    {
        return IRQ_NONE;
    }

    /*-----------------------------------------------------------------------------------------------------*/
    /* if we got an error, handle it                                                                       */
    /*-----------------------------------------------------------------------------------------------------*/
    else if (ret != 0)
    {
        NPCMX50_SERIAL_MSG("NPCMX50 Serial: IRQ Error\n");


        if (ret == (-EIO))
        {
            NPCMX50_SERIAL_MSG("NPCMX50 Serial: break!\n");
            port->icount.brk++;
//            flag = TTY_BREAK;

            uart_handle_break(port);
        }

        else if (ret == (-EMSGSIZE))
        {
            port->icount.frame++;
            NPCMX50_SERIAL_MSG("NPCMX50 Serial: frame error!\n");
        }
        else if (ret == (-EOVERFLOW))
        {
            port->icount.overrun++;
            NPCMX50_SERIAL_MSG("NPCMX50 Serial: overrun error!\n");
        }
        else if (ret == (-EILSEQ))
        {
            port->icount.parity++;
            NPCMX50_SERIAL_MSG("NPCMX50 Serial: parity error!\n");
        }
    }

    /*-----------------------------------------------------------------------------------------------------*/
    /* We completed the task successfully                                                                  */
    /*-----------------------------------------------------------------------------------------------------*/
    return IRQ_HANDLED;

}



/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmX50_serial_tx_empty                                                                */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  port -                                                                                 */
/*                                                                                                         */
/* Returns:                                                                                                */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine performs returns true if TX fifo is empty                                 */
/*---------------------------------------------------------------------------------------------------------*/
static unsigned int npcmX50_serial_tx_empty(struct uart_port *port)
{
    return !npcmx50_uart_test_tx((NPCMX50_UART_DEV_T)port->line);
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmX50_serial_break_ctl                                                               */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  break_state -                                                                          */
/*                  port -                                                                                 */
/*                                                                                                         */
/* Returns:         none                                                                                   */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine handles break condition change                                            */
/*---------------------------------------------------------------------------------------------------------*/
static void npcmX50_serial_break_ctl(struct uart_port *port, int break_state)
{
    port_lock(port);

    npcmx50_uart_set_break((NPCMX50_UART_DEV_T)port->line, break_state);

    port_unlock(port);
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmX50_serial_shutdown                                                                */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  port -                                                                                 */
/*                                                                                                         */
/* Returns:         none                                                                                   */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine performs port uninitialization                                            */
/*---------------------------------------------------------------------------------------------------------*/
static void npcmX50_serial_shutdown(struct uart_port *port)
{
    struct npcmX50_uart_port *ourport = npcmX50_convert_port_to_ourport(port);

    NPCMX50_SERIAL_MSG("NPCMX50 Serial: Called function %s\n", __FUNCTION__);

    /*-----------------------------------------------------------------------------------------------------*/
    /* unregister IRQ                                                                                      */
    /*-----------------------------------------------------------------------------------------------------*/
    if (ourport->tx_claimed || ourport->rx_claimed)
    {
        free_irq(port->irq, ourport);
    }

    /*-----------------------------------------------------------------------------------------------------*/
    /* Disable TX IRQ                                                                                      */
    /*-----------------------------------------------------------------------------------------------------*/
    if (ourport->tx_claimed)
    {
        npcmx50_uart_set_tx_irq_state((NPCMX50_UART_DEV_T)port->line, false);
        tx_enabled(port) = 0;
        ourport->tx_claimed = 0;
    }

    /*-----------------------------------------------------------------------------------------------------*/
    /* Disable RX IRQ                                                                                      */
    /*-----------------------------------------------------------------------------------------------------*/
    if (ourport->rx_claimed)
    {
        npcmx50_uart_set_rx_irq_state((NPCMX50_UART_DEV_T)port->line, false);
        ourport->rx_claimed = 0;
        rx_enabled(port) = 0;
    }
}




/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmX50_serial_startup                                                                 */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  port -                                                                                 */
/*                                                                                                         */
/* Returns:                                                                                                */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine performs port startup initializations                                     */
/*---------------------------------------------------------------------------------------------------------*/
static int npcmX50_serial_startup(struct uart_port *port)
{
    struct npcmX50_uart_port *ourport = npcmX50_convert_port_to_ourport(port);
    int ret;

    NPCMX50_SERIAL_MSG("NPCMX50 Serial: Called function %s\n", __FUNCTION__);

    /*-----------------------------------------------------------------------------------------------------*/
    /* Initializing the port's HW                                                                          */
    /*-----------------------------------------------------------------------------------------------------*/
    // skip this, already done it in console startup which runs very early
    // warning: the next function is not protected by spinlock. 
    // it is assumed to run early. 
#ifndef CONFIG_SERIAL_NPCMX50_CONSOLE    
    npcmx50_uart_init((NPCMX50_UART_DEV_T)port->line, NPCMX50_UART_MUX_MODE3_HSP1_UART1__HSP2_UART2__UART3_SI2, NPCMX50_SERIAL_BAUD);
#endif
    /*-----------------------------------------------------------------------------------------------------*/
    /* Reseting the ports fifos                                                                            */
    /*-----------------------------------------------------------------------------------------------------*/
    npcmx50_uart_reset_fifo((NPCMX50_UART_DEV_T)port->line, true, true);

    /*-----------------------------------------------------------------------------------------------------*/
    /* Configuring RX                                                                                      */
    /*-----------------------------------------------------------------------------------------------------*/
   npcmx50_uart_set_rx_config((NPCMX50_UART_DEV_T)port->line, (u8)NPCMX50_SERIAL_RX_TIMEOUT, NPCMX50_SERIAL_RX_THRES);

    /*-----------------------------------------------------------------------------------------------------*/
    /* Registering IRQ handler                                                                             */
    /*-----------------------------------------------------------------------------------------------------*/
    ret = request_irq(port->irq, npcmX50_serial_irq, 0,
                    npcmX50_convert_port_to_portname(port), (void*)ourport);

    if (ret != 0)
    {
        printk(KERN_ERR "NPCMX50 Serial: cannot get irq %d\n", port->irq);
        npcmX50_serial_shutdown(port);
        return ret;
    }

    /*-----------------------------------------------------------------------------------------------------*/
    /* Enable RX                                                                                           */
    /*-----------------------------------------------------------------------------------------------------*/
    npcmx50_uart_set_rx_irq_state((NPCMX50_UART_DEV_T)port->line, true);
    rx_enabled(port) = 1;

    ourport->rx_claimed = 1;
    ourport->tx_claimed = 1;

    NPCMX50_SERIAL_MSG("NPCMX50 Serial: Port %d started with flags 0x%X\n", port->line, port->flags);

    return 0;
}



/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmX50_serial_set_termios                                                             */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  new -                                                                                  */
/*                  old -                                                                                  */
/*                  port -                                                                                 */
/*                                                                                                         */
/* Returns:         none                                                                                   */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine handles TERMIOS                                                           */
/*---------------------------------------------------------------------------------------------------------*/
static void npcmX50_serial_set_termios(struct uart_port * port, struct ktermios *new, struct ktermios *old)
{
    unsigned int bits   = 0;
    unsigned int baud   = 0;

    port_lock(port);

    NPCMX50_SERIAL_MSG("NPCMX50 Serial: Called function %s\n", __FUNCTION__);
    NPCMX50_SERIAL_MSG("NPCMX50 Serial: Flags NEW = iflags[0x%X], oflags[0x%X], cflags[0x%X], lflags[0x%X]\n",new->c_iflag, new->c_oflag, new->c_cflag, new->c_lflag);

    /*----------------------------------------------------------------------------------------------------*/
    /* We don't support modem control lines.                                                              */
    /*----------------------------------------------------------------------------------------------------*/
    new->c_cflag &= ~(HUPCL | CMSPAR);
    new->c_cflag |= CLOCAL;

    /*-----------------------------------------------------------------------------------------------------*/
    /* Configure number of bits                                                                             */
    /*-----------------------------------------------------------------------------------------------------*/
    switch (new->c_cflag & CSIZE)
    {
        case CS5:    bits = 5;      break;
        case CS6:    bits = 6;      break;
        case CS7:    bits = 7;      break;
        case CS8:
        default:     bits = 8;      break;
    }

    npcmx50_uart_set_bits_per_char((NPCMX50_UART_DEV_T)port->line, bits);

    NPCMX50_SERIAL_MSG("NPCMX50 Serial: %dbits/char\n", bits);

    /*-----------------------------------------------------------------------------------------------------*/
    /* Configure Stop bits                                                                                 */
    /*-----------------------------------------------------------------------------------------------------*/
    if (new->c_cflag & CSTOPB)
    {
        npcmx50_uart_set_stop_bit((NPCMX50_UART_DEV_T)port->line, NPCMX50_UART_STOPBIT_DYNAMIC);
        NPCMX50_SERIAL_MSG("NPCMX50 Serial: Stop Bits dynamic\n");
    }
    else
    {
        npcmx50_uart_set_stop_bit((NPCMX50_UART_DEV_T)port->line, NPCMX50_UART_STOPBIT_1);
        NPCMX50_SERIAL_MSG("NPCMX50 Serial: 1 Stop Bit\n");
    }

    /*-----------------------------------------------------------------------------------------------------*/
    /* Configure parity                                                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
    if (new->c_cflag & PARENB)
    {
        if (new->c_cflag & PARODD)
        {
            npcmx50_uart_set_parity((NPCMX50_UART_DEV_T)port->line, NPCMX50_UART_PARITY_ODD);
            NPCMX50_SERIAL_MSG("NPCMX50 Serial: ODD  parity\n");
        }
        else
        {
            npcmx50_uart_set_parity((NPCMX50_UART_DEV_T)port->line, NPCMX50_UART_PARITY_EVEN);
            NPCMX50_SERIAL_MSG("NPCMX50 Serial: EVEN parity\n");
        }
    }
    else
    {
        npcmx50_uart_set_parity((NPCMX50_UART_DEV_T)port->line, NPCMX50_UART_PARITY_NONE);
        NPCMX50_SERIAL_MSG("NPCMX50 Serial: NO parity\n");
    }


    /*-----------------------------------------------------------------------------------------------------*/
    /* Configure baudrate                                                                                  */
    /*-----------------------------------------------------------------------------------------------------*/
    switch (new->c_cflag & CBAUD)
    {
        case  B110:     npcmx50_uart_set_baud_rate((NPCMX50_UART_DEV_T)port->line, NPCMX50_UART_BAUDRATE_110);    baud = 110;     break;
        case  B300:     npcmx50_uart_set_baud_rate((NPCMX50_UART_DEV_T)port->line, NPCMX50_UART_BAUDRATE_300);    baud = 300;     break;
        case  B600:     npcmx50_uart_set_baud_rate((NPCMX50_UART_DEV_T)port->line, NPCMX50_UART_BAUDRATE_600);    baud = 600;     break;
        case  B1200:    npcmx50_uart_set_baud_rate((NPCMX50_UART_DEV_T)port->line, NPCMX50_UART_BAUDRATE_1200);   baud = 1200;    break;
        case  B2400:    npcmx50_uart_set_baud_rate((NPCMX50_UART_DEV_T)port->line, NPCMX50_UART_BAUDRATE_2400);   baud = 2400;    break;
        case  B4800:    npcmx50_uart_set_baud_rate((NPCMX50_UART_DEV_T)port->line, NPCMX50_UART_BAUDRATE_4800);   baud = 4800;    break;
        case  B9600:    npcmx50_uart_set_baud_rate((NPCMX50_UART_DEV_T)port->line, NPCMX50_UART_BAUDRATE_9600);   baud = 9600;    break;
        case  B19200:   npcmx50_uart_set_baud_rate((NPCMX50_UART_DEV_T)port->line, NPCMX50_UART_BAUDRATE_19200);  baud = 19200;   break;
        case  B38400:   npcmx50_uart_set_baud_rate((NPCMX50_UART_DEV_T)port->line, NPCMX50_UART_BAUDRATE_38400);  baud = 38400;   break;
        case  B57600:   npcmx50_uart_set_baud_rate((NPCMX50_UART_DEV_T)port->line, NPCMX50_UART_BAUDRATE_57600);  baud = 57600;   break;
        default:
        case  B115200:  npcmx50_uart_set_baud_rate((NPCMX50_UART_DEV_T)port->line, NPCMX50_UART_BAUDRATE_115200); baud = 115200;  break;
        case  B230400:  npcmx50_uart_set_baud_rate((NPCMX50_UART_DEV_T)port->line, NPCMX50_UART_BAUDRATE_230400); baud = 230400;  break;
        case  B460800:  npcmx50_uart_set_baud_rate((NPCMX50_UART_DEV_T)port->line, NPCMX50_UART_BAUDRATE_460800); baud = 460800;  break;
    }

    NPCMX50_SERIAL_MSG("NPCMX50 Serial: Baudrate %d\n", baud);

    /*-----------------------------------------------------------------------------------------------------*/
    /* Updating timeout for the given baurdate                                                             */
    /*-----------------------------------------------------------------------------------------------------*/
    uart_update_timeout(port, new->c_cflag, baud);

    port_unlock(port);

}





/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmX50_serial_type                                                                    */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  port -                                                                                 */
/*                                                                                                         */
/* Returns:                                                                                                */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine returns serial port name                                                  */
/*---------------------------------------------------------------------------------------------------------*/
static const char *npcmX50_serial_type(struct uart_port *port)
{
    NPCMX50_SERIAL_MSG("NPCMX50 Serial: Called function %s\n", __FUNCTION__);

    if (port->type == PORT_NPCMX50)
    {
        return npcmX50_convert_port_to_portname(port);
    }
    else
    {
        return NULL;
    }
}



/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmX50_serial_config_port                                                             */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  flags -                                                                                */
/*                  port -                                                                                 */
/*                                                                                                         */
/* Returns:         none                                                                                   */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine performs port configuration                                               */
/*---------------------------------------------------------------------------------------------------------*/
static void npcmX50_serial_config_port(struct uart_port *port, int flags)
{
    NPCMX50_SERIAL_MSG("NPCMX50 Serial: Called function %s\n", __FUNCTION__);

    port_lock(port);

    if (flags & UART_CONFIG_TYPE)
    {
        port->type = PORT_NPCMX50;
    }

    port_unlock(port);
}



/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmX50_serial_verify_port                                                             */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  port -                                                                                 */
/*                  ser -                                                                                  */
/*                                                                                                         */
/* Returns:                                                                                                */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine checks if the given port is ours                                          */
/*---------------------------------------------------------------------------------------------------------*/
static int npcmX50_serial_verify_port(struct uart_port *port, struct serial_struct *ser)
{
    NPCMX50_SERIAL_MSG("NPCMX50 Serial: Called function %s\n", __FUNCTION__);

    if (ser->type != PORT_UNKNOWN && ser->type != PORT_NPCMX50)
        return -EINVAL;

    return 0;
}




/*---------------------------------------------------------------------------------------------------------*/
/* STUB functions for serial OPS                                                                           */
/*---------------------------------------------------------------------------------------------------------*/
static void npcmX50_serial_set_mctrl(struct uart_port * port, unsigned int mctrl)
{
    NPCMX50_SERIAL_MSG("NPCMX50 Serial: Called function %s\n", __FUNCTION__);
}

static unsigned int npcmX50_serial_get_mctrl(struct uart_port * port)
{
    NPCMX50_SERIAL_MSG("NPCMX50 Serial: Called function %s\n", __FUNCTION__);
    return 0;
}

static void npcmX50_serial_flush_buffer(struct uart_port * port)
{
    NPCMX50_SERIAL_MSG("NPCMX50 Serial: Called function %s\n", __FUNCTION__);
}

static void npcmX50_serial_set_ldisc(struct uart_port * port, struct ktermios * new)
{
    NPCMX50_SERIAL_MSG("NPCMX50 Serial: Called function %s\n", __FUNCTION__);
}

static void npcmX50_serial_pm(struct uart_port * port, unsigned int state, unsigned int oldstate)
{
    NPCMX50_SERIAL_MSG("NPCMX50 Serial: Called function %s\n", __FUNCTION__);
}

#if 0
static int npcmX50_serial_set_wake(struct uart_port * port, unsigned int state)
{
    NPCMX50_SERIAL_MSG("NPCMX50 Serial: Called function %s\n", __FUNCTION__);
    return 0;
}
#endif

static void npcmX50_serial_release_port(struct uart_port * port)
{
    NPCMX50_SERIAL_MSG("NPCMX50 Serial: Called function %s\n", __FUNCTION__);
}

static int npcmX50_serial_request_port(struct uart_port * port)
{
    NPCMX50_SERIAL_MSG("NPCMX50 Serial: Called function %s\n", __FUNCTION__);
    return 0;
}



/*---------------------------------------------------------------------------------------------------------*/
/* Uart port driver structure                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
static struct uart_ops npcmX50_serial_ops =
{
    .startup        = npcmX50_serial_startup,
    .shutdown       = npcmX50_serial_shutdown,
    .tx_empty       = npcmX50_serial_tx_empty,
    .stop_tx        = npcmX50_serial_stop_tx,
    .start_tx       = npcmX50_serial_start_tx,
    .stop_rx        = npcmX50_serial_stop_rx,
    .break_ctl      = npcmX50_serial_break_ctl,
    .type           = npcmX50_serial_type,
    .config_port    = npcmX50_serial_config_port,
    .verify_port    = npcmX50_serial_verify_port,
    .set_termios    = npcmX50_serial_set_termios,
    .set_mctrl      = npcmX50_serial_set_mctrl,
    .get_mctrl      = npcmX50_serial_get_mctrl,
    .flush_buffer   = npcmX50_serial_flush_buffer,
    .set_ldisc      = npcmX50_serial_set_ldisc,
    .pm             = npcmX50_serial_pm,
   // .set_wake       = npcmX50_serial_set_wake,
    .release_port   = npcmX50_serial_release_port,
    .request_port   = npcmX50_serial_request_port,

};




/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                       Serial platform driver Code                                       */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/

#ifdef CONFIG_SERIAL_NPCMX50_CONSOLE
#define NPCMX50_SERIAL_CONSOLE &npcmX50_serial_console
#else
#define NPCMX50_SERIAL_CONSOLE NULL
#endif

static struct uart_driver npcmX50_uart_drv =
{
    .owner          = THIS_MODULE,
    .dev_name       = NPCMX50_SERIAL_NAME,
    .nr             = NPCMX50_UART_NUM_OF_MODULES,
    .cons           = NPCMX50_SERIAL_CONSOLE,
    .driver_name    = NPCMX50_SERIAL_NAME,
    .major          = NPCMX50_SERIAL_MAJOR,
    .minor          = NPCMX50_SERIAL_MINOR,
};



/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmX50_serial_init_port                                                               */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  ourport -                                                                              */
/*                  platdev -                                                                              */
/*                                                                                                         */
/* Returns:                                                                                                */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine performs single port                                                      */
/*---------------------------------------------------------------------------------------------------------*/
static int npcmX50_serial_init_port(struct npcmX50_uart_port *ourport,
                                    struct platform_device *platdev)
{
    struct uart_port *port      = &ourport->port;
    int ret = 0;
	
#ifdef CONFIG_OF
	struct resource *res;
	struct clk* ser_clk = NULL;
#endif

    NPCMX50_SERIAL_MSG("NPCMX50 Serial platform: Called function %s\n", __FUNCTION__);

    /*-----------------------------------------------------------------------------------------------------*/
    /* Parameters check                                                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
    if (platdev == NULL)
        return -ENODEV;

    /*-----------------------------------------------------------------------------------------------------*/
    /* Cross linkage between port and dev                                                                  */
    /*-----------------------------------------------------------------------------------------------------*/
    port->dev   = &platdev->dev;

    /*-----------------------------------------------------------------------------------------------------*/
    /* Configure UART clocks and return CLK value                                                          */
    /*-----------------------------------------------------------------------------------------------------*/
    #ifdef CONFIG_OF	
    ser_clk = devm_clk_get(&platdev->dev, NULL);
    
    if (IS_ERR(ser_clk))		
        return PTR_ERR(ser_clk);

    NPCMX50_SERIAL_MSG("\tserial clock is %ld\n" , clk_get_rate(ser_clk)); 
    
    port->uartclk   = clk_get_rate(ser_clk);
    #else
    port->uartclk   = 24*_1MHz_;
    #endif // CONFIG_OF
    

    /*-----------------------------------------------------------------------------------------------------*/
    /* Setting MEMBASE address                                                                             */
    /*-----------------------------------------------------------------------------------------------------*/
#ifdef CONFIG_OF
    res = platform_get_resource(platdev, IORESOURCE_MEM, 0);
	NPCMX50_SERIAL_MSG("\tmemory resource is 0x%lx, statically it was 0x%lx\n" , (long unsigned int)res, (long unsigned int)port->membase); 

	
	if (!request_mem_region(res->start, resource_size(res), platdev->name)) {
		ret = -EBUSY;
	}
    else
    {
    	dev_set_drvdata(&platdev->dev, res);
    	port->membase = ioremap(res->start, resource_size(res));
    }
#else	
    port->membase = (void*)UART_VIRT_BASE_ADDR(port->line);
#endif


	
	NPCMX50_SERIAL_MSG("\tmemory resource is 0x%lx\n" ,(long unsigned int)port->membase); 
	
   /*-----------------------------------------------------------------------------------------------------*/
    /* Get irq from the device                                                                             */
    /*-----------------------------------------------------------------------------------------------------*/
    port->irq       = platform_get_irq(platdev, 0);

    /*-----------------------------------------------------------------------------------------------------*/
    /* Init Fifosize                                                                                       */
    /*-----------------------------------------------------------------------------------------------------*/
    port->fifosize  = NPCMX50_SERIAL_TX_FIFO_SIZE;
	
    NPCMX50_SERIAL_MSG("NPCMX50 Serial platform: Port %2d initialized mem=%08x, irq=%d, clock=%d\n", port->line, (int)npcmX50_serial_ports[port->line].port.membase, port->irq, port->uartclk);

    return ret;
}



/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmX50_serial_probe                                                                   */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  dev -                                                                                  */
/*                                                                                                         */
/* Returns:                                                                                                */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine adds new Serial driver device                                             */
/*---------------------------------------------------------------------------------------------------------*/
static int npcmX50_serial_probe(struct platform_device *dev)
{
    struct npcmX50_uart_port *ourport;
    int ret;	
		
	#ifdef CONFIG_OF
	struct device_node *np;
	#endif

    /*-----------------------------------------------------------------------------------------------------*/
    /* Parameters check                                                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
    if (dev == NULL)
        return -ENODEV;


    NPCMX50_SERIAL_MSG("NPCMX50 Serial platform: Called function %s with ID=%d\n", __FUNCTION__, dev->id);
	
	#ifdef CONFIG_OF
	np = dev->dev.of_node;
	dev->id = of_alias_get_id(np, "serial");
	if (dev->id < 0)
		dev->id = 0;
	#endif

    /*-----------------------------------------------------------------------------------------------------*/
    /* Retriving the current free port                                                                     */
    /*-----------------------------------------------------------------------------------------------------*/
    ourport = &npcmX50_serial_ports[dev->id];

    /*-----------------------------------------------------------------------------------------------------*/
    /* Initializing the port                                                                               */
    /*-----------------------------------------------------------------------------------------------------*/
    ret = npcmX50_serial_init_port(ourport, dev);
    if (ret < 0)
       return ret;

    /*-----------------------------------------------------------------------------------------------------*/
    /* Adding the serial port to the "system"                                                              */
    /*-----------------------------------------------------------------------------------------------------*/
    NPCMX50_SERIAL_MSG("NPCMX50 Serial platform: adding port number %d\n", ourport->port.line);
    npcmX50_uart_drv.minor = NPCMX50_SERIAL_MINOR +  dev->id;
    uart_add_one_port(&npcmX50_uart_drv, &ourport->port);
    platform_set_drvdata(dev, &ourport->port);

    gcr_regmap = syscon_regmap_lookup_by_compatible("nuvoton,npcm750-gcr");
    if (IS_ERR(gcr_regmap)) {
        pr_err("%s: failed to find nuvoton,npcm750-gcr\n", __func__);
        return IS_ERR(gcr_regmap);
    }

    return 0;
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmX50_serial_remove                                                                  */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  dev -                                                                                  */
/*                                                                                                         */
/* Returns:                                                                                                */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine removes Serial driver device                                              */
/*---------------------------------------------------------------------------------------------------------*/
static int npcmX50_serial_remove(struct platform_device *dev)
{
    struct uart_port *port = npcmX50_convert_dev_to_port(&dev->dev);

    NPCMX50_SERIAL_MSG("NPCMX50 Serial platform: Called function %s\n", __FUNCTION__);

    if (port)
    {
        uart_remove_one_port(&npcmX50_uart_drv, port);
    }

    return 0;
}

static const struct of_device_id uart_dt_id[] = {
	{ .compatible = "nuvoton,npcm750-uart",  },
	{},
};
MODULE_DEVICE_TABLE(of, uart_dt_id);


/*---------------------------------------------------------------------------------------------------------*/
/* NPCMX50 Serial platform driver structure                                                                */
/*---------------------------------------------------------------------------------------------------------*/
static struct platform_driver npcmX50_serial_drv =
{
    .probe      = npcmX50_serial_probe,
    .remove     = npcmX50_serial_remove,
    .suspend    = NULL,
    .resume     = NULL,
    .driver     =
    {
        .name   = "npcmX50-uart",
        .owner  = THIS_MODULE,
		.of_match_table = of_match_ptr(uart_dt_id),
    },
};




/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmX50_serial_modinit                                                                 */
/*                                                                                                         */
/* Parameters:      none                                                                                   */
/* Returns:                                                                                                */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine performs serial module registration                                       */
/*---------------------------------------------------------------------------------------------------------*/
static int __init npcmX50_serial_modinit(void)
{
    NPCMX50_SERIAL_MSG("NPCMX50 Serial platform: Called function %s\n", __FUNCTION__);

    /*-----------------------------------------------------------------------------------------------------*/
    /* Registering our ports                                                                               */
    /*-----------------------------------------------------------------------------------------------------*/
	#ifndef CONFIG_OF
    platform_add_devices(npcmX50_uart_devs, ARRAY_SIZE(npcmX50_uart_devs));
	#endif

    /*-----------------------------------------------------------------------------------------------------*/
    /* registering our UART driver description to UART core logic                                          */
    /*-----------------------------------------------------------------------------------------------------*/
    if (uart_register_driver(&npcmX50_uart_drv) < 0)
    {
        printk(KERN_ERR "NPCMX50 Serial platform: failed to register UART driver\n");
        return -1;
    }

    /*-----------------------------------------------------------------------------------------------------*/
    /* Register out SERIAL driver to the system                                                            */
    /*-----------------------------------------------------------------------------------------------------*/
    platform_driver_register(&npcmX50_serial_drv);

    return 0;
}



/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmX50_serial_modexit                                                                 */
/*                                                                                                         */
/* Parameters:      none                                                                                   */
/* Returns:                                                                                                */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine performs serial module unregistration                                     */
/*---------------------------------------------------------------------------------------------------------*/
static void __exit npcmX50_serial_modexit(void)
{
    NPCMX50_SERIAL_MSG("NPCMX50 Serial platform: Called function %s\n", __FUNCTION__);

    /*-----------------------------------------------------------------------------------------------------*/
    /* Unregister our drivers from system and uart core                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
    platform_driver_unregister(&npcmX50_serial_drv);
    uart_unregister_driver(&npcmX50_uart_drv);
}


module_init(npcmX50_serial_modinit);
module_exit(npcmX50_serial_modexit);



/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*                                              Console code                                               */
/*---------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
#ifdef CONFIG_SERIAL_NPCMX50_CONSOLE


/*---------------------------------------------------------------------------------------------------------*/
/* Console's uart port                                                                                     */
/*---------------------------------------------------------------------------------------------------------*/
static struct uart_port* npcmX50_console_port;


/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmX50_console_putchar                                                                */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  ch -                                                                                   */
/*                  port -                                                                                 */
/*                                                                                                         */
/* Returns:         none                                                                                   */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine writes single char for the console                                        */
/*---------------------------------------------------------------------------------------------------------*/
static void npcmX50_console_putchar(struct uart_port *port, int ch)
{
    npcmx50_uart_putc((NPCMX50_UART_DEV_T)port->line, ch);
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmX50_console_setup                                                                  */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  co -                                                                                   */
/*                  options -                                                                              */
/*                                                                                                         */
/* Returns:                                                                                                */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine performs console setup                                                    */
/*---------------------------------------------------------------------------------------------------------*/
static int __init npcmX50_console_setup(struct console *co, char *options)
{
    struct uart_port *port;

     //   int n = 0;
    
    /*-----------------------------------------------------------------------------------------------------*/
    /* Setting default values                                                                              */
    /*-----------------------------------------------------------------------------------------------------*/
    int baud    = NPCMX50_SERIAL_BAUD;
    int bits    = NPCMX50_SERIAL_BITS;
    int parity  = NPCMX50_SERIAL_PARITY;
    int flow    = 0;

    NPCMX50_SERIAL_MSG("NPCMX50 Serial Console: Called function %s\n", __FUNCTION__);

    /*-----------------------------------------------------------------------------------------------------*/
    /* if port is illigal we use default console port                                                      */
    /*-----------------------------------------------------------------------------------------------------*/
    if (co->index == -1 || co->index >= NPCMX50_UART_NUM_OF_MODULES)
        co->index = NPCMX50_SERIAL_CONSOLE_PORT;

    /*-----------------------------------------------------------------------------------------------------*/
    /* Getting port structure                                                                              */
    /*-----------------------------------------------------------------------------------------------------*/
    port = &npcmX50_serial_ports[co->index].port;

    /*-----------------------------------------------------------------------------------------------------*/
    /* Is the port configured already?                                                                     */
    /*-----------------------------------------------------------------------------------------------------*/
    if (port->membase != 0x0)
    {
        //return -1;
    }

    
    // warning: this function is invoked during init (console_initcall)
        // it is called before the driver is probed, so we have no choise but to select the HW resources statically.
     //   for (n = 0; n < NPCMX50_UART_NUM_OF_MODULES; n++)
      //      npcmX50_serial_ports[n].port.membase = (void*)UART_VIRT_BASE_ADDR(n);


    /*-----------------------------------------------------------------------------------------------------*/
    /* Initializing the port's HW                                                                          */
    /*-----------------------------------------------------------------------------------------------------*/
    npcmx50_uart_init((NPCMX50_UART_DEV_T)co->index, NPCMX50_UART_MUX_MODE3_HSP1_UART1__HSP2_UART2__UART3_SI2, NPCMX50_SERIAL_BAUD);

    /*-----------------------------------------------------------------------------------------------------*/
    /* Configuring the selected port                                                                       */
    /*-----------------------------------------------------------------------------------------------------*/
    //npcmX50_serial_init_port(&npcmX50_serial_ports[co->index], npcmX50_uart_devs[co->index]);


    /*-----------------------------------------------------------------------------------------------------*/
    /* Preserving the port of our console                                                                  */
    /*-----------------------------------------------------------------------------------------------------*/
    npcmX50_console_port = port;


    NPCMX50_SERIAL_MSG("NPCMX50 Serial Console: Using port %d\n", port->line);

    /*-----------------------------------------------------------------------------------------------------*/
    /* Parsing command line options, if there are none, staying with defaults                              */
    /*-----------------------------------------------------------------------------------------------------*/
    if (options)
    {
        uart_parse_options(options, &baud, &parity, &bits, &flow);
    }

    return uart_set_options(port, co, baud, parity, bits, flow);
}



/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmX50_console_write                                                                  */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                  co -                                                                                   */
/*                  count -                                                                                */
/*                  s -                                                                                    */
/*                                                                                                         */
/* Returns:         none                                                                                   */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine is console interface function that writes a given string to the console   */
/*---------------------------------------------------------------------------------------------------------*/
static void npcmX50_console_write(struct console *co, const char *s,
                 unsigned int count)
{
    uart_console_write(npcmX50_console_port, s, count, npcmX50_console_putchar);
}


/*---------------------------------------------------------------------------------------------------------*/
/* NPCMX50 Console Driver structure                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
static struct console npcmX50_serial_console =
{
    .name       = NPCMX50_SERIAL_NAME,
    .device     = uart_console_device,
    .flags      = CON_PRINTBUFFER,
    .index      = -1,
    .write      = npcmX50_console_write,
    .setup      = npcmX50_console_setup,
    .data       = &npcmX50_uart_drv,
};



//static struct early_platform_driver early_npcmx50_uart_driver __initdata = {
//	.class_str    = NPCMX50_SERIAL_NAME,
//	.pdrv         = &npcmX50_serial_drv,
//	.requested_id = EARLY_PLATFORM_ID_UNSET,
//};


/*---------------------------------------------------------------------------------------------------------*/
/* Function:        npcmX50_serial_initconsole                                                             */
/*                                                                                                         */
/* Parameters:      none                                                                                   */
/* Returns:                                                                                                */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                  This routine performs initialized the serial console                                   */
/*---------------------------------------------------------------------------------------------------------*/
static int npcmX50_console_init(void)
{
    int    rc = 0; 

#ifdef CONFIG_OF
	struct resource res;	
	struct device_node *np = NULL;

#ifdef CLK_TREE_SUPPORT_IN_EARLY_INIT    
	struct platform_device *pdev = NULL; 
	struct clk* ser_clk = NULL;
#else
	npcmX50_serial_ports[NPCMX50_SERIAL_CONSOLE_PORT].port.uartclk = 24*_1MHz_;
#endif

    np = of_find_node_by_name(NULL, "serial3");
    if (np == NULL){
        printk(KERN_ERR "%s- can't get device tree node\n", __FUNCTION__);
    }
    
    rc = of_address_to_resource(np, 0, &res);
    if (rc) {
        pr_info("%s: of_address_to_resource fail ret %d \n", __FUNCTION__, rc);
        return -EINVAL;
    }
    
    npcmX50_serial_ports[NPCMX50_SERIAL_CONSOLE_PORT].port.membase = ioremap(res.start, resource_size(&res));
    
    if (!npcmX50_serial_ports[NPCMX50_SERIAL_CONSOLE_PORT].port.membase) {
                    pr_info("%s:serial_virt_addr fail \n", __FUNCTION__);
                    return -ENOMEM;
    }
    NPCMX50_SERIAL_MSG("%s: console UART base is 0x%08X\n", __FUNCTION__, (u32)npcmX50_serial_ports[NPCMX50_SERIAL_CONSOLE_PORT].port.membase);

            // alternative solution:
        	//early_platform_driver_register(&early_npcmx50_uart_driver, "npcmX50-uart");
        	//early_platform_driver_probe(NPCMX50_SERIAL_NAME, NPCMX50_UART_NUM_OF_MODULES, 0);

#ifdef CLK_TREE_SUPPORT_IN_EARLY_INIT  	
    pdev = of_find_device_by_node(np);
    ser_clk = devm_clk_get(&pdev->dev, NULL);
    
    if (IS_ERR(ser_clk))		
        return PTR_ERR(ser_clk);

    NPCMX50_SERIAL_MSG("%s: serial clock is %ld\n" , __FUNCTION__, clk_get_rate(ser_clk)); 
    
    npcmX50_serial_ports[NPCMX50_SERIAL_CONSOLE_PORT].port.uartclk   = clk_get_rate(ser_clk);
#endif //  CLK_TREE_SUPPORT_IN_EARLY_INIT    

#else
    npcmX50_serial_ports[NPCMX50_SERIAL_CONSOLE_PORT].port.uartclk = 24*_1MHz_;
    npcmX50_serial_ports[0].port.membase = (unsigned char*)UART_VIRT_BASE_ADDR(0);
    npcmX50_serial_ports[1].port.membase = (unsigned char*)UART_VIRT_BASE_ADDR(1);
    npcmX50_serial_ports[2].port.membase = (unsigned char*)UART_VIRT_BASE_ADDR(2);
    npcmX50_serial_ports[3].port.membase = (unsigned char*)UART_VIRT_BASE_ADDR(3);
#endif // CONFIG_OF

    /*-----------------------------------------------------------------------------------------------------*/
    /* Initializing our ports                                                                              */
    /*-----------------------------------------------------------------------------------------------------*/
    //npcmX50_console_init_ports();

    /*-----------------------------------------------------------------------------------------------------*/
    /* Register our console for early messages printing                                                    */
    /*-----------------------------------------------------------------------------------------------------*/
    register_console(&npcmX50_serial_console);

    printk(KERN_NOTICE "NPCMX50 Console Initialized\n");

    return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
/* Registering our console                                                                                 */
/*---------------------------------------------------------------------------------------------------------*/
console_initcall(npcmX50_console_init);


#endif /* CONFIG_SERIAL_NPCMX50_CONSOLE */


/*---------------------------------------------------------------------------------------------------------*/
/* Module licensing information                                                                            */
/*---------------------------------------------------------------------------------------------------------*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mark Lukoyanichev <Mark.Lukoyanichev@nuvoton.com");
MODULE_DESCRIPTION("NPCMX50 Serial port driver");

