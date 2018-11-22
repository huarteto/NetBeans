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
                                            /*   Bit0..6: Buttons  */
                                            /*   Bit   7: Reserved */

U8 OutReport;                               /* HID Out Report      */
                                            /*   Bit0..3: LEDs     */


/*----------------------------------------------------------------------------
  Get HID Input Report -> InReport
 *----------------------------------------------------------------------------*/
void GetInReport (void) {

  InReport = 0x00;
  if ((GPIOD->IDR & KEY2   ) == 0) InReport |= 0x01; // Key2   pressed means 0
  if ((GPIOD->IDR & KEY3   ) == 0) InReport |= 0x02; // Key3   pressed means 0
  if ((GPIOD->IDR & KEY4_SL) == 0) InReport |= 0x04; // Select pressed means 0
  if ((GPIOD->IDR & KEY4_RI) == 0) InReport |= 0x08; // Right  pressed means 0
  if ((GPIOD->IDR & KEY4_LE) == 0) InReport |= 0x10; // Left   pressed means 0
  if ((GPIOD->IDR & KEY4_UP) == 0) InReport |= 0x20; // Up     pressed means 0
  if ((GPIOD->IDR & KEY4_DN) == 0) InReport |= 0x40; // Down   pressed means 0
}


/*----------------------------------------------------------------------------
  Set HID Output Report <- OutReport
 *----------------------------------------------------------------------------*/
void SetOutReport (void) {

  GPIOC->BRR   = (0xF<<4);                           // LEDs off
  GPIOC->BSRR  = ((OutReport&0xF)<<4);               // LEDs on
}


/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {

  stm32_Init ();                                     /* STM32 Initialization */

  lcdWrStr ("KEIL");

  USB_Init();                                        /* USB Initialization */
  USB_Connect(__TRUE);                               /* USB Connect */

  while (1);                                         /* Loop forever */
}
