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

#include <AT91SAM7X256.H>                /* AT91SAM7X256 definitions         */
#include "../../AT91SAM7X-EK.h"

#include "type.h"

#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "mscuser.h"

#include "memory.h"


AT91S_PIO * pPIO_Led = AT91D_BASE_PIO_LED;  /* Global Pointer to PIO */

extern U8 Memory[MSC_MemorySize];           /* MSC Memory in RAM */


/*------------------------------------------------------------------------------
  Switch on LEDs
 *------------------------------------------------------------------------------*/
void LED_On (unsigned int value) {
  pPIO_Led->PIO_CODR = (value);             /* Turn On  LED */
}


/*------------------------------------------------------------------------------
  Switch off LEDs
 *------------------------------------------------------------------------------*/
void LED_Off (unsigned int value) {
  pPIO_Led->PIO_SODR = (value);             /* Turn Off LED */
}


/*------------------------------------------------------------------------------
  Main Program
 *------------------------------------------------------------------------------*/
int main (void) {
  U32 n;

  for (n = 0; n < MSC_ImageSize; n++) {     /* Copy Initial Disk Image */
    Memory[n] = DiskImage[n];               /*   from Flash to RAM     */
  }

  /* Enable Clock for PIO */
  AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_PIOB);         /* LEDs     */

  pPIO_Led->PIO_PER  = LED_MSK;             /* Enable PIO for LED1..4  */
  pPIO_Led->PIO_OER  = LED_MSK;             /* LED1..4 are Outputs     */
  pPIO_Led->PIO_SODR = LED_MSK;             /* Turn off LED's ("1")    */

  USB_Init();                               /* USB Initialization */
  USB_Connect(__TRUE);                      /* USB Connect */

  while (1);                                /* Loop forever */
}
