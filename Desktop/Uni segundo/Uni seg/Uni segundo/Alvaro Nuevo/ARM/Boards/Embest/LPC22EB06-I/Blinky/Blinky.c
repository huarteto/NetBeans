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



void delay(void)
{
  int i;
  for(i=0; i<0xFFFF; i++); /* delay */
}

int main (void)  {

 
  IODIR2 = 0x00000030;                    /* P2.23 2.24 defined as Output */

  while (1) {                               /* Loop forever */
    IOCLR2 = 0x01000000;                    /* Turn LED On  (P2.23 = 0 */
    delay();
    IOSET2 = 0x01800000;                    /* Turn LED OFF  (P2.23 = 1) */
    delay();                                /* Wait */
    IOCLR2 = 0x00800000;                    /* Turn LED ON (P2.24 =0) */
    delay();
    IOSET2 = 0x01800000;                    /* Turn LED Off (P2.24 = 1) */
    delay();                                /* Wait */
  }
}
