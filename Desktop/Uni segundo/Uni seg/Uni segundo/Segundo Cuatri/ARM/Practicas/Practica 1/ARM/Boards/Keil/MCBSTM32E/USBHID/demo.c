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
  if ((GPIOG->IDR & UP)     == 0) InReport |= 0x01;  /* up     pressed means 0 */
  if ((GPIOG->IDR & LEFT)   == 0) InReport |= 0x02;  /* left   pressed means 0 */
  if ((GPIOG->IDR & RIGHT)  == 0) InReport |= 0x04;  /* right  pressed means 0 */
  if ((GPIOG->IDR & SELECT) == 0) InReport |= 0x08;  /* select pressed means 0 */
  if ((GPIOD->IDR & DOWN)   == 0) InReport |= 0x10;  /* down   pressed means 0 */
}


/*----------------------------------------------------------------------------
  Set HID Output Report <- OutReport
 *----------------------------------------------------------------------------*/
void SetOutReport (void) {

  GPIOB->ODR = ((OutReport &0x3F) << 8);	         /* LED Bits PB8..PB13 are set */
}


/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {

  stm32_Init ();                                     /* STM32 Initialization */

  USB_Init();                                        /* USB Initialization */
  USB_Connect(__TRUE);                               /* USB Connect */

  while (1);                                         /* Loop forever */
}
