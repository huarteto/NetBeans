/*----------------------------------------------------------------------------
 * Name:    KBD.c
 * Purpose: AT91SAM3U-EK low level PushButton functions
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
 * Copyright (c) 2008 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------
 * History:
 *          V1.00 Initial Version
 *----------------------------------------------------------------------------*/

#include <SAM3U.H>                               /* ATSAM3U definitions       */
#include "KBD.h"


/*----------------------------------------------------------------------------
  initialize LED Pins
 *----------------------------------------------------------------------------*/
void KBD_init (void) {

  PMC->PMC_PCER |= (1UL << ID_PIOA);             /* enable clock for PIOA     */  

  PIOA->PIO_IDR  = 
  PIOA->PIO_ODR  = 
  PIOA->PIO_PER  = (PIO_PA19 | PIO_PA18);        /* Setup Pins PA18..PA19 for Buttons */

}


/*----------------------------------------------------------------------------
  Get Push Button Values value
 *----------------------------------------------------------------------------*/
uint32_t KBD_get  (void) {
  uint32_t kbd_val;

  kbd_val = (PIOA->PIO_PDSR >> 18) & KBD_MASK;   /* 0 = pressed, 1 = released */
  return (kbd_val);
}

