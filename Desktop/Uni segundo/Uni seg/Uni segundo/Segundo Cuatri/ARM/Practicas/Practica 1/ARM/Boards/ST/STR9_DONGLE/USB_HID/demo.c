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


U8 InReport;                                /* HID Input Report    */
                                            /*   Bit0..3: Buttons  */
                                            /*   Bit4..7: Reserved */

U8 OutReport;                               /* HID Out Report      */
                                            /*   Bit0..3: LEDs     */


/*----------------------------------------------------------------------------
  Get HID Input Report -> InReport
 *----------------------------------------------------------------------------*/
void GetInReport (void) {

  InReport = 0x00;
  if (GPIO7->DR[S1*4] == 0) InReport |= 0x01;  /* S1 pressed means 0 */
  if (GPIO7->DR[S2*4] == 0) InReport |= 0x02;  /* S2 pressed means 0 */
  if (GPIO7->DR[S3*4] == 0) InReport |= 0x04;  /* S3 pressed means 0 */
  if (GPIO7->DR[S4*4] == 0) InReport |= 0x08;  /* S4 pressed means 0 */
}


/*----------------------------------------------------------------------------
  Set HID Output Report <- OutReport
 *----------------------------------------------------------------------------*/
void SetOutReport (void) {

  GPIO6->DR[0x0F*4] = ~(OutReport & LED_MSK);  /* All LED Bits are set */
}


/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {

  /* Push Button Setup */
  SCU->GPIOOUT[7]   = 0x0000;               /* P7.0..7 no output              */
  GPIO7->DDR        = 0x00;                 /* P7.0..7 Input (Push Buttons)   */

  /* LED Setup */
  SCU->GPIOOUT[6]   = 0x0055;               /* P6.0..3 output - mode 1        */
  GPIO6->DDR        = 0x0F;                 /* P6.0..3 Outputs (LED Data)     */
  GPIO6->DR[0x0F*4] = LED_MSK;              /* clear LEDs                     */

  USB_Init();                               /* USB Initialization */
  USB_Connect(__TRUE);                      /* USB Connect */

  while (1);                                /* Loop forever */
}
