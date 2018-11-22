/*----------------------------------------------------------------------------
 * Name:    LED.c
 * Purpose: Toshiba TMPM395 LED functions
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

#include "TMPM395.h"                    /* TMPM395 definitions                */
#include "LED.h"                        /* LED     definitions                */

#define LED_PG2    (1UL<<2)
#define LED_PB3    (1UL<<3)
const unsigned long led_mask[] = { LED_PG2, LED_PB3 };


/*----------------------------------------------------------------------------
 *  Initialize LED Pins
 *----------------------------------------------------------------------------*/
void LED_Init (void) {
  TSB_PB->CR   |=  LED_PB3;             /* Set LED pins to outputs            */
  TSB_PB->IE   &= ~LED_PB3;             /* Disable LED pins input             */
  TSB_PB->DATA |=  LED_PB3;             /* Turn all LEDs initially off        */

  TSB_PG->CR   |=  LED_PG2;             /* Set LED pins to outputs            */
  TSB_PG->IE   &= ~LED_PG2;             /* Disable LED pins input             */
  TSB_PG->DATA |=  LED_PG2;             /* Turn all LEDs initially off        */
}


/*----------------------------------------------------------------------------
 *  Switch LED On
 *----------------------------------------------------------------------------*/
void LED_On (uint32_t num) {

  if (num < 1) TSB_PG->DATA &= ~led_mask[num];
  else         TSB_PB->DATA &= ~led_mask[num];
}

/*----------------------------------------------------------------------------
 *  Switch Led Off
 *----------------------------------------------------------------------------*/
void LED_Off (uint32_t num) {

  if (num < 1) TSB_PG->DATA |=  led_mask[num];
  else         TSB_PB->DATA |=  led_mask[num];
}

/*----------------------------------------------------------------------------
 *  Output Value to LEDs
 *----------------------------------------------------------------------------*/
void LED_Out(uint32_t value) {
  int i;

  for (i = 0; i < LED_NUM; i++) {
    if (value & (1<<i)) {
      LED_On (i);
    } else {
      LED_Off(i);
    }
  }
}
