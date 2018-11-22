/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2008 Keil Software. All rights reserved.                     */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <TMPM330.H>                    // TMPM330 definitions


volatile unsigned long SysTickCnt;      // SysTick Counter

// SysTick Setup
void SysTick_Setup (void) {
  SysTick->LOAD = 40000 - 1;            // 1ms: 40000 ticks @ 40MHz
  SysTick->CTRL = 0x0007;               // Enable Counter and Tick Interrupt
}

// SysTick Interrupt Handler (tick each 1ms)
void SysTick_Handler (void) {
  SysTickCnt++;
}


// Delay Function
void Delay (unsigned long tick) {
  unsigned long systickcnt;

  systickcnt = SysTickCnt;
  while ((SysTickCnt - systickcnt) < tick);
}


/* Main Program */
int main (void) {

  /* Disable Watchdog */
  TSB_WD->MOD  = 0x00;                  // Clear WDTE
  TSB_WD->CR   = 0xB1;                  // Disable Code

  /* Setup Clock */
  TSB_CG->OSCCR  = 0x14;                // Enable PLL
  TSB_CG->PLLSEL = 0x01;                // Switch to PLL: 40MHz (XTAL=10MHz)

  /* Setup I/O */
  TSB_PJ->PUP  = 0x01;                  // PJ.0 with enabled Pull-up
  TSB_PJ->IE   = 0x01;                  // PJ.0 defined as Input (Push Button)
  TSB_PI->CR   = 0x01;                  // PI.0 defined as Output (LED)

  SysTick_Setup();                      // SysTick Setup

  while (1) {                           // Loop forever
    if (TSB_PJ->DATA & 0x01) {          // Check SW3 Button (PJ.0)
      TSB_PI->DATA ^= 0x01;             // Toggle LED (PI.0)
      Delay(500);                       // Delay 500ms
    }
  }
}
