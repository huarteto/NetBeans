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
#include <AT91SAM7X256.H>                   /* AT91SAMT7X256 definitions */
#include "..\AT91SAM7X-EK.h"


AT91S_PIO * pPIO_Led = AT91D_BASE_PIO_LED;  /* Global Pointer to PIO */
AT91S_PIO * pPIO_Sw  = AT91D_BASE_PIO_SW;   /* Global Pointer to PIO */


extern volatile unsigned long timeval;      /* Current Time Tick */

void wait (unsigned long time) {            /* Wait Function */
  unsigned long tick;

  tick = timeval;
  while ((timeval - tick) < time);          /* Wait for specified Time */
}


extern void init_pit    (void);             /* Initialize Timer */
extern void init_tc0    (void);             /* Initialize Timer  Counter 0 */
extern void init_serial (void);             /* Initialize Serial Interface */


/*
 * Main Program
 */

int main (void) {
  unsigned long n;

  *AT91C_PMC_PCER = (1 << AT91C_ID_PIOA) |  /* Enable Clock for PIOA (SWs)  */
                    (1 << AT91C_ID_PIOB) |  /* Enable Clock for PIOB (LEDs) */
                    (1 << AT91C_ID_IRQ0);   /* Enable Clock for IRQ0   */

  pPIO_Led->PIO_PER  = AT91B_LED_MASK;      /* Enable PIO for LED1..4  */
  pPIO_Led->PIO_OER  = AT91B_LED_MASK;      /* LED1..4 are Outputs     */
  pPIO_Led->PIO_SODR = AT91B_LED_MASK;      /* Turn off LED's ("1")    */
  pPIO_Led->PIO_OWER = AT91B_LED3;          /* LED3 ODSR Write Enable  */
  pPIO_Led->PIO_OWER = AT91B_LED4;          /* LED4 ODSR Write Enable  */

  init_pit   ();                            /* Initialize Timer */
  init_tc0   ();                            /* Initialize Timer Counter 0  */
  init_serial();                            /* Initialize Serial Interface */

  while (1) {                               /* Loop forever */
    n = pPIO_Sw->PIO_PDSR;                  /* Read Pin Data */
    if ((n & AT91B_SW1) == 0) {             /* Check if SW1 is pressed */
      pPIO_Led->PIO_CODR = AT91B_LED1;      /* Turn On LED1 */
      printf ("Hello World !\n");           /* Print "Hello World !" */
      wait(100);                            /* Wait 100ms */
      pPIO_Led->PIO_SODR = AT91B_LED1;      /* Turn Off LED1 */
      wait(100);                            /* Wait 100ms */
    }
    if ((n & AT91B_SW3) == 0)       {       /* Check if SW3 is pressed */
      pPIO_Led->PIO_CODR = AT91B_LED2;      /* Turn On LED2 */
    }
    if ((n & AT91B_SW4) == 0) {             /* Check if SW4 is pressed */
      pPIO_Led->PIO_SODR = AT91B_LED2;      /* Turn Off LED2 */
    }
  }
}
