/******************************************************************************/
/* HELLO.C: Hello World Example                                               */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <stdio.h>                          /* I/O Functions */
#include <AT91SAM7S64.H>                    /* AT91SAM7S64 definitions */
#include "..\Board.h"                       /* Development board definititons */
#include "RT_Agent.h"                       /* Real-Time Agent definitions */


AT91S_PIO * pPIO = AT91C_BASE_PIOA;         /* Global Pointer to PIO */

extern volatile unsigned long timeval;      /* Current Time Tick */

void wait (unsigned long time) {            /* Wait Function */
  unsigned long tick;

  tick = timeval;
  while ((timeval - tick) < time);          /* Wait for specified Time */
}


extern void init_system_int  (void);        /* Initialize System Interrupt */
extern void init_extint (void);             /* Initialize External Interrupt */
extern void init_serial (void);             /* Initialize Serial Interface */


/*
 * Main Program
 */

int main (void) {
  unsigned long n;

  *AT91C_PMC_PCER = (1 << AT91C_ID_PIOA) |  /* Enable Clock for PIO    */
                    (1 << AT91C_ID_IRQ0) |  /* Enable Clock for IRQ0   */
                    (1 << AT91C_ID_US0);    /* Enable Clock for USART0 */

  pPIO->PIO_PER  = LED_MASK;                /* Enable PIO for LED1..4  */
  pPIO->PIO_OER  = LED_MASK;                /* LED1..4 are Outputs     */
  pPIO->PIO_SODR = LED_MASK;                /* Turn off LED's ("1")    */
  pPIO->PIO_OWER = LED4;                    /* LED4 ODSR Write Enable  */

  init_system_int ();                       /* Initialize System Interrupt */
  init_extint();                            /* Initialize External Interrupt */
  init_serial();                            /* Initialize Serial Interface */
  RTA_Init();                               /* Initialize Real-Time Agent */

  RTA_printf("\nHello World Example\n\n");  /* Say 'hello' to RTA terminal */
  printf("\nHello World Example\n\n");      /* Say 'hello' to stdout (redirected to com1) */

  while (1) {                               /* Loop forever */
    n = pPIO->PIO_PDSR;                     /* Read Pin Data */
    if ((n & SW1) == 0) {                   /* Check if SW1 is pressed */
      pPIO->PIO_CODR = LED1;                /* Turn On LED1 */
      RTA_printf ("Real-Time Agent!\n");    /* Print to the RTA Terminal */
      printf ("Com Port 1!\n");             /* Print to stdout (redirected to com1) */
      wait(100);                            /* Wait 100ms */
      pPIO->PIO_SODR = LED1;                /* Turn Off LED1 */
      wait(100);                            /* Wait 100ms */
    }
    if (((n & SW3) == 0) && (n & SW4)) {    /* Check if SW3 is pressed */
      pPIO->PIO_CODR = LED3;                /* Turn On LED3 */
    }
    if ((n & SW4) == 0) {                   /* Check if SW4 is pressed */
      pPIO->PIO_SODR = LED3;                /* Turn Off LED3 */
    }
  }
}
