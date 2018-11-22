/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <AT91SAM7A3.H>                     /* AT91SAM7A3 definitions */
#include "..\Board.h"

#define TCK  1000                           /* Timer Clock  */
#define PIV  ((MCK/TCK/16)-1)               /* Periodic Interval Value */


AT91S_PIO  * pPIOA = AT91C_BASE_PIOA;       /* Global Pointer to PIOA */
AT91S_PIO  * pPIOB = AT91C_BASE_PIOB;       /* Global Pointer to PIOB */
AT91S_AIC  * pAIC  = AT91C_BASE_AIC;        /* Global Pointer to AIC  */
AT91S_PITC * pPIT  = AT91C_BASE_PITC;       /* Global Pointer to PIT */


const int LED_Mask[] = {                    /* LED I/O Mask */
  0, LED1, LED2, LED3, LED4
};

unsigned char Pos   = 0;                    /* Current Position */
unsigned char Dir   = 0;                    /* Current Direction */
unsigned int  Mask  = 0;                    /* Current Invert Mask */
unsigned int  Delay = 500;                  /* Current Delay */
unsigned int  JS    = 0xFFFFFFFF;           /* Current Joystick Values */

volatile unsigned int TimeTick;             /* Current Time Tick */


/*
 * Scan Joystick and setup global variables
 */

void joystick (void) {
  unsigned int js;

  js = pPIOB->PIO_PDSR;                     /* Read Joystick Pins */
  if (((js & JS_PUSH) == 0) &&              /* Push pressed */
      ((JS & JS_PUSH) != 0)) {
    Mask ^= 0xFFFFFFFF;                     /* Change Mask */
    pPIOA->PIO_SODR = LED_MASK;             /* Turn Off all LED's */
    pPIOA->PIO_CODR = LED_Mask[Pos] ^ Mask; /* Turn On selected LED's */
  }
  if ((js & JS_LEFT) == 0) {                /* Left -> Change Direction */
    Dir = 0;                                /* Direction is Left */
  }
  if ((js & JS_RIGHT) == 0) {               /* Right -> Change Direction */
    Dir = 1;                                /* Direction is Right */
  }
  if ((js & JS_DOWN) == 0) {                /* Down -> Slower */
    if (Delay < 1000) Delay += 10;          /* Increase Delay */
  }
  if ((js & JS_UP) == 0) {                  /* Up -> Faster */
    if (Delay > 50)   Delay -= 10;          /* Decrease Delay */
  }
  JS = js;                                  /* Save Joystick Values */
}


/*
 * System Interrupt Service Routine
 *  executed every 1ms
 */

void sys_irq (void) __irq {

  if (pPIT->PITC_PISR & AT91C_PITC_PITS) {  /* Check PIT Interrupt */
    TimeTick++;                             /* Increment Time Tick */
    if ((TimeTick & 0x1F) == 0) {           /* Check every 32ms */
      joystick();                           /* Scan Joystick */
    }
    pAIC->AIC_EOICR = pPIT->PITC_PIVR;      /* Ack & End of Interrupt */
  } else {
    pAIC->AIC_EOICR = 0;                    /* End of Interrupt */
  }
}


/*
 * Main Program
 */

int main (void) {
  unsigned int tick;

  *AT91C_PMC_PCER = (1 << AT91C_ID_PIOA) |  /* Enable Clock for PIOA */
                    (1 << AT91C_ID_PIOB);   /* Enable Clock for PIOB */

  pPIOA->PIO_PER  = LED_MASK;               /* Enable PIO for LED's  */
  pPIOA->PIO_OER  = LED_MASK;               /* LED's are Outputs     */
  pPIOA->PIO_SODR = LED_MASK;               /* Turn Off LED's ("1")  */

  pPIT->PITC_PIMR = AT91C_PITC_PITIEN |     /* PIT Interrupt Enable */
                    AT91C_PITC_PITEN  |     /* PIT Enable */
                    PIV;                    /* Periodic Interval Value */

  /* Setup System Interrupt Mode and Vector with Priority 7 and Enable it */
  pAIC->AIC_SMR[AT91C_ID_SYS] = AT91C_AIC_SRCTYPE_INT_POSITIVE_EDGE | 7;
  pAIC->AIC_SVR[AT91C_ID_SYS] = (unsigned long) sys_irq;
  pAIC->AIC_IECR = (1 << AT91C_ID_SYS);

  while (1) {                               /* Loop forever */
    pPIOA->PIO_SODR = LED_MASK;             /* Turn Off all LED's */
    pPIOA->PIO_CODR = LED_Mask[Pos] ^ Mask; /* Turn On selected LED's */
    tick = TimeTick;                        /* Get Initial TimeTick */
    while ((TimeTick - tick) < Delay);      /* Wait for specified Delay */
    if (Dir) {                              /* Direction is Down */
      if (Pos > 0) {
        Pos--;                              /* Change Position */
      } else {
        Pos = LED_CNT;                      /* Change Position (Wrap) */
      }
    } else {                                /* Direction is Up */
      if (Pos < LED_CNT) {
        Pos++;                              /* Change Position */
      } else {
        Pos = 0;                            /* Change Position (Wrap) */
      }
    }
  }
}
