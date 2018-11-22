/*----------------------------------------------------------------------------
 * Name:    memory.c
 * Purpose: USB Memory Storage Demo
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

#include <75x_lib.H>                    /* STR750 definitions                */

#include "type.h"

#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "mscuser.h"

#include "memory.h"
#include "LCD.h"

extern U8 Memory[MSC_MemorySize];           /* MSC Memory in RAM */

/*------------------------------------------------------------------------------
  Switch on LEDs
 *------------------------------------------------------------------------------*/
void LED_On (unsigned int value) {

  GPIO2->PM  &= ~LED_MSK;                   /* enable bit 0..7 */
  GPIO2->PD  |=  (value);                   /* Turn On  LED    */
  GPIO2->PM  |=  LED_MSK;                   /* mask   bit 0..7 */
}


/*------------------------------------------------------------------------------
  Switch off LEDs
 *------------------------------------------------------------------------------*/
void LED_Off (unsigned int value) {

  GPIO2->PM  &= ~LED_MSK;                   /* enable bit 0..7 */
  GPIO2->PD  &= ~(value);                   /* Turn Off LED    */
  GPIO2->PM  |=  LED_MSK;                   /* mask   bit 0..7 */
}


/*------------------------------------------------------------------------------
  Main Program
 *------------------------------------------------------------------------------*/
int main (void) {
  U32 n;


  /* LED Setup */
  GPIO2->PM  &= ~0x00FF;                    /* enable bit 0..7               */
  GPIO2->PC0 |=  0x00FF;                    /* P2.00..07 output push-pull    */
  GPIO2->PC1 &= ~0x00FF;
  GPIO2->PC2 |=  0x00FF;                     
  GPIO2->PD  &= ~0x00FF;                    /* clear all LEDs                */
  GPIO2->PM  |=  0x00FF;                    /* mask   bit 0..7               */

  for (n = 0; n < MSC_ImageSize; n++) {     /* Copy Initial Disk Image */
    Memory[n] = DiskImage[n];               /*   from Flash to RAM     */
  }

  lcd_init();
  lcd_clear();
  lcd_print (" MCBSTR750  MSD ");
  set_cursor (0, 1);
  lcd_print ("  www.keil.com  ");

  USB_Init();                               /* USB Initialization */
  USB_Connect(__TRUE);                      /* USB Connect */

  while (1);                                /* Loop forever */
}
