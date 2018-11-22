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

#include <91x_lib.H>

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

  GPIO7->DR[0xFF*4] |= (value);             /* Turn On  LED */
}


/*------------------------------------------------------------------------------
  Switch off LEDs
 *------------------------------------------------------------------------------*/
void LED_Off (unsigned int value) {

  GPIO7->DR[0xFF*4] &= ~(value);            /* Turn Off LED */
}


/*------------------------------------------------------------------------------
  Main Program
 *------------------------------------------------------------------------------*/
int main (void) {
  U32 n;

  /* LED Setup */
  SCU->GPIOOUT[7]   = 0x5555;               /* P7.0..7 output - mode 1        */
  GPIO7->DDR        = 0xFF;                 /* P7.0..7 Outputs (LED Data)     */
  GPIO7->DR[0xFF*4] = 0x00;                 /* clear LEDs                     */

  /* LCD Setup   */
  GPIO8->DDR       = 0xFF;                  /* P8.0..7 Outputs (LCD Data)     */
  GPIO9->DDR       = 0x07;                  /* P9.0..2 Outputs (LCD Control)  */


  for (n = 0; n < MSC_ImageSize; n++) {     /* Copy Initial Disk Image */
    Memory[n] = DiskImage[n];               /*   from Flash to RAM     */
  }

  lcd_init();
  lcd_clear();
  lcd_print ("MCBSTR9 MSD Demo");
  set_cursor (0, 1);
  lcd_print ("  www.keil.com  ");

  USB_Init();                               /* USB Initialization */
  USB_Connect(__TRUE);                      /* USB Connect */

  while (1);                                /* Loop forever */
}
