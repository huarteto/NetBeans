/*----------------------------------------------------------------------------
 *      RL-ARM - RTX
 *----------------------------------------------------------------------------
 *      Name:    BLINKY.C
 *      Purpose: RTX example program
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2010 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/
                  
#include <RTL.h>
#include <stdio.h>
#include "BU1511KV2.h"                    /* BU1511KV2 settings              */

OS_TID t_uart;                            /* assigned task id of task: uart  */
OS_TID t_blink;                           /* assigned task id of task: blink */


/*--------------------------- ser_init --------------------------------------*/

static void ser_init (void) {
  /* Initialize UART2 interface. */
  ROHM_UART2->FCR = 0;                   /* disable FIFO                     */ 
  ROHM_UART2->LCR = 0x83;                /* 8 bits, no Parity, 1 Stop bit    */
  ROHM_UART2->DLL = 22;                  /* 115200 Baud Rate @ 40.5 MHZ PCLK */
  ROHM_UART2->DLH = 0;                   /* High divisor latch = 0           */
  ROHM_UART2->LCR = 0x03;                /* DLAB = 0                         */
}

/*--------------------------- fputc -----------------------------------------*/

int fputc(int ch, FILE *f)  {
  /* Print to to serial port. */

  if (ch == '\n')  {
    while (!(ROHM_UART2->LSR & 0x20));
    ROHM_UART2->THR = '\r';
  }
  while (!(ROHM_UART2->LSR & 0x20));
  ROHM_UART2->THR = ch;
  return (ch);
}

/*----------------------------------------------------------------------------
 *        Task 1 'uart': Uart Output
 *---------------------------------------------------------------------------*/
__task void uart (void) {
  U32 count;

  count = 0;
  printf ("RTX_Blinky started ...\n");
  for (;;) {
    os_evt_wait_or (0x0001,0xFFFF);
    printf ("LED blink %d\n",count++);
  }
}

/*----------------------------------------------------------------------------
 *        Task 2 'blink': Toggle GPIO
 *---------------------------------------------------------------------------*/
__task void blink (void) {
  for (;;) {
    ROHM_GPIO->swporta_dr |= (1<<0);    /* set GPIO port 0 high              */
    os_evt_set (0x0001, t_uart);
    os_dly_wait (10);                   /* wait 10 clock ticks               */
    ROHM_GPIO->swporta_dr &= ~(1<<0);   /* set GPIO port 0 low               */
    os_dly_wait (90);                   /* wait 90 clock ticks               */
  }
}

/*----------------------------------------------------------------------------
 *        Task 3 'init': Initialize
 *---------------------------------------------------------------------------*/
__task void init (void) {

  ROHM_GPIO->swporta_ddr |=  (1 << 0);   /* GPIO0 is output                  */
  ser_init ();                           /* UART#2 Initialization            */

  t_uart  = os_tsk_create (uart, 0);     /* start task 'uart'                */
  t_blink = os_tsk_create (blink, 0);    /* start task 'blink'               */
  os_tsk_delete_self ();
}

/*----------------------------------------------------------------------------
 *        Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/
int main (void) {
  os_sys_init (init);                   /* Initialize RTX and start init     */
}

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/
