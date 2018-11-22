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

#include <71x_lib.H>

#include "type.h"

#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "mscuser.h"

#include "memory.h"

extern U8 Memory[MSC_MemorySize];           /* MSC Memory in RAM */

/*------------------------------------------------------------------------------
  Switch on LEDs
 *------------------------------------------------------------------------------*/
void LED_On (unsigned int value) {

  GPIO2->PD  |=  (value);                   /* Turn On  LED    */
}


/*------------------------------------------------------------------------------
  Switch off LEDs
 *------------------------------------------------------------------------------*/
void LED_Off (unsigned int value) {

  GPIO2->PD  &= ~(value);                   /* Turn Off LED    */
}


/*------------------------------------------------------------------------------
  Main Program
 *------------------------------------------------------------------------------*/
int main (void) {
  U32 n;

  RCCU_MCLKConfig(RCCU_DEFAULT);                    /* MCLK = RCLK */
  RCCU_FCLKConfig(RCCU_RCLK_2);                     /* FCLK = RCLK/2 */
  RCCU_PCLKConfig(RCCU_RCLK_4);                     /* PCLK = RCLK/4 */
  RCCU_PLL1Config(RCCU_PLL1_Mul_12, RCCU_Div_2);    /* 48MHz PLL @ 16MHz XTAL */

  while (RCCU_FlagStatus(RCCU_PLL1_LOCK) == RESET); /* Wait for PLL to Lock */
  RCCU_RCLKSourceConfig(RCCU_PLL1_Output);          /* Select PLL for RCLK */

  /* P2.8..15 (LED) defined as Outputs (Push-pull) */
  GPIO2->PC0 |=  LED_MSK;
  GPIO2->PC1 &= ~LED_MSK;
  GPIO2->PC2 |=  LED_MSK;
  GPIO2->PD  &= ~LED_MSK;

  for (n = 0; n < MSC_ImageSize; n++) {     /* Copy Initial Disk Image */
    Memory[n] = DiskImage[n];               /*   from Flash to RAM     */
  }

  USB_Init();                               /* USB Initialization */
  USB_Connect(__TRUE);                      /* USB Connect */

  while (1);                                /* Loop forever */
}
