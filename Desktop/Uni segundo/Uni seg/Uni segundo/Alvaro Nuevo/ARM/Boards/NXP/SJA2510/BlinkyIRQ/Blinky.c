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

#define TMR_CLK      80000000           // In Hz

#define LED_NUM      6                  // Number of LEDs used for blinking

const unsigned int led_mask[] = { 1 << 24, 1 << 25, 1 << 26, 1 << 27, 1 << 28, 1 << 29 };
const unsigned int but_mask[] = { 1 << 30, 1UL << 31 };

extern          char Speed;
extern unsigned char Clock;


/* Change LED blinking speed depending on buttons pressed                     */
void change_speed (void) {
  if (!(GPIO1_PINS & but_mask[0])) {    // SW1002 - speed up
    if (Speed >  0) Speed --;
  }
  if (!(GPIO1_PINS & but_mask[1])) {    // SW1001 - slow down
    if (Speed < 10) Speed ++;
  }
}


/* Wait Function                                                              */
void wait (void) {

  change_speed();

  while (!Clock);
  Clock = 0;
}


int main (void) {

  int num = -1;
  int dir =  1;

  Speed = 5;                            // Initial speed at half

  // Configure the LED's GPIO pins as Outputs
  GPIO1_DR |=  (0x3F << 24);

  // Configure the BUTTON's GPIO pins as Inputs with internal pull-up
  SFSP1_30  =  (0x03   << 2);
  SFSP1_31  =  (0x03   << 2);
  GPIO1_DR &= ~(0x03UL << 30);

  // Configure timer clock = PLL clock / 2 = 80 MHz
  TMR_CLK_CONF = (2UL << 24) | (1 << 11) | (1 << 2);

  // Setup Timer 0 interrupt generation
  INT_REQUEST_2 = (1 << 28) |           // Enable setting of priority level
                  (1 << 27) |           // Enable setting interrupt target 
                  (1 << 26) |           // Write enable of new settings
                  (1 << 16) |           // Enable interrupt request
                  (1 <<  8) |           // Interrupt target is IRQ interrupt
                  (1 <<  0) ;           // Set priority level to 1

  // Setup Timer 0 to generate interrupt every 10 ms
  TIM0_PR   = (TMR_CLK / 1000) - 1;     // Prescaler to drive timer with 1 ms
  TIM0_MR0  = (10 - 1);                 // Match value = 10 * 1ms = 10 ms
  TIM0_MCR  = 1;                        // On Match 0 event reset timer
  TIM0_INT_SET_ENABLE = 1;              // Enable Match 0 to generate interrupt
  TIM0_TCR  = 1;                        // Start Timer 0

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
