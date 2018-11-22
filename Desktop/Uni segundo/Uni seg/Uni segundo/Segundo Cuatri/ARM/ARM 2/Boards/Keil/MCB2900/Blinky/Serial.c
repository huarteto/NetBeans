/******************************************************************************/
/* SERIAL.C: Low Level Serial Routines                                        */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <LPC29xx.H>                     /* LPC29xx definitions               */

#define UART0                            /* Use UART 0 for printf             */

/* If UART 0 is used for printf                                               */
#ifdef UART0
  #define UxFCR  U0FCR
  #define UxLCR  U0LCR
  #define UxDLL  U0DLL
  #define UxDLM  U0DLM
  #define UxLSR  U0LSR
  #define UxTHR  U0THR
  #define UxRBR  U0RBR
/* If UART 1 is used for printf                                               */
#elif defined(UART1)
  #define UxFCR  U1FCR
  #define UxLCR  U1LCR
  #define UxDLL  U1DLL
  #define UxDLM  U1DLM
  #define UxLSR  U1LSR
  #define UxTHR  U1THR
  #define UxRBR  U1RBR
#endif


void init_serial (void)  {               /* Initialize Serial Interface       */

  /* Configure timer clock = PLL clock / 4 = 40 MHz                           */
  UART_CLK_CONF = (2UL << 24) | (1 << 11) | (3 << 2);

  #ifdef UART0
    SFSP1_22 = 1;                        /* Enable TxD0                       */
    SFSP1_23 = 1 | (1 << 2);             /* Enable RxD0                       */
  #elif defined (UART1)
    SFSP0_24 = 1;                        /* Enable TxD1                       */
    SFSP0_25 = 1 | (1 << 2);             /* Enable RxD1                       */
  #endif
  UxFCR    = 0;                          /* FIFO not used                     */
  UxLCR    = 0x83;                       /* 8 bits, no Parity, 1 Stop bit     */
  UxDLL    = 4;                          /* 9600 Baud Rate @ 40.0 MHZ UART clk*/
  UxDLM    = 1;                          /* High divisor latch = 1            */
  UxLCR    = 0x03;                       /* DLAB = 0                          */
}


/* Implementation of putchar (also used by printf function to output data)    */
int sendchar (int ch)  {                 /* Write character to Serial Port    */

  while (!(UxLSR & 0x20));

  return (UxTHR = ch);
}


int getkey (void)  {                     /* Read character from Serial Port   */

  while (!(UxLSR & 0x01));

  return (UxRBR);
}
