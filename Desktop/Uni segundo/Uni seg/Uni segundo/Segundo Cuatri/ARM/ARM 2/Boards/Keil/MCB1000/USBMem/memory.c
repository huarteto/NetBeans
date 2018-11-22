/*----------------------------------------------------------------------------
 * Name:    memory.c
 * Purpose: USB Memory Storage Demo
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
#include "usbcfg_LPC13xx.h"
#include "usbhw_LPC13xx.h"
#include "mscuser.h"

#include "memory.h"


unsigned int LED_State = 0;                 /* reflect the LEDs */

extern U8 Memory[MSC_MemorySize];           /* MSC Memory in RAM */


/*------------------------------------------------------------------------------
  Set value to LEDs
 *------------------------------------------------------------------------------*/
void LED_Set (unsigned int value) {
  static unsigned long led_mask[] = { 1<<0, 1<<1, 1<<2, 1<<3, 1<<4, 1<<5, 1<<6, 1<<7 };
  int i;

  for (i = 0; i < LED_NUM; i++) {
    if (value & (1<<i)) {
      LPC_GPIO2->DATA &= ~led_mask[i];
    } else {
      LPC_GPIO2->DATA |=  led_mask[i];
    }
  }
}


/*------------------------------------------------------------------------------
  Switch on LEDs
 *------------------------------------------------------------------------------*/
void LED_On (unsigned int value) {

  LED_State |= value;
  LED_Set(LED_State);
}


/*------------------------------------------------------------------------------
  Switch off LEDs
 *------------------------------------------------------------------------------*/
void LED_Off (unsigned int value) {

  LED_State &= ~value;
  LED_Set(LED_State);
}


/*------------------------------------------------------------------------------
  Main Program
 *------------------------------------------------------------------------------*/
int main (void) {
  U32 n;

  for (n = 0; n < MSC_ImageSize; n++) {       /* Copy Initial Disk Image */
    Memory[n] = DiskImage[n];                 /*   from Flash to RAM     */
  }

  LPC_SYSCON->SYSAHBCLKCTRL |= 1 << 6;        /* Enable clock for GPIO              */
  LPC_GPIO2->DIR |=  LED_MASK;                /* LEDs on PORT2 defined as Output    */

  SystemInit();                               /* initialize clocks */

  USB_Init();                                 /* USB Initialization */
  USB_Connect(__TRUE);                        /* USB Connect */

  while (1);                                  /* Loop forever */
}
