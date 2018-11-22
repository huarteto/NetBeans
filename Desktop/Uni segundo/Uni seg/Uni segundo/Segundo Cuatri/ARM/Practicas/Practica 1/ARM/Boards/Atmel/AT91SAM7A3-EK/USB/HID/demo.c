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

#include <AT91SAM7A3.H>                     /* AT91SAM7A3 definitions */

#include "..\..\Board.h"

#include "type.h"

#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "usbcore.h"

#include "demo.h"


AT91S_PIO * pPIOA = AT91C_BASE_PIOA;        /* Global Pointer to PIOA */
AT91S_PIO * pPIOB = AT91C_BASE_PIOB;        /* Global Pointer to PIOB */


BYTE InReport;                              /* HID Input Report */
                                            /*   Bit0..4: Buttons */
                                            /*   Bit5..7: Reserved */

BYTE OutReport;                             /* HID Out Report */
                                            /*   Bit0..3: LEDs */
                                            /*   Bit4..7: Reserved */


/*
 *  Get HID Input Report -> InReport
 */

void GetInReport (void) {
  DWORD key;

  key = pPIOB->PIO_PDSR;                     /* Read Pin Data */

  InReport = 0x00;
  if ((key & JS_LEFT)  == 0) InReport |= 0x01;  /* Check if Left is pressed */
  if ((key & JS_RIGHT) == 0) InReport |= 0x02;  /* Check if Right is pressed */
  if ((key & JS_UP)    == 0) InReport |= 0x04;  /* Check if Up is pressed */
  if ((key & JS_DOWN)  == 0) InReport |= 0x08;  /* Check if Down is pressed */
  if ((key & JS_PUSH)  == 0) InReport |= 0x10;  /* Check if Push is pressed */
}


/*
 *  Set HID Output Report <- OutReport
 */

void SetOutReport (void) {

  if (OutReport & 0x01) pPIOA->PIO_CODR = LED1; else pPIOA->PIO_SODR = LED1;
  if (OutReport & 0x02) pPIOA->PIO_CODR = LED2; else pPIOA->PIO_SODR = LED2;
  if (OutReport & 0x04) pPIOA->PIO_CODR = LED3; else pPIOA->PIO_SODR = LED3;
  if (OutReport & 0x08) pPIOA->PIO_CODR = LED4; else pPIOA->PIO_SODR = LED4;
}


/* Main Program */

int main (void) {

  *AT91C_PMC_PCER = (1 << AT91C_ID_PIOA) |  /* Enable Clock for PIOA */
                    (1 << AT91C_ID_PIOB);   /* Enable Clock for PIOB */

  pPIOA->PIO_PER  = LED_MASK;               /* Enable PIO for LED's  */
  pPIOA->PIO_OER  = LED_MASK;               /* LED's are Outputs     */
  pPIOA->PIO_SODR = LED_MASK;               /* Turn Off LED's ("1")  */

  USB_Init();                               /* USB Initialization */
  USB_Connect(TRUE);                        /* USB Connect */

  while (1);                                /* Loop forever */
}
