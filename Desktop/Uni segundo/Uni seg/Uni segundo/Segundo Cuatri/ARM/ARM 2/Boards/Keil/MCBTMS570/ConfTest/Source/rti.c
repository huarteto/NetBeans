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
//#include "rti.h"
//#include "sys_vim.h"
//#include "adc.h"
//#include "gio.h"
//#include "nhet.h"
#include "ConfidenceTest.h"


typedef void (*FuncPTR)(void);
#define pRTI      ((RTI_ST     *)RTI)
#define pVIM      ((VIM_ST     *)VIM)
#define pVIM_RAM  ((VIM_RAM_ST *)VIM_RAM)


#define RTICLK 80000000UL
#define RTICMP0 2

unsigned int clock_1s;                  /* Flag activated each second         */
static unsigned int BeepLen;


unsigned int clock_1s;

void beep (void) {

  BeepLen = 500;
}

#define LED_NUM    6                    /* Number of user LEDs                */
const unsigned long led_gioMask[]  = { 1UL<< 0, 1UL<< 3, 1UL<< 4,  1UL<< 5};
const unsigned long led_nhetMask[] = { 1UL<< 1, 1UL<< 31};

/*----------------------------------------------------------------------------
  Function that turns on requested LED
 *----------------------------------------------------------------------------*/
static void LED_On (unsigned int num) {
  if (num < 4) {
    gioPORTA->DSET = led_gioMask[num];
  } else {
    nhetREG->CCDSET = led_nhetMask[num-4];
  }
}

/*----------------------------------------------------------------------------
  Function that turns off requested LED
 *----------------------------------------------------------------------------*/
static void LED_Off (unsigned int num) {
  if (num < 4) {
    gioPORTA->DCLR = led_gioMask[num];
  } else {
    nhetREG->CCDCLR = led_nhetMask[num-4];
  }
}

/*----------------------------------------------------------------------------
  Function that outputs value to LEDs
 *----------------------------------------------------------------------------*/
void LED_Out(unsigned int value) {
  int i;

  for (i = 0; i < LED_NUM; i++) {
    if (value & (1<<i)) {
      LED_On (i);
    } else {
      LED_Off(i);
    }
  }
}

int CheckSwitch1 (void) {         // Switch 1 is connected to pin GIOB3 (active low)
  return (1 - ((gioPORTB->DIN >> 3) & 0x01));
}

int CheckSwitch2 (void) {         // Switch 2 is connected to pin GIOB6 (active low)
  return (1 - ((gioPORTB->DIN >> 6) & 0x01));
}

//#pragma INTERRUPT(RTICMP0_IRQHandler, IRQ)

/* RTICNT1 IRQ: Executed periodically                                         */
__irq void RTICMP0_IRQHandler (void) {
  static unsigned long ticks;
  static unsigned long timetick;
  static unsigned char leds = 0x01;

  pRTI->RTIINTFLAG_UN.RTIINTFLAG_UL = 1; /* Clear interrupt flag */
  if (ticks++ >= 999) {                  /* Set Clock1s to 1 every 1 second    */
    ticks    = 0;
    clock_1s = 1;
  }

  /* Blink the LEDs */
  if (timetick++ >= 100) {
    timetick   = 0;
    leds     <<= 1;
    if (leds > 0x20) leds = 0x01;
    LED_Out (leds);
  }
  if (BeepLen) {
    BeepLen--;
    if (BeepLen & 1) {
      nhetREG->CCDSET = bit30;
    } else {
      nhetREG->CCDCLR = bit30;
    }
  }
  /* Read buttons and start the beep */
  if (CheckSwitch1 ()) {                // if Switch 1 is pressed
    BeepLen = 500;                      // start beep
  }
  if (CheckSwitch2 ()) {                // if Switch 2 is pressed
    BeepLen = 1000;
  }
  
}

void TIM_init (void) {

  pVIM_RAM->ISR[RTICMP0+1] = (FuncPTR)RTICMP0_IRQHandler;
  pVIM->REQMASKSET0_UL   = (1UL << RTICMP0);

  pRTI->RTICPUC1_UL  = (RTICLK / 1000UL) - 1;                     /* 1ms tick */
  pRTI->RTICOMP0_UL  = 1;
  pRTI->RTIUDCP0_UL  = 1;
  pRTI->RTICOMPCTRL_UN.RTICOMPCTRL_ST.COMPSEL0_B1 = 1;
  pRTI->RTISETINT_UN.RTISETINT_ST.SETINT0_B1      = 1;

  pRTI->RTIGCTRL_UN.RTIGCTRL_ST.CNT1EN_B1         = 1;         /* enable CNT1 */
}

