/*----------------------------------------------------------------------------
 * Name:    LED.c
 * Purpose: AT91SAM3U-EK low level LED functions
 * Version: V1.00
 * Note(s):
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2009 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------
 * History:
 *          V1.00 Initial Version
 *----------------------------------------------------------------------------*/

#include <SAM3U.H>                            /* ATSAM3U definitions          */
#include "LED.h"

const unsigned long led_mask[] = { PIO_PB0, PIO_PB1 };

/*----------------------------------------------------------------------------
  initialize LED Pins
 *----------------------------------------------------------------------------*/
void LED_init (void) {

  PMC->PMC_PCER |= (1UL << ID_PIOB);          /* enable clock for PIOB        */  

  PIOB->PIO_IDR  = 
  PIOB->PIO_OER  =
  PIOB->PIO_PER  = (PIO_PB1|PIO_PB0);         /* Setup Pins PB0..PB1 for LEDs */
  PIOB->PIO_SODR = (PIO_PB1|PIO_PB0);         /* Turn LEDs Off                */
}


/*----------------------------------------------------------------------------
  Switch LED on. LED = Bitposition
 *----------------------------------------------------------------------------*/
void LED_on  (unsigned char led) {
  int i;

  for (i = 0; i < LED_NUM; i++) {
    if (led & (1<<i)) {
      PIOB->PIO_CODR = led_mask[i];
    }
  }
}

/*----------------------------------------------------------------------------
  Switch Led off. LED = Bitposition
 *----------------------------------------------------------------------------*/
void LED_off (unsigned char led) {
  int i;

  for (i = 0; i < LED_NUM; i++) {
    if (led & (1<<i)) {
      PIOB->PIO_SODR = led_mask[i];
    }
  }
}

/*----------------------------------------------------------------------------
  Output value to LEDs
 *----------------------------------------------------------------------------*/
void LED_out (unsigned char led) {
  int i;

  for (i = 0; i < LED_NUM; i++) {
    if (led & (1<<i)) {
      PIOB->PIO_CODR = led_mask[i];
    } else {
      PIOB->PIO_SODR = led_mask[i];
    }
  }
}
