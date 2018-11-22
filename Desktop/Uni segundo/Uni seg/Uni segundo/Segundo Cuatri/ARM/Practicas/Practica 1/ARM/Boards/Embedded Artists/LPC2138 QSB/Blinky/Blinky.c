/******************************************************************************/
/* Blinky.c: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <LPC213x.H>                    /* LPC213x definitions                */
#include <stdio.h>

extern void init_serial (void);         /* Initialize Serial Interface        */


void delay (void) {                     /* Delay function                     */
  unsigned int cnt;

  for (cnt = 0; cnt < 2000000; cnt++);
}


int main (void) {
  unsigned int n;

  IODIR0 = 0x00FFFF00;                  /* P0.8..23 defined as Outputs        */
  init_serial();                        /* Initialize Serial Interface        */

  while (1) {                           /* Loop forever                       */
    for (n = 0x00000100; n <= 0x00800000; n <<= 1) {
      /* Blink LED1 .. LED16                                                  */
      IOCLR0 = n;                       /* Turn on LED                        */

      /* Output state of LEDs pins to serial UART0                            */
      printf ("LEDs state 0x%04X\r\n", (~n >> 8) & 0xFFFF);

      delay();                          /* Delay                              */
      IOSET0 = 0x00FFFF00;              /* Turn off LEDs                      */
    }
  }
}
