/******************************************************************************/
/* HELLO.C: Hello World Example                                               */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <stdio.h>                /* prototype declarations for I/O functions */
#include <LPC21xx.H>              /* LPC21xx definitions                      */


/****************/
/* main program */
/****************/
int main (void)  {                /* execution starts here                    */

  /* initialize the serial interface   */
  PINSEL0 = 0x00050000;           /* Enable RxD1 and TxD1                     */
  U1LCR = 0x83;                   /* 8 bits, no Parity, 1 Stop bit            */
  U1DLL = 97;                     /* 9600 Baud Rate @ 15MHz VPB Clock         */
  U1LCR = 0x03;                   /* DLAB = 0                                 */

  printf ("Hello World\n");       /* the 'printf' function call               */

  while (1) {                          /* An embedded program does not stop and       */
    ;  /* ... */                       /* never returns. We use an endless loop.      */
  }                                    /* Replace the dots (...) with your own code.  */

}
