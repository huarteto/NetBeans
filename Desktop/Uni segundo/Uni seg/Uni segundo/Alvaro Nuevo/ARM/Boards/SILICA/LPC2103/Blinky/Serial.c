/******************************************************************************/
/* Serial.C: Low Level Serial Routines                                        */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <LPC21xx.H>                    /* LPC21xx definitions                */

#define CR     0x0D


void SerialInit (void)  {               /* Initialize Serial Interface        */
  PINSEL0 = 0x00050000;                 /* Enable RxD1 and TxD1               */
  U1LCR = 0x83;                         /* 8 bits, no Parity, 1 Stop bit      */
  U1DLL = 48;                           /* 19200 Baud Rate @ 14.7MHz VPB Clock*/
  U1LCR = 0x03;                         /* DLAB = 0                           */
}


/* Implementation of putchar (also used by printf function to output data)    */
int sendchar (int ch)  {                /* Write character to Serial Port     */

  if (ch == '\n')  {
    while (!(U1LSR & 0x20));
    U1THR = CR;                         /* output CR                          */
  }
  while (!(U1LSR & 0x20));
  return (U1THR = ch);
}


int getkey (void)  {                    /* Read character from Serial Port    */

  while (!(U1LSR & 0x01));

  return (U1RBR);
}

/* Output string to serial UART                                               */
void SendStr (char *p) {                /* Write string                       */
  while (*p) {
    sendchar (*p++);
  }
}
