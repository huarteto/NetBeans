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

#include <71x_lib.H>

#include "type.h"

#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"

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
  if ((GPIO1->PD & S2) == 0) InReport |= 0x01;
  if ((GPIO0->PD & S3) != 0) InReport |= 0x02;
}


/*----------------------------------------------------------------------------
  Set HID Output Report <- OutReport
 *----------------------------------------------------------------------------*/
void SetOutReport (void) {

  GPIO2->PD &= ~LEDMSK;
  GPIO2->PD |= OutReport << 8;
}


/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {

  RCCU_MCLKConfig(RCCU_DEFAULT);                    /* MCLK = RCLK */
  RCCU_FCLKConfig(RCCU_RCLK_2);                     /* FCLK = RCLK/2 */
  RCCU_PCLKConfig(RCCU_RCLK_4);                     /* PCLK = RCLK/4 */
  RCCU_PLL1Config(RCCU_PLL1_Mul_12, RCCU_Div_2);    /* 48MHz PLL @ 16MHz XTAL */

  while (RCCU_FlagStatus(RCCU_PLL1_LOCK) == RESET); /* Wait for PLL to Lock */
  RCCU_RCLKSourceConfig(RCCU_PLL1_Output);          /* Select PLL for RCLK */

  /* P2.8..15 (LED) defined as Outputs (Push-pull) */
  GPIO2->PC0 |=  LEDMSK;
  GPIO2->PC1 &= ~LEDMSK;
  GPIO2->PC2 |=  LEDMSK;
  GPIO2->PD  &= ~LEDMSK;

  USB_Init();                               /* USB Initialization */
  USB_Connect(__TRUE);                      /* USB Connect */

  while (1);                                /* Loop forever */
}
