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

#include <91x_lib.H>

#include "type.h"

#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"

#include "demo.h"
#include "lcd.h"


U8 InReport;                                /* HID Input Report    */
                                            /*   Bit0..1: Buttons  */
                                            /*   Bit2..7: Reserved */

U8 OutReport;                               /* HID Out Report      */
                                            /*   Bit0..7: LEDs     */


/*----------------------------------------------------------------------------
  Get HID Input Report -> InReport
 *----------------------------------------------------------------------------*/
void GetInReport (void) {

  InReport = 0x00;
  if (GPIO3->DR[S2*4] == 0) InReport |= 0x01;  /* S2 pressed means 0 */
  if (GPIO3->DR[S3*4] == 0) InReport |= 0x02;  /* S3 pressed means 0 */
}


/*----------------------------------------------------------------------------
  Set HID Output Report <- OutReport
 *----------------------------------------------------------------------------*/
void SetOutReport (void) {

  GPIO7->DR[0xFF*4] = OutReport;	        /* All LED Bits are set           */
}


/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {

  /* Push Button Setup */
  SCU->GPIOOUT[3]   = 0x0000;               /* P3.0..7 no output              */
  GPIO3->DDR        = 0x00;                 /* P3.0..7 Input (Push Buttons)   */

  /* LED Setup */
  SCU->GPIOOUT[7]   = 0x5555;               /* P7.0..7 output - mode 1        */
  GPIO7->DDR        = 0xFF;                 /* P7.0..7 Outputs (LED Data)     */
  GPIO7->DR[0xFF*4] = 0x00;                 /* clear LEDs                     */

  /* LCD Setup   */
  GPIO8->DDR       = 0xFF;                  /* P8.0..7 Outputs (LCD Data)     */
  GPIO9->DDR       = 0x07;                  /* P9.0..2 Outputs (LCD Control)  */

  lcd_init();
  lcd_clear();
  lcd_print ("MCBSTR9 HID Demo");
  set_cursor (0, 1);
  lcd_print ("  www.keil.com  ");

  USB_Init();                               /* USB Initialization */
  USB_Connect(__TRUE);                      /* USB Connect */

  while (1);                                /* Loop forever */
}
