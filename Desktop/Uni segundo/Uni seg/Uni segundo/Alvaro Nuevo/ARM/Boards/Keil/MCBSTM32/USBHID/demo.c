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

#include <stm32f10x_lib.h>                           // stm32f10x definitions

#include "type.h"

#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"

#include "STM32_Init.h"                              // stm32 initialisation
#include "demo.h"
#include "LCD.h"


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
  if ((GPIOA->IDR & S2) == 0) InReport |= 0x01;      /* S2 pressed means 0 */
  if ((GPIOC->IDR & S3) == 0) InReport |= 0x02;      /* S3 pressed means 0 */
}


/*----------------------------------------------------------------------------
  Set HID Output Report <- OutReport
 *----------------------------------------------------------------------------*/
void SetOutReport (void) {

  GPIOB->ODR = (OutReport << 8);	                 /* All LED Bits are set */
}


/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {

  stm32_Init ();                                     /* STM32 Initialization */

  lcd_init();
  lcd_clear();
  lcd_print ("  MCBSTM32 HID  ");
  set_cursor (0, 1);
  lcd_print ("  www.keil.com  ");

  USB_Init();                                        /* USB Initialization */
  USB_Connect(__TRUE);                               /* USB Connect */

  while (1);                                         /* Loop forever */
}
