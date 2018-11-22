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
 * Copyright (c) 2009 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <LPC29xx.H>                        /* LPC29xx definitions */

#include "type.h"

#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"

#include "demo.h"
#include "GLCD.h"


U8 InReport;                                /* HID Input Report    */
                                            /*   Bit0   : Buttons  */
                                            /*   Bit1..7: Reserved */

U8 OutReport;                               /* HID Out Report      */
                                            /*   Bit0..7: LEDs     */


/*------------------------------------------------------------------------------
  Get HID Input Report -> InReport
 *------------------------------------------------------------------------------*/
void GetInReport (void) {

  if ((GPIO2_PINS & PB_INT0) == PB_INT0) {  /* Check if PBINT is pressed */
    InReport = 0x00;
  } else {
    InReport = 0x01;
  }
}


/*------------------------------------------------------------------------------
  Set HID Output Report <- OutReport
 *------------------------------------------------------------------------------*/
void SetOutReport (void) {

  GPIO2_OR &= ~LED_MSK;
  GPIO2_OR |=  (OutReport << 16);	
}


/* Main Program */

int main (void) {

  /* Configure the BUTTON's GPIO pins as Inputs with internal pull-up         */
  SFSP2_26  =  (0x03   << 2);
  SFSP2_27  =  (0x03   << 2);
  GPIO2_DR &= ~(0x03UL << 26);

  /* Configure the LED's GPIO pins as Outputs: P2.16 -- P2.23 */
  GPIO2_DR |=  (0xFF << 16);
  

  GLCD_Init ();                             /* GLCD initialization */ 
  GLCD_Clear(White);                        /* Clear the GLCD */

  GLCD_SetBackColor(Blue);                  /* Set the Back Color */
  GLCD_SetTextColor(White);                 /* Set the Text Color */

  GLCD_DisplayString(3, 0, "  MCB2929 HID Demo    ");
  GLCD_DisplayString(6, 0, "    www.keil.com      ");


  USB_Init();                               /* USB Initialization */
  USB_Connect(__TRUE);                      /* USB Connect */

  while (1);                                /* Loop forever */
}
