/******************************************************************************/
/* HELLO.C: Hello World Example                                               */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <stdio.h>                          /* I/O Functions */
#include <LPC22XX.H>                        /* LPC22XX Peripheral Registers */

extern void init_serial (void);

void delay (void)  {                        /* Delay Function */
  unsigned long x;

  for (x = 0; x < 400000; x++);
}

int main (void) {

  init_serial();                            /* Initialize Serial Interface */

  IODIR0 = 0x00000100;                      /* P0.8 defined as Output */

  while (1) {                               /* Loop forever */
    IOCLR0 = 0x00000100;                    /* Turn LED On  (P0.8 = 0) */
    printf ("Hello World\n");               /* Print "Hello World" */
    IOSET0 = 0x00000100;                    /* Turn LED Off (P0.8 = 1) */
    delay();                                /* Delay */
  }
}
