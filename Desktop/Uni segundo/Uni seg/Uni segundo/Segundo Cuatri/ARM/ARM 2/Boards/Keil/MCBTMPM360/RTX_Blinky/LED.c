/*----------------------------------------------------------------------------
 * Name:    LED.c
 * Purpose: Toshiba TMPM364 LED functions
 * Note(s):
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2010 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <TMPM364.h>                    /* Toshiba TMPM364 definitions        */

#define  LED_ALL   0xF0


/*----------------------------------------------------------------------------
 *  Initialize LED Pins
 *----------------------------------------------------------------------------*/
void LED_Init (void) {
  TSB_PM->CR   |=  LED_ALL;             /* Set LED pins to outputs            */
  TSB_PM->FR1  &= ~LED_ALL;             /* Set LED pins function as "Port"    */
  TSB_PM->OD   |=  LED_ALL;             /* Set LED pins as Open-drain         */
  TSB_PM->PUP  |=  LED_ALL;             /* Enable  LED pins pull ups          */
  TSB_PM->IE   &= ~LED_ALL;             /* Disable LED pins input             */
  TSB_PM->DATA |=  LED_ALL;             /* Turn all LEDs initially off        */
}


/*----------------------------------------------------------------------------
 *  Switch LED On
 *----------------------------------------------------------------------------*/
void LED_On (int led) {
  TSB_PM->DATA &= ~((1 << 4) << led);
}

/*----------------------------------------------------------------------------
 *  Switch Led Off
 *----------------------------------------------------------------------------*/
void LED_Off (int led) {
  TSB_PM->DATA |=  ((1 << 4) << led);
}

/*----------------------------------------------------------------------------
 *  Output Value to LEDs
 *----------------------------------------------------------------------------*/
void LED_Out (int val) {
  TSB_PM->DATA = ((TSB_PM->DATA & 0x0F) | ((~val) << 4));
}
