/*----------------------------------------------------------------------------
 * Name:    demo.c
 * Purpose: USB HID Demo
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2008 - 2010 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <AT91SAM7X256.H>                /* AT91SAM7X256 definitions         */
#include "../../AT91SAM7X-EK.h"

#include "type.h"

#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"

#include "demo.h"


AT91S_PIO * pPIO_Led = AT91D_BASE_PIO_LED;  /* Global Pointer to PIO */
AT91S_PIO * pPIO_Sw  = AT91D_BASE_PIO_SW;   /* Global Pointer to PIO */


U8 InReport;                                /* HID Input Report */
                                            /*   Bit0..4: Buttons */
                                            /*   Bit5..7: Reserved */

U8 OutReport;                               /* HID Out Report */
                                            /*   Bit0..3: LEDs */
                                            /*   Bit4..7: Reserved */


/*
 *  Get HID Input Report -> InReport
 */

void GetInReport (void) {
  U32 key;

  key = pPIO_Sw->PIO_PDSR;                        /* Read Pin Data */
  InReport = 0x00;
  if ((key & AT91B_SW1) == 0) InReport |= 0x01;   /* Check if SW1 is pressed */
  if ((key & AT91B_SW2) == 0) InReport |= 0x02;   /* Check if SW2 is pressed */
  if ((key & AT91B_SW3) == 0) InReport |= 0x04;   /* Check if SW3 is pressed */
  if ((key & AT91B_SW4) == 0) InReport |= 0x08;   /* Check if SW4 is pressed */
  if ((key & AT91B_SW5) == 0) InReport |= 0x10;   /* Check if SW5 is pressed */
}


/*
 *  Set HID Output Report <- OutReport
 */

void SetOutReport (void) {

  if (OutReport & 0x01) pPIO_Led->PIO_CODR = AT91B_LED1; else pPIO_Led->PIO_SODR = AT91B_LED1;
  if (OutReport & 0x02) pPIO_Led->PIO_CODR = AT91B_LED2; else pPIO_Led->PIO_SODR = AT91B_LED2;
  if (OutReport & 0x04) pPIO_Led->PIO_CODR = AT91B_LED3; else pPIO_Led->PIO_SODR = AT91B_LED3;
  if (OutReport & 0x08) pPIO_Led->PIO_CODR = AT91B_LED4; else pPIO_Led->PIO_SODR = AT91B_LED4;
}


/* Main Program */

int main (void) {

  /* Enable Clock for PIO */
  AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_PIOA);         /* Joystick */
  AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_PIOB);         /* LEDs     */

  pPIO_Led->PIO_PER  = AT91B_LED_MASK;      /* Enable PIO for LED1..4  */
  pPIO_Led->PIO_OER  = AT91B_LED_MASK;      /* LED1..4 are Outputs     */
  pPIO_Led->PIO_SODR = AT91B_LED_MASK;      /* Turn off LED's ("1")    */

  USB_Init();                               /* USB Initialization */
  USB_Connect(__TRUE);                      /* USB Connect */

  while (1);                                /* Loop forever */
}
