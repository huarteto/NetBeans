/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <LPC22XX.H>                        /* LPC22XX Peripheral Registers */

extern long volatile timeval;
extern void init_timer(void);

void wait (void)  {                         /* wait function */
  unsigned long i;

  i = timeval;
  while ((timeval - i) != 10);              /* wait 100ms */
}

int main (void)  {

  init_timer();                             /* Initialize Timer */

  IODIR0 = 0x00000100;                      /* P0.8 defined as Output */

  while (1) {                               /* Loop forever */
    IOCLR0 = 0x00000100;                    /* Turn LED On  (P0.8 = 0) */
    wait();                                 /* Wait */
    IOSET0 = 0x00000100;                    /* Turn LED Off (P0.8 = 1) */
    wait();                                 /* Wait */
  }
}
