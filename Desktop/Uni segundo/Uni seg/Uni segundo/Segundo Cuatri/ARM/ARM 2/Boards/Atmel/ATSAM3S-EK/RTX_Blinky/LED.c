/*----------------------------------------------------------------------------
 * Name:    LED.c
 * Purpose: AT91SAM3S-EK low level LED functions
 * Note(s):
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2010 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <SAM3S.H>                            /* ATSAM3S definitions          */
#include "LED.h"

const unsigned long led_mask[] = { (1 << 20), (1 << 21) };

/*----------------------------------------------------------------------------
  initialize LED Pins
 *----------------------------------------------------------------------------*/
void LED_init (void) {

  PMC->PMC_WPMR = 0x504D4300;                 /* Disable write protect        */
  PMC->PMC_PCER1 = (1 << ID_PIOC);            /* enable clock for PIOC        */  

  PIOC->PIO_PER  = ((1 << 20) | (1 << 21));   /* Setup LED Pins PC19..PC20    */
  PIOC->PIO_OER  = ((1 << 20) | (1 << 21));
  PIOC->PIO_PUDR = ((1 << 20) | (1 << 21));
  PIOC->PIO_OWER = ((1 << 20) | (1 << 21));

  PIOC->PIO_SODR = ((1 << 20) | (1 << 21));   /* Turn LEDs Off                */
}


/*----------------------------------------------------------------------------
  Switch LED on. LED = Bitposition
 *----------------------------------------------------------------------------*/
void LED_on  (unsigned int led) {
  int i;

  for (i = 0; i < LED_NUM; i++) {
    if (led & (1<<i)) {
      PIOC->PIO_CODR = led_mask[i];
    }
  }
}

/*----------------------------------------------------------------------------
  Switch Led off. LED = Bitposition
 *----------------------------------------------------------------------------*/
void LED_off (unsigned int led) {
  int i;

  for (i = 0; i < LED_NUM; i++) {
    if (led & (1<<i)) {
      PIOC->PIO_SODR = led_mask[i];
    }
  }
}

/*----------------------------------------------------------------------------
  Output value to LEDs
 *----------------------------------------------------------------------------*/
void LED_out (unsigned int led) {
  int i;

  for (i = 0; i < LED_NUM; i++) {
    if (led & (1<<i)) {
      PIOC->PIO_CODR = led_mask[i];
    } else {
      PIOC->PIO_SODR = led_mask[i];
    }
  }
}
