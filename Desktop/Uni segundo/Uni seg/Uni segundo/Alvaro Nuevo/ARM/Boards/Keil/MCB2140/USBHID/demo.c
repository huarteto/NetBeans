/*----------------------------------------------------------------------------
 * Name:    demo.c
 * Purpose: USB HID Demo
 * Version: V1.20
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2008 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <LPC214X.H>                        /* LPC214x definitions */

#include "type.h"

#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"

#include "demo.h"


U8 InReport;                                /* HID Input Report    */
                                            /*   Bit0   : Buttons  */
                                            /*   Bit1..7: Reserved */

U8 OutReport;                               /* HID Out Report      */
                                            /*   Bit0..7: LEDs     */


/*------------------------------------------------------------------------------
  Get HID Input Report -> InReport
 *------------------------------------------------------------------------------*/
void GetInReport (void) {

  if ((IOPIN0 & PB_INT0) == 0) {            /* Check if PBINT is pressed */
    InReport = 0x01;
  } else {
    InReport = 0x00;
  }
}


/*------------------------------------------------------------------------------
  Set HID Output Report <- OutReport
 *------------------------------------------------------------------------------*/
void SetOutReport (void) {

  IOPIN1 = (IOPIN1 & ~LED_MSK) | (OutReport << 16);
}


/*------------------------------------------------------------------------------
  Main Program
 *------------------------------------------------------------------------------*/
int main (void) {

  IODIR1 = LED_MSK;                         /* LED's defined as Outputs */

  USB_Init();                               /* USB Initialization */
  USB_Connect(__TRUE);                      /* USB Connect */

  while (1);                                /* Loop forever */
}
