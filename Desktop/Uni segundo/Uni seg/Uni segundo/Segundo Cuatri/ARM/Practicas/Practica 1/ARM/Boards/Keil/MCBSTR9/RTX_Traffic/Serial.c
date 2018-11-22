/*----------------------------------------------------------------------------
 *      RL-ARM - RTX
 *----------------------------------------------------------------------------
 *      Name:    SERIAL.C
 *      Purpose: Interrupt controlled and event driven serial IO
 *      Rev.:    V4.05
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2009 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include <91x_lib.h>                 /* STR91x definitions                   */
#include <RTL.h>                     /* RTX kernel functions & defines       */
#include <stdio.h>

#define  CTRL_Q  0x11                /* Control+Q character code             */
#define  CTRL_S  0x13                /* Control+S character code             */

static BIT sendstop;                 /* flag: marks XOFF character           */
static char recbuf[256];
static U8 ridx;
static U8 widx;

OS_TID wr_task;
OS_TID rd_task;

/*----------------------------------------------------------------------------
 *       fputc:  event driven putchar function
 *---------------------------------------------------------------------------*/
int fputc (int c, FILE *f) {
  wr_task = os_tsk_self ();          /* identify task for serial interrupt   */
  os_evt_clr (0x0100, wr_task); 
  if (c == '\n') {                   /* expand new line character:           */
    UART0->DR = (char)'\r';
    os_evt_wait_or (0x0100, 0xffff); /* wait till character transmited       */
  }
  UART0->DR = (char)c;               /* transmit a character                 */
  os_evt_wait_or (0x0100, 0xffff);   /* wait till character transmited       */
  return (c);                        /* return character: ANSI requirement   */
}

/*----------------------------------------------------------------------------
 *       getkey:  event driven getkey function
 *---------------------------------------------------------------------------*/
int getkey (void) {
  if (ridx == widx) {
    rd_task = os_tsk_self ();
    os_evt_clr (0x0100, rd_task);
    os_evt_wait_or (0x0100, 0xffff); /* wait till character received         */
  }
  return (recbuf[ridx++]);
}


/*----------------------------------------------------------------------------
 *       serial:  serial receive and transmit interrupt
 *---------------------------------------------------------------------------*/
void int_serial (void) __irq {
  volatile U32 IIR;
           U8  c;

  IIR = UART0->MIS & (UART_IT_Transmit | UART_IT_Receive);
  if (IIR & UART_IT_Receive) {
    UART0->ICR = UART_IT_Receive;	    /* clear the interrupt               */

    c = (char) UART0->DR;               /* read character                    */
    switch (c)  {                       /* process character                 */
      case CTRL_S:
        sendstop = __TRUE;              /* if Control+S stop transmission    */
        goto ack;
 
      case CTRL_Q:
        sendstop = __FALSE;             /* if Control+Q start transmission   */
        break;

      default:                          /* send character to a mailbox       */
        recbuf[widx++] = c;
        isr_evt_set (0x0100, rd_task);  /* set event character received      */
        goto ack;
    } // end switch
  }

  if (IIR & UART_IT_Transmit) {
    UART0->ICR = UART_IT_Transmit;	    /* clear the interrupt               */

    if (!sendstop)  {                   /* if not Control+S received         */
      isr_evt_set (0x0100, wr_task);    /* Go on, set event transmit ready   */
    }
  }

ack:
  VIC1->VAR = 0;                        /* Acknowledge Interrupt             */
}

/*----------------------------------------------------------------------------
 *       com_baudrate: initialize serial interface
 *---------------------------------------------------------------------------*/
void com_baudrate (unsigned int baudrate) {
  unsigned long      BRDi = 0, BRDf = 0;
  unsigned long long UARTClk = 48000000;

  if ((SCU->CLKCNTR & SCU_BRCLK_Div2) != SCU_BRCLK_Div2) { 
    UARTClk /= 2;							      /* set UART Clk acc. BRSEL */
  }

  /* baudrate calculation */
  BRDi = ((100 * UARTClk) / (16 * baudrate));     /* calculate integer part  */
  BRDf = BRDi - (100 * (BRDi / 100));             /* calculate fractal part  */

  UART0->CR  &= 0xFFFE;                           /* disable UART            */

  UART0->IBRD = BRDi / 100;				          /* set the integer part    */
  UART0->FBRD = ((((BRDf * 64) + 50) / 100));     /* set the fractal part    */

  UART0->LCR &= 0xFFFF;                           /* to accept baudrate      */
  UART0->CR  |= 0x0001;  			              /* enable UART             */	 
}

/*----------------------------------------------------------------------------
 *       serial_init: initialize serial interface
 *---------------------------------------------------------------------------*/
void serial_init (void) {

 /* Setup UART0 TX/RX pins */	
  SCU->GPIOOUT[3] = 0x0008;                       /* P3.1 output UART0 TX    */
  SCU->GPIOIN[3]  = 0x01;                         /* P3.0 input  UART0 RX    */

  UART0->LCR  =   UART_WordLength_8D              /* 8 bits                  */
                | UART_Parity_No                  /* no Parity               */
				| UART_HardwareFlowControl_None   /* no flow control         */
				& UART_StopBits_1                 /* 1 Stop bit              */
				& UART_FIFO_Disable;              /* FIFO disabled           */

  UART0->CR   =   UART_Mode_Rx 					  /* Receive Enable          */
                | UART_Mode_Tx					  /* Transmit Enable         */
                | 0x0001;                         /* UART Enable             */

  UART0->IMSC =  UART_IT_Transmit                 /* Enable Tx Interrupt     */
                |UART_IT_Receive ;                /* Enable Rx Interrupt     */

  com_baudrate (115200);                          /* 115200 at 48MHz         */

  /* Setup UART0 Interrupt, UART0_ITLine = 16 */
  VIC1->VAiR[3] = (U32)int_serial;                /* set IRQ handler         */
  VIC1->INTSR  &= ~(0x01 << (UART0_ITLine - 16)); /* generate an IRQ int.    */
  VIC1->VCiR[3] =    0x20 | (UART0_ITLine - 16);  /* enable vetored int.No 0 */
  VIC1->INTER  |=  (0x01 << (UART0_ITLine - 16)); /* enable the int.         */

  sendstop = __FALSE;                             /* CtrlQ not received      */
  ridx = widx = 0;                                /* clear buffer indexes    */
}

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/
