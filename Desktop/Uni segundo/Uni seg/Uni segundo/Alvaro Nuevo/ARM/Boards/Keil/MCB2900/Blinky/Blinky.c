/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <stdio.h>
#include <LPC29xx.H>
#include "LCD.h"                        /* LCD function prototypes            */

#define TMR_CLK      80000000           /* In Hz                              */

#define LED_NUM      6                  /* Number of LEDs used for blinking   */

const unsigned int led_mask[] = { 1 << 24, 1 << 25, 1 << 26, 1 << 27, 1 << 28, 1 << 29 };
const unsigned int but_mask[] = { 1 << 30, 1UL << 31 };

/* Import external functions from Serial.c file                               */
extern       void init_serial    (void);

extern          char Speed;
extern unsigned char Clock;
extern unsigned char Clock1s;
extern short         AD_last;


/* Change LED blinking speed depending on buttons pressed                     */
void change_speed (void) {
  if (!(GPIO1_PINS & but_mask[0])) {    /* S1 - speed up                      */
    if (Speed >  0) Speed --;
  }
  if (!(GPIO1_PINS & but_mask[1])) {    /* S2 - slow down                     */
    if (Speed < 10) Speed ++;
  }
}


int main (void) {

  int i;
  int num = -1; 
  int dir =  1;
  short AD_old, AD_value, AD_print;
  static short LEDState;

  Speed = 5;                            /* Initial speed at half              */

  /* Configure the LED's GPIO pins as Outputs                                 */
  GPIO1_DR |=  (0x3F << 24);

  /* Configure the BUTTON's GPIO pins as Inputs with internal pull-up         */
  SFSP1_30  =  (0x03   << 2);
  SFSP1_31  =  (0x03   << 2);
  GPIO1_DR &= ~(0x03UL << 30);

  /* Configure MSCSS clock = PLL clock / 2 = 80 MHz (max allowed 80 MHz)      */
  MSCSS_CLK_CONF   = (2UL << 24) | (1 << 11) | (1 << 2);

  MTIM0_PR   = (80000000 / 1000) - 1;   /* Prescaler to drive timer with 1 ms */
  MTIM0_MR2  = (10 - 1);                /* Match value = 10 * 1ms = 10 ms     */
  MTIM0_MCR  = (1 << 4);                /* On Match 2 event reset timer       */
  MTIM0_EMR |= (3 << 8);                /* Match 2 event generate logic 1     */
  MTIM0_TCR  = 1;                       /* Start Timer 0                      */

  /* Configure ADC clock = FDIV0 / 1 = 4 MHz (max allowed 4.5 MHz)            */
  ADC_CLK_CONF     = (5UL << 24) | (1 << 11) | (0 << 2);

  ADC2_CTRL  = (1<<1);                  /* Stop  ADC conversion               */

  SFSP0_16         = 1;                 /* Configure P0.16 as analog input    */

  /* Setup ADC2 interrupt generation                                          */
  INT_REQUEST_18   = (1  << 28) |       /* Enable setting of priority level   */
                     (1  << 27) |       /* Enable setting interrupt target    */
                     (1  << 26) |       /* Write enable of new settings       */
                     (1  << 16) |       /* Enable interrupt request           */
                     (1  <<  8) |       /* Interrupt target is IRQ interrupt  */
                     (15 <<  0) ;       /* Priority level 15, disable nesting */

  /* Setup ADC2 IN0 to convert in 10-bit resolution and generate interrupt    */
  ADC2_ACC0        = 0x0000000A;        /* Set ADC2 IN0 10-bit resolution     */
  ADC2_CONFIG      = (1 << 14);         /* ADC single scan, start on pos edge */
  ADC2_CTRL        = 4;                 /* Copy the configuration             */
  ADC2_INT_SET_ENABLE = 1;              /* Enable end of scan interrupt       */

  /* Configure timer clock = PLL clock / 2 = 80 MHz                           */
  TMR_CLK_CONF = (2UL << 24) | (1 << 11) | (1 << 2);

  /* Setup Timer 0 interrupt generation                                       */
  INT_REQUEST_2    = (1 << 28) |        /* Enable setting of priority level   */
                     (1 << 27) |        /* Enable setting interrupt target    */
                     (1 << 26) |        /* Write enable of new settings       */
                     (1 << 16) |        /* Enable interrupt request           */
                     (1 <<  8) |        /* Interrupt target is IRQ interrupt  */
                     (15 << 0) ;        /* Priority level 15, disable nesting */

  /* Setup Timer 0 to generate interrupt every 10 ms                          */
  TIM0_PR   = (TMR_CLK / 1000) - 1;     /* Prescaler to drive timer with 1 ms */
  TIM0_MR0  = (10 - 1);                 /* Match value = 10 * 1ms = 10 ms     */
  TIM0_MCR  = 1;                        /* On Match 0 event reset timer       */
  TIM0_INT_SET_ENABLE = 1;              /* Enable Match 0 to generate interupt*/
  TIM0_TCR  = 1;                        /* Start Timer 0                      */

  init_serial ();                       /* Init UART                          */

  LCD_init ();
  LCD_cls ();
  LCD_print (0, 0, "  MCB2900 DEMO  ");
  LCD_print (0, 1, "  www.keil.com  ");

  for (i = 0; i < 12000000; i++);       /* Wait for initial display           */

  for (;;) {                            /* Loop forever                       */

    if (Clock) {                        /* LED activate and deactivate        */
      Clock     = 0;
      LEDState ^= 0x01;

      change_speed();                   /* Check buttons to change LED speed  */

      if (LEDState) {
        GPIO1_OR |=  led_mask[num];     /* Turn LED on                        */
      } else  {
        GPIO1_OR &= ~led_mask[num];     /* Turn LED off                       */

        /* Calculate 'num': 0,1,...,LED_NUM-1,LED_NUM-1,...,1,0,0,...         */
        num += dir;
        if (num == LED_NUM) { dir = -1; num =  LED_NUM-1; } 
        else if   (num < 0) { dir =  1; num =  0;         }
      }
    }

    AD_value = AD_last;                 /* Read AD_last value                 */
    if (AD_value != AD_last)            /* Make sure that AD interrupt did    */
      AD_value = AD_last;               /* not interfere with value reading   */
    AD_print  = AD_value;               /* Get unscaled value for printout    */
    AD_value /= 10;                     /* Scale to AD_Value to 0 - 100       */
    if (AD_old != AD_value)  {          /* If AD value has changed            */
      AD_old = AD_value;
      LCD_bargraph (0, 1, 16, AD_value);/* Display bargraph according to AD   */
    }

    if (Clock1s) {                      /* Output AD value on UART every 1 sec*/
      Clock1s = 0;
      printf ("AD value = 0x%04x\n\r", AD_print);
    }
  }
}
