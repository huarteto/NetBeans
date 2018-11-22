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
#include <S3C44B0X.H>                       /* S3C44B0X definitions */


extern volatile unsigned long timeval;      /* Current Time Tick */

void wait (unsigned long time) {            /* Wait Function */
  unsigned long tick;

  tick = timeval;
  while ((timeval - tick) < time);          /* Wait for specified Time */
}


extern void init_timer (void);              /* Initialize Timer */
extern void init_serial (void);             /* Initialize Serial Interface */

int main (void) {

  init_timer();                             /* Initialize Timer */
  init_serial();                            /* Initialize Serial Interface */

  while (1) {                               /* Loop forever */
    pPIO->PDATE &= ~0x00000001;             /* Turn LED On  (PE0 = 1) */
    printf ("Hello World !\n");             /* Print "Hello World !" */
    wait(100);                              /* Wait 100ms */
    pPIO->PDATE |=  0x00000001;             /* Turn LED Off (PE0 = 0) */
    wait(900);                              /* Wait 900ms */
  }
}
