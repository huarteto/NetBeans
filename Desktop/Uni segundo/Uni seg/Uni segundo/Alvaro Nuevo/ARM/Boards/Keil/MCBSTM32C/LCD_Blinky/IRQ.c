/******************************************************************************/
/* IRQ.c: IRQ Handlers                                                        */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2009 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/


unsigned long ticks = 0;
unsigned char ClockLEDOn;               /* On for 100 ms every 150 ms         */
unsigned char ClockLEDOff;              /* On for  50 ms every 150 ms         */
unsigned char ClockANI;                 /* Clock for Animation 150 ms         */


void SysTick_Handler (void) {           /* SysTick Interrupt Handler (10ms)   */
  ticks++;
  switch (ticks) {
    case 10:
      ClockLEDOff = 1;
      break;
  case 15:
      ticks       = 0;
      ClockANI    = 1;
      ClockLEDOn  = 1;
    default:
      break;
  }
}
