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

  PFDR  = 0x06;                             /* PF.1 & PF.2 Output Value */
  PFDDR = 0x06;                             /* Define PF.1 & PF.2 as Outputs */

  init_timer();                             /* Initialize Timer */
  init_serial();                            /* Initialize Serial Interface */

  while (1) {                               /* Loop forever */
    printf ("Hello World\n");               /* Print "Hello World" */
    PFDR |=  0x04;                          /* Turn LED1 Off (PF.2 = 1) */
    PFDR &= ~0x02;                          /* Turn LED0 On  (PF.1 = 0) */
    wait();                                 /* Call Wait Function */
    PFDR |=  0x02;                          /* Turn LED0 Off (PF.1 = 1) */
    PFDR &= ~0x04;                          /* Turn LED1 On  (PF.2 = 0) */
    wait();                                 /* Call Wait Function */
  }
}
