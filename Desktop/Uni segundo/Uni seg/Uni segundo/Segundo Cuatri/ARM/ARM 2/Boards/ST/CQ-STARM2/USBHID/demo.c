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
                                            /*   Bit0..7: Buttons  */

U8 OutReport;                               /* HID Out Report      */
                                            /*   Bit0..7: LEDs     */


/*----------------------------------------------------------------------------
  Get HID Input Report -> InReport
 *----------------------------------------------------------------------------*/
void GetInReport (void) {

  InReport = 0x00;
  if ((GPIOE->IDR & SW1) == 0) InReport |= 0x01;    // SW1 pressed means 0
  if ((GPIOE->IDR & SW2) == 0) InReport |= 0x02;    // SW2 pressed means 0
  if ((GPIOE->IDR & SW3) == 0) InReport |= 0x04;    // SW3 pressed means 0
  if ((GPIOE->IDR & SW4) == 0) InReport |= 0x08;    // SW4 pressed means 0
  if ((GPIOE->IDR & SW5) == 0) InReport |= 0x10;    // SW5 pressed means 0
  if ((GPIOE->IDR & SW6) == 0) InReport |= 0x20;    // SW6 pressed means 0
  if ((GPIOE->IDR & SW7) == 0) InReport |= 0x40;    // SW7 pressed means 0
  if ((GPIOE->IDR & SW8) == 0) InReport |= 0x80;    // SW8 pressed means 0
}


/*----------------------------------------------------------------------------
  Set HID Output Report <- OutReport
 *----------------------------------------------------------------------------*/
void SetOutReport (void) {

  if (OutReport & 0x01) GPIOD->BSRR |= (1<<15); else GPIOD->BRR = (1<<15); 
  if (OutReport & 0x02) GPIOD->BSRR |= (1<< 8); else GPIOD->BRR = (1<< 8); 
  if (OutReport & 0x04) GPIOE->BSRR |= (1<<14); else GPIOE->BRR = (1<<14); 
  if (OutReport & 0x08) GPIOD->BSRR |= (1<<11); else GPIOD->BRR = (1<<11); 
  if (OutReport & 0x10) GPIOC->BSRR |= (1<<11); else GPIOC->BRR = (1<<11); 
  if (OutReport & 0x20) GPIOD->BSRR |= (1<< 2); else GPIOD->BRR = (1<< 2); 
  if (OutReport & 0x40) GPIOD->BSRR |= (1<< 7); else GPIOD->BRR = (1<< 7); 
  if (OutReport & 0x80) GPIOD->BSRR |= (1<<12); else GPIOD->BRR = (1<<12); 
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
