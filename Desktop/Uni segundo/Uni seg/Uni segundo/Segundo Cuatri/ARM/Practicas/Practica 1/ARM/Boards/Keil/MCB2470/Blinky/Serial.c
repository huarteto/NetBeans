/******************************************************************************/
/* SERIAL.C: Low Level Serial Routines                                        */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <LPC23xx.H>                    /* LPC23xx/LPC24xx definitions        */

#define UART2                           /* Use UART 2 for printf              */

/* If UART 0 is used for printf                                               */
#ifdef UART0
  #define UxFDR  U0FDR
  #define UxLCR  U0LCR
  #define UxDLL  U0DLL
  #define UxDLM  U0DLM
  #define UxLSR  U0LSR
  #define UxTHR  U0THR
  #define UxRBR  U0RBR
/* If UART 1 is used for printf                                               */
#elif defined(UART1)
  #define UxFDR  U1FDR
  #define UxLCR  U1LCR
  #define UxDLL  U1DLL
  #define UxDLM  U1DLM
  #define UxLSR  U1LSR
  #define UxTHR  U1THR
  #define UxRBR  U1RBR
/* If UART 2 is used for printf                                               */
#elif defined (UART2)
  #define UxFDR  U2FDR
  #define UxLCR  U2LCR
  #define UxDLL  U2DLL
  #define UxDLM  U2DLM
  #define UxLSR  U2LSR
  #define UxTHR  U2THR
  #define UxRBR  U2RBR
#endif


void init_serial (void)  {               /* Initialize Serial Interface       */
  #ifdef UART0
    PCONP   |= (1 << 3);                 /* Enable UART0 power                */
    PINSEL0 |= 0x00000050;               /* Enable TxD0 and RxD0              */
  #elif defined (UART1)
    PCONP   |= (1 << 4);                 /* Enable UART1 power                */
    PINSEL7 |= 0x0000000F;               /* Enable TxD1 and RxD1              */
  #elif	defined (UART2)
    PCONP   |= (1 <<24);                 /* Enable UART2 power                */
    PINSEL0 &= ~0x00F00000;
    PINSEL0 |=  0x00500000;              /* Enable TxD2 and RxD2              */
  #endif
  UxFDR    = 0;                          /* Fractional divider not used       */
  UxLCR    = 0x83;                       /* 8 bits, no Parity, 1 Stop bit     */
  UxDLL    = 117;                        /* 9600 Baud Rate @ 18 MHZ PCLK      */
  UxDLM    = 0;                          /* High divisor latch = 0            */
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
