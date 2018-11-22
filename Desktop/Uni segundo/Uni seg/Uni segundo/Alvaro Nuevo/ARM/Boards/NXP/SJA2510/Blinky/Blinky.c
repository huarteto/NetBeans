/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <SJA2510.H>

#define MAX_LED_SPEED         (50000)
#define MIN_LED_SPEED         (1000000)
#define CHG_LED_SPEED         (50000)

#define LED_NUM               6

const unsigned int led_mask[] = { 1 << 24, 1 << 25, 1 << 26, 1 << 27, 1 << 28, 1 << 29 };
const unsigned int but_mask[] = { 1 << 30, 1UL << 31 };

unsigned int LEDSpeed = (MIN_LED_SPEED - MAX_LED_SPEED) / 2;


/* Change LED blinking speed depending on buttons pressed                     */
void change_speed (void) {
  if (!(GPIO1_PINS & but_mask[0])) {    // SW1002 - speed up
    if (LEDSpeed > MAX_LED_SPEED) LEDSpeed -= CHG_LED_SPEED;
    else                          LEDSpeed  = MAX_LED_SPEED;
  }
  if (!(GPIO1_PINS & but_mask[1])) {    // SW1001 - slow down
    if (LEDSpeed < MIN_LED_SPEED) LEDSpeed += CHG_LED_SPEED;
    else                          LEDSpeed  = MIN_LED_SPEED;
  }
}


/* Wait function                                                              */
void wait (void) {
  unsigned int n;

  change_speed();
  for (n = 0; n < LEDSpeed; n++);
}
  

int main (void) {

  int num = -1; 
  int dir =  1;

  // Configure the LED's GPIO pins as Outputs
  GPIO1_DR |=  (0x3F << 24);

  // Configure the BUTTON's GPIO pins as Inputs with internal pull-up
  SFSP1_30  =  (0x03   << 2);
  SFSP1_31  =  (0x03   << 2);
  GPIO1_DR &= ~(0x03UL << 30);

  for (;;) {                            // Loop forever

    // Calculate 'num' to go: 0, 1, ... , LED_NUM-1, LED_NUM-1, ... , 1, 0, 0, ...
    num += dir;
    if (num == LED_NUM) { dir = -1; num =  LED_NUM-1; } 
    else if   (num < 0) { dir =  1; num =  0;         }

    // Turn ON and OFF LED with index 'num'
    GPIO1_OR |=  led_mask[num];         // Turn LED on
    wait();
    GPIO1_OR &= ~led_mask[num];         // Turn LED off
    wait();
  }
}
