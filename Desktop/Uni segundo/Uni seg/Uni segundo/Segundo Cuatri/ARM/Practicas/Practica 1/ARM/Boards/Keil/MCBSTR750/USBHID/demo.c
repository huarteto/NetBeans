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

#include <75x_lib.H>                    /* STR750 definitions                */

#include "type.h"

#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"

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
  if ((GPIO1->PD & S2) == 0) InReport |= 0x01; /* 0 means S2 pressed         */
  if ((GPIO1->PD & S3) == 0) InReport |= 0x02; /* 0 means S3 pressed         */
}


/*----------------------------------------------------------------------------
  Set HID Output Report <- OutReport
 *----------------------------------------------------------------------------*/
void SetOutReport (void) {
  GPIO2->PM  &= ~LEDMSK;                       /* enable bit 0..7            */
  GPIO2->PD   =  OutReport;	                   /* All LED Bits are set       */
  GPIO2->PM  |=  LEDMSK;                       /* mask   bit 0..7            */
}



/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {

  /* Push Button Setup */
  GPIO1->PM  &= ~0xA000;                    /* enable bit 13, 15             */
  GPIO1->PC0 |=  0xA000;                    /* P1.13,15 input                */
  GPIO1->PC1 |=  0xA000;
  GPIO1->PC2 &= ~0xA000;

  /* LED Setup */
  GPIO2->PM  &= ~0x00FF;                    /* enable bit 0..7               */
  GPIO2->PC0 |=  0x00FF;                    /* P2.00..07 output push-pull    */
  GPIO2->PC1 &= ~0x00FF;
  GPIO2->PC2 |=  0x00FF;                     
  GPIO2->PD  &= ~0x00FF;                    /* clear all LEDs                */
  GPIO2->PM  |=  0x00FF;                    /* mask   bit 0..7               */

  lcd_init();
  lcd_clear();
  lcd_print (" MCBSTR750  HID ");
  set_cursor (0, 1);
  lcd_print ("  www.keil.com  ");

  USB_Init();                               /* USB Initialization */
  USB_Connect(__TRUE);                      /* USB Connect */

  while (1);                                /* Loop forever */
}
