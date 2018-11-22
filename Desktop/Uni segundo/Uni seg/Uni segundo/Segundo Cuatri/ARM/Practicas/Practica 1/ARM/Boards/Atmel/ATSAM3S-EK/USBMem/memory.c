/*----------------------------------------------------------------------------
 * Name:    memory.c
 * Purpose: USB Memory Storage Demo
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

#include <SAM3S.H>                         /* ATSAM3S definitions             */

#include "type.h"

#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "mscuser.h"

#include "memory.h"




extern U8 Memory[MSC_MemorySize];                      /* MSC Memory in RAM */


/*------------------------------------------------------------------------------
  Switch on LEDs
 *------------------------------------------------------------------------------*/
void LED_On (unsigned int value) {

  PIOC->PIO_CODR = (value);                            /* Turn On  LED */
}


/*------------------------------------------------------------------------------
  Switch off LEDs
 *------------------------------------------------------------------------------*/
void LED_Off (unsigned int value) {

  PIOC->PIO_SODR = (value);                            /* Turn Off LED */
}


/*------------------------------------------------------------------------------
  Main Program
 *------------------------------------------------------------------------------*/
int main (void) {
  U32 n;

  SystemInit();                                        /* System setup            */

  for (n = 0; n < MSC_ImageSize; n++) {                /* Copy Initial Disk Image */
    Memory[n] = DiskImage[n];                          /*   from Flash to RAM     */
  }

  PMC->PMC_WPMR = 0x504D4300;                          /* Disable write protect   */
  /* Enable Clock for PIO */
  PMC->PMC_PCER0 = (1 << ID_PIOC);                     /* enable clock for LEDs   */

  PIOC->PIO_IDR   =                                    /* Setup Pins PC20..PC21 for LEDs */
  PIOC->PIO_OER   =
  PIOC->PIO_PER   = PIO_PC20 | PIO_PC21;
  PIOC->PIO_SODR  = PIO_PC20 | PIO_PC21;               /* Turn LEDs Off           */

  USB_Init();                                          /* USB Initialization      */
  USB_Connect(__TRUE);                                 /* USB Connect             */

  while (1);                                           /* Loop forever            */
}
