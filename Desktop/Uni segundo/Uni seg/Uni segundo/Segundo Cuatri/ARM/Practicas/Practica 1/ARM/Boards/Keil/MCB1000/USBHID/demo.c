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
 * Copyright (c) 2009 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <LPC13xx.H>                        /* LPC13xx definitions */

#include "type.h"

#include "usb.h"
#include "usbhw_LPC13xx.h"

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
  uint32_t kbd_val;

  kbd_val = LPC_GPIO0->DATA & KBD_MASK;

  InReport = 0x00;
  if ((kbd_val & KBD_BOOT) == 0) InReport |= 0x01;  /* key pressed */


}


/*------------------------------------------------------------------------------
  Set HID Output Report <- OutReport
 *------------------------------------------------------------------------------*/
void SetOutReport (void) {
  static unsigned long led_mask[] = { 1<<0, 1<<1, 1<<2, 1<<3, 1<<4, 1<<5, 1<<6, 1<<7 };
  int i;

  for (i = 0; i < LED_NUM; i++) {
    if (OutReport & (1<<i)) {
      LPC_GPIO2->DATA &= ~led_mask[i];
    } else {
      LPC_GPIO2->DATA |=  led_mask[i];
    }
  }
}


/*------------------------------------------------------------------------------
  Main Program
 *------------------------------------------------------------------------------*/
int main (void) {

  LPC_SYSCON->SYSAHBCLKCTRL |= 1 << 6;        /* Enable clock for GPIO              */
  LPC_GPIO0->DIR &= ~KBD_MASK;                /* Push Button on P0.1 is input       */
  LPC_GPIO2->DIR |=  LED_MASK;                /* LEDs on PORT2 defined as Output    */

  SystemInit();                               /* initialize clocks */

  USB_Init();                                 /* USB Initialization */
  USB_Connect(__TRUE);                        /* USB Connect */

  while (1);                                  /* Loop forever */
}
