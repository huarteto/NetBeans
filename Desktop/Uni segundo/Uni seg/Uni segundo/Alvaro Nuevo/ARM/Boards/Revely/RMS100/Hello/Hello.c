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
#include <LH75401.H>                        /* LH75401 definitons */


extern volatile unsigned long timeval;      /* Current Time Tick */

void wait (void)  {                         /* Wait function */
  unsigned long tick;

  tick = timeval;
  while ((timeval - tick) < 50);            /* Wait 500ms */
}


extern void init_timer (void);              /* Initialize Timer */
extern void init_serial (void);             /* Initialize Serial Interface */

int main (void)  {

  PCDR  = 0xC0;                             /* PC.6 & PC.7 Output Value */
  PCDDR = 0xC0;                             /* Define PC.6 & PC.7 as Outputs */

  init_timer();                             /* Initialize Timer */
  init_serial();                            /* Initialize Serial Interface */

  while (1) {                               /* Loop forever */
    printf ("Hello World\n");               /* Print "Hello World" */
    PCDR |=  0x40;                          /* Turn LED1 On  (PC.6 = 1) */
    PCDR &= ~0x80;                          /* Turn LED2 Off (PC.7 = 0) */
    wait();                                 /* Call Wait Function */
    PCDR &= ~0x40;                          /* Turn LED1 Off (PC.6 = 0) */
    PCDR |=  0x80;                          /* Turn LED2 On  (PC.7 = 1) */
    wait();                                 /* Call Wait Function */
  }
}
