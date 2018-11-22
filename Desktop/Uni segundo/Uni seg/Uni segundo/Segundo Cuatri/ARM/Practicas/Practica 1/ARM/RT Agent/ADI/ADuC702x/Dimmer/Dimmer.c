/******************************************************************************/
/* DIMMER.C: LED Dimmer                                                       */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <ADuC7026.H>               // ADuC7026 definitions
#include "RT_Agent.h"               // Real-Time Agent definitions

#define CLOCK   22544384            // CPU configured for 22.544384 MHz clock
#define T0_FREQ       96            // Timer 0 Reload Frequency

#define T0_LD   ((unsigned short)(CLOCK/16 / T0_FREQ))


extern volatile int T0_Tick;        // Timer 0 Current Tick Value
                int T0_Last;        // Timer 0 Last Tick Value

int main (void) {
  static unsigned char dac;

  RTA_Init();                       // Initialise the Real-Time Agent

  DAC1CON = 0x13;                   // Configure DAC1 (LED) Range 0..VDD

  IRQEN = 0x00000004;               // Enable Timer 0 Interrupt
  T0LD  = T0_LD;                    // Timer 0 Load Value
  T0CON = 0xC4;                     // Timer 0 Enabled, Periodic, Prescaler = 16

  dac = 0;                          // Initial DAC (8-bit) Value
  while (1) {                       // Loop forever
    while (T0_Tick == T0_Last);     // Synchronize to Timer Interrupt
    T0_Last = T0_Tick;
    DAC1DAT = dac << 20;            // Update DAC1 Output
    if ((GP0DAT & 0x10) == 0) {     // Check if IRQ (P0.4) Button is not active
      dac++;                        // Increment DAC Value
      if (dac == 192) dac = 0;      // Check DAC Limit
    }
  }
}
