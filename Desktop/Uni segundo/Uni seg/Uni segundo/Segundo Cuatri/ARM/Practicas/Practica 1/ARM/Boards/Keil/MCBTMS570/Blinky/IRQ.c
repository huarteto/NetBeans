/******************************************************************************/
/* IRQ.c:                                                                     */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2010 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <TMS570.h>

extern void GetLightAndTemp (unsigned int * Light, unsigned int * Temp);


#define pRTI      ((RTI_ST     *)RTI)

unsigned int AD_Light_value;
unsigned int AD_Temp_value;
unsigned int clock_1s;                  /* Flag activated each second         */

/* Import function for turning LEDs on or off                                 */
extern void LED_Out (unsigned int num);
unsigned int clock_1s;

/* RTICNT1 IRQ: Executed periodically                                         */
__irq void RTICMP0_IRQHandler (void) {
  static unsigned long ticks;
  static unsigned long timetick;
  static unsigned char leds = 0x01;

  pRTI->RTIINTFLAG_UN.RTIINTFLAG_UL = 1;              /* Clear interrupt flag */

  if (ticks++ >= 99) {                  /* Set Clock1s to 1 every 1 second    */
    ticks    = 0;
    clock_1s = 1;
  }

  /* Blink the LEDs depending on ADC_ConvertedValue                           */
  if (timetick++ >= (AD_Light_value >> 7)) {
    timetick   = 0;
    leds     <<= 1;
    if (leds > 0x20) leds = 0x01;
    LED_Out (leds);
  }

  GetLightAndTemp (&AD_Light_value, &AD_Temp_value);
}
