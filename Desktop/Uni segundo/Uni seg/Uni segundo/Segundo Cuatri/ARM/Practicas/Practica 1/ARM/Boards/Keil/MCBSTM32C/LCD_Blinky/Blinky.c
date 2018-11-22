/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2009 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include "stdio.h"
#include <stm32f10x_cl.h>
#include "GLCD.h"
#include "TS.h"

#define LED_NUM     8                   /* Number of user LEDs                */
const long led_mask[] = { 1<<15, 1<<14, 1<<13, 1<<12, 1<<11, 1<<10, 1<<9, 1<<8 };

extern unsigned char ClockLEDOn;
extern unsigned char ClockLEDOff;
extern unsigned char ClockANI;
extern unsigned char Bg_16bpp_t[];
extern unsigned char Bg_16bpp_l[];
extern unsigned char Bg_16bpp_r[];
extern unsigned char Bg_16bpp_b[];
extern unsigned char ARM_Ani_16bpp[];
extern unsigned char Arrows_16bpp_red[];
extern unsigned char Button_16bpp[];
extern unsigned char Bulb_16bpp[];

int main (void) {
  int ad_val   =  0, ad_avg   = 0, ad_val_  = 0xFFFF;
  int joy      =  0, joy_     = -1;
  int but      =  0, but_     = -1;
  int tch_x    =  0, tch_y    =  0, tch_z   = 0;
  int tch_int  =  0, tch_on   =  0, tch_on_ = 0;
  int tch_smpl =  0;
  int light_on =  0;
  int state    =  0;
  int num      = -1; 
  int dir      =  1;
  int pic      =  0;
  int i        =  0;

  SystemInit();
  SysTick_Config(SystemFrequency/100-1);/* Generate interrupt every 10 ms     */

  /* Configure the GPIO for Push Buttons                                      */
  RCC->APB2ENR |=  1 <<  2;             /* Enable GPIOA clock                 */
  RCC->APB2ENR |=  1 <<  3;             /* Enable GPIOB clock                 */
  RCC->APB2ENR |=  1 <<  4;             /* Enable GPIOC clock                 */
  GPIOA->CRL   &= 0xFFFFFFF0;
  GPIOA->CRL   |= 0x00000004;
  GPIOB->CRL   &= 0x0FFFFFFF;
  GPIOB->CRL   |= 0x40000000;
  GPIOC->CRH   &= 0xFF0FFFFF;
  GPIOC->CRH   |= 0x00400000;

  /* Configure the GPIO for Joystick                                          */
  RCC->APB2ENR |=  1 <<  5;             /* Enable GPIOD clock                 */
  GPIOD->CRH   &= 0x00000FFF;
  GPIOD->CRH   |= 0x44444000;

  /* Setup GPIO for LEDs                                                      */
  RCC->APB2ENR |=  1 <<  6;             /* Enable GPIOE clock                 */
  GPIOE->CRH    = 0x33333333;           /* Configure the GPIO for LEDs        */

  /* Setup and initialize ADC converter                                       */
  RCC->APB2ENR |=  1 <<  9;             /* Enable ADC1 clock                  */
  GPIOC->CRL   &= 0xFFF0FFFF;           /* Configure PC4 as ADC.14 input      */
  ADC1->SQR1    = 0x00000000;           /* Regular channel 1 conversion       */
  ADC1->SQR2    = 0x00000000;           /* Clear register                     */
  ADC1->SQR3    = 14 <<  0;             /* SQ1 = channel 14                   */
  ADC1->SMPR1   =  5 << 12;             /* Channel 14 sample time is 55.5 cyc */
  ADC1->SMPR2   = 0x00000000;           /* Clear register                     */
  ADC1->CR1     =  1 <<  8;             /* Scan mode on                       */
  ADC1->CR2     = (1 << 20) |           /* Enable external trigger            */
                  (7 << 17) |           /* EXTSEL = SWSTART                   */
                  (1 <<  1) |           /* Continuous conversion              */
                  (1 <<  0) ;           /* ADC enable                         */
  ADC1->CR2    |=  1 <<  3;             /* Initialize calibration registers   */
  while (ADC1->CR2 & (1 << 3));         /* Wait for initialization to finish  */
  ADC1->CR2    |=  1 <<  2;             /* Start calibration                  */
  while (ADC1->CR2 & (1 << 2));         /* Wait for calibration to finish     */
  ADC1->CR2    |=  1 << 22;             /* Start first conversion             */ 

  GLCD_Init();                          /* Initialize graphical LCD display   */
  GLCD_Clear(White);                    /* Clear graphical LCD display        */

  for (;;) {                            /* Loop forever                       */
    /* Collect all input signals                                              */
    /* AD converter input                                                     */
    if (ADC1->SR & (1 << 1)) {          /* If conversion has finished         */
      ad_avg += (ADC1->DR & 0x0FFF)<<8; /* Add AD value to averaging          */
      ad_avg ++;
      ADC1->CR2 |= 1 << 22;             /* Start new conversion               */ 
      if ((ad_avg & 0xFF) == 0x10) {
        ad_val = ad_avg >> 12;
        ad_avg = 0;
      }
    }

    /* Joystick input                                                         */
    joy = 0;
    if (GPIOD->IDR & (1 << 15)) joy |= (1 << 0);  /* Joystick left            */
    if (GPIOD->IDR & (1 << 13)) joy |= (1 << 1);  /* Joystick right           */
    if (GPIOD->IDR & (1 << 12)) joy |= (1 << 2);  /* Joystick up              */
    if (GPIOD->IDR & (1 << 14)) joy |= (1 << 3);  /* Joystick down            */
    if (GPIOD->IDR & (1 << 11)) joy |= (1 << 4);  /* Joystick select          */
    joy ^= 0x1F;

    /* Button inputs                                                          */
    but = 0;
    if (GPIOB->IDR & (1 <<  7)) but |= (1 << 0);  /* Button User (S1)         */
    if (GPIOC->IDR & (1 << 13)) but |= (1 << 1);  /* Button Tamper (S2)       */
    if (GPIOA->IDR & (1 <<  0)) but |= (1 << 2);  /* Button Wakeup (S3)       */
    but ^= 0x03;

    /* Show all signals                                                       */
    switch (state) {
      case 0:                           /* Background for Animating logo      */
        GLCD_Bmp (  0,   0, 320,  69, Bg_16bpp_t+70);
        GLCD_Bmp (  0,  69,   4, 102, Bg_16bpp_l+70);
        GLCD_Bmp (316,  69,   4, 102, Bg_16bpp_r+70);
        GLCD_Bmp (  0, 171, 320,  69, Bg_16bpp_b+70);
        state++;
        break;

      case 1:                           /* Display rotating ARM logo          */
        if (ClockANI && (pic < 40)) {
          ClockANI = 0;
          if (pic++ >= 39) state++;
          GLCD_Bmp (99, 99, 120, 45, &ARM_Ani_16bpp[(pic&7)*(120*45*2)]);
        }
        break;

      case 2:                           /* Display static text                */
        GLCD_Clear(White);
        GLCD_SetTextColor(Maroon);
        GLCD_SetBackColor(Blue);
        GLCD_SetTextColor(White);
        GLCD_DisplayString(0, 0, 1, "   MCBSTM32C Demo    ");
        GLCD_SetTextColor(Blue);
        GLCD_SetBackColor(White);
        GLCD_DisplayString(2, 0, 1, "AD value:            ");
        GLCD_DisplayString(3, 0, 1, "Buttons :            ");
        GLCD_DisplayString(5, 0, 1, "Joystick:            ");
        GLCD_Bmp(  9, 176, 120, 48, Button_16bpp);
        GLCD_Bmp(209, 178,  26, 40, Bulb_16bpp);
                                        /* Setup Touch-Screen controller      */
        TS_Init  ();                    /* Initialize Touch-Screen controller */
        TS_Write (0x03, 1, 0x02);       /* Reset Touch-screen controller      */
        for (i = 0; i < 180000; i++);   /* Wait minimum of 10ms               */
        TS_Write (0x04, 1, 0x0C);       /* Enable TSC and ADC                 */
        TS_Write (0x0A, 1, 0x07);       /* Enable Touch detect, FIFO          */
        TS_Write (0x20, 1, 0x69);       /* Set sample time , 12-bit mode      */
        for (i = 0; i < 36000; i++);    /* Wait minimum of 2ms                */
        TS_Write (0x21, 1, 0x01);       /* ADC frequency 3.25 MHz             */
        TS_Write (0x41, 1, 0xF5);       /* Set TSC_CFG register               */
        TS_Write (0x4A, 1, 0x01);       /* Threshold for FIFO                 */
        TS_Write (0x4B, 1, 0x01);       /* FIFO reset                         */
        TS_Write (0x4B, 1, 0x00);       /* FIFO not reset                     */
        TS_Write (0x56, 1, 0x07);       /* Fraction z                         */
        TS_Write (0x58, 1, 0x01);       /* Drive 50 mA typical                */
        TS_Write (0x40, 1, 0x01);       /* Enable TSC                         */
        TS_Write (0x0B, 1, 0xFF);       /* Clear interrupt status             */
        TS_Write (0x09, 1, 0x01);       /* Enable global interrupt            */
        state++;
        break;

      case 3:                           /* Main demo                          */
        tch_int = TS_Read (0x0B, 1);    /* Read Touch-screen interrupt status */
        if (tch_int) {                  /* If Touch-screen interrupt active   */
          if (tch_int & 1) {            /* If touch detected                  */
            tch_on ^= 1;
            if (tch_on) {
              tch_x = 0;
              tch_y = 0;
              tch_z = 0;
            }
          }
          if (tch_int & 2) {            /* If FIFO is above threshold         */
            tch_on   = 1;
            tch_smpl = TS_Read (0x4C, 1);
            while (--tch_smpl) {
              TS_Read (0xD7, 4);
            }
            tch_z = TS_Read (0xD7, 4);  /* Read coordinates                   */
            tch_x = (tch_z >> 20) & 0x00000FFF;
            tch_y = (tch_z >>  8) & 0x00000FFF;
            tch_z = (tch_z >>  0) & 0x0000000F;
          }
          TS_Write (0x0B, 1, tch_int);
        }
        if (ad_val ^ ad_val_) {         /* Show AD value bargraph             */
          GLCD_SetTextColor(Cyan);
          GLCD_Bargraph (144, Line2, 160, 20, (ad_val>>2));
          ad_val_ = ad_val;
        }
        if (but ^ but_) {               /* Show buttons status                */
          GLCD_SetTextColor(Black);
          if (  but & (1 << 0) ) GLCD_DisplayString(3,  9, 1, "S1  ");
          if (  but & (1 << 1) ) GLCD_DisplayString(3, 13, 1, "S2  ");
          if (  but & (1 << 2) ) GLCD_DisplayString(3, 17, 1, "S3");
          GLCD_SetTextColor(LightGrey);
          if (!(but & (1 << 0))) GLCD_DisplayString(3,  9, 1, "S1  ");
          if (!(but & (1 << 1))) GLCD_DisplayString(3, 13, 1, "S2  ");
          if (!(but & (1 << 2))) GLCD_DisplayString(3, 17, 1, "S3");
          but_ = but;
        }
        if (joy ^ joy_) {               /* Show joystick status               */
          GLCD_Bmp(196+ 0, 103+20, 19, 14, &Arrows_16bpp_red[(((joy>>0)&1)*2416)]);
          GLCD_Bmp(196+35, 103+20, 19, 14, &Arrows_16bpp_red[(((joy>>1)&1)*2416)+1*532]);
          GLCD_Bmp(196+20, 103+ 0, 14, 19, &Arrows_16bpp_red[(((joy>>2)&1)*2416)+2*532]);
          GLCD_Bmp(196+20, 103+35, 14, 19, &Arrows_16bpp_red[(((joy>>3)&1)*2416)+3*532]);
          GLCD_Bmp(196+21, 103+21, 12, 12, &Arrows_16bpp_red[(((joy>>4)&1)*2416)+4*532]);
          joy_ = joy;
        }
        if ((tch_on ^ tch_on_) &&       /* Show touch button status           */ 
            (tch_x)            && 
            (tch_y)             ) {
          if ((tch_x >= 200) && (tch_x <= 1300) &&
              (tch_y >= 100) && (tch_y <= 1600)  ) {
            GLCD_Bmp(  9, 176, 120, 48, &Button_16bpp[tch_on*11520]);
            if (tch_on) {
              light_on ^= 1;
              GLCD_Bmp(209, 178, 26, 40, &Bulb_16bpp[light_on*2080]);
            }
          }
          if (!tch_on) {
            GLCD_Bmp(  9, 176, 120, 48, &Button_16bpp[tch_on*11520]);
          }
          tch_on_ = tch_on;
        }
        break;
    }

    if (ClockLEDOn) {
      ClockLEDOn = 0;
      /* Calculate 'num': 0, 1, ... , LED_NUM-1, LED_NUM-1, ... , 1, 0, 0, ...*/
      num += dir;
      if (num >= LED_NUM) { dir = -1; num = LED_NUM-1; } 
      else if   (num < 0) { dir =  1; num = 0;         }
      GPIOE->BSRR = led_mask[num];      /* Turn LED on                        */
    }
    if (ClockLEDOff) {
      ClockLEDOff = 0;
      GPIOE->BSRR = led_mask[num] << 16;/* Turn LED off                       */
    }
  }
}
