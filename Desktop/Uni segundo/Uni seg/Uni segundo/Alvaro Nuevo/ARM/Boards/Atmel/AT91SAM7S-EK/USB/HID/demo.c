/*----------------------------------------------------------------------------
 *      Name:    DEMO.C
 *      Purpose: USB HID Demo
 *      Version: V1.10
 *----------------------------------------------------------------------------
 *      This file is part of the uVision/ARM development tools.
 *      Copyright (c) 2005-2006 Keil Software. All rights reserved.
 *      This software may only be used under the terms of a valid, current,
 *      end user licence from KEIL for a compatible version of KEIL software
 *      development tools. Nothing else gives you the right to use it.
 *---------------------------------------------------------------------------*/

#include <AT91SAM7S64.H>                    /* AT91SAM7S64 definitions */

#include "..\..\Board.h"

#include "type.h"

#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "usbcore.h"

#include "demo.h"


AT91S_PIO * pPIO = AT91C_BASE_PIOA;         /* Global Pointer to PIO */


BYTE InReport;                              /* HID Input Report */
                                            /*   Bit0..3: Buttons */
                                            /*   Bit4..7: Reserved */

BYTE OutReport;                             /* HID Out Report */
                                            /*   Bit0..3: LEDs */
                                            /*   Bit4..7: Reserved */


/*
 *  Get HID Input Report -> InReport
 */

void GetInReport (void) {
  DWORD key;

  key = pPIO->PIO_PDSR;                     /* Read Pin Data */
  InReport = 0x00;
  if ((key & SW1) == 0) InReport |= 0x01;   /* Check if SW1 is pressed */
  if ((key & SW2) == 0) InReport |= 0x02;   /* Check if SW2 is pressed */
  if ((key & SW3) == 0) InReport |= 0x04;   /* Check if SW3 is pressed */
  if ((key & SW4) == 0) InReport |= 0x08;   /* Check if SW4 is pressed */
}


/*
 *  Set HID Output Report <- OutReport
 */

void SetOutReport (void) {

  if (OutReport & 0x01) pPIO->PIO_CODR = LED1; else pPIO->PIO_SODR = LED1;
  if (OutReport & 0x02) pPIO->PIO_CODR = LED2; else pPIO->PIO_SODR = LED2;
  if (OutReport & 0x04) pPIO->PIO_CODR = LED3; else pPIO->PIO_SODR = LED3;
  if (OutReport & 0x08) pPIO->PIO_CODR = LED4; else pPIO->PIO_SODR = LED4;
}


/* Main Program */

int main (void) {

  /* Enable Clock for PIO */
  AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_PIOA);

  pPIO->PIO_PER  = LED_MASK;                /* Enable PIO for LED1..4  */
  pPIO->PIO_OER  = LED_MASK;                /* LED1..4 are Outputs     */
  pPIO->PIO_SODR = LED_MASK;                /* Turn off LED's ("1")    */

  USB_Init();                               /* USB Initialization */
  USB_Connect(TRUE);                        /* USB Connect */

  while (1);                                /* Loop forever */
}
