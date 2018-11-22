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

#define CAN_LOOPBACK

#define TMR_CLK      80000000           /* In Hz                              */

volatile unsigned char  Clock1s;
volatile          short AD_val;

volatile unsigned char  Rx_Data;
volatile unsigned char  Tx_Data;


/* Import external functions from Serial.c file                               */
extern void init_serial (void);


int main (void) {
  int   LED_val, LED_old = -1;
  int   i;
  char  buf[20];

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
                     (15 <<  0) ;       /* Priority level 15, disable nesting */

  /* Setup Timer 0 to generate interrupt every 10 ms                          */
  TIM0_PR   = (TMR_CLK / 1000) - 1;     /* Prescaler to drive timer with 1 ms */
  TIM0_MR0  = (10 - 1);                 /* Match value = 10 * 1ms = 10 ms     */
  TIM0_MCR  = 1;                        /* On Match 0 event reset timer       */
  TIM0_INT_SET_ENABLE = 1;              /* Enable Match 0 to generate interupt*/
  TIM0_TCR  = 1;                        /* Start Timer 0                      */

  init_serial();                        /* Init UART                          */

  /* Configure CAN/LIN clock = PLL clock / 2 = 80 MHz                         */
  IVNSS_CLK_CONF = (2UL << 24) | (1 << 11) | (1 << 2);

  /* Configure CAN pins */
  SFSP0_0 = 0x06;                       /* Configure P0.0  as CAN0 TxD        */
  SFSP0_1 = 0x06;                       /* Configure P0.1  as CAN0 RxD        */

  /* Configure CAN0 */
  CAN0MOD = 1;                          /* Start in Reset Mode                */
  CAN0BTR = ((7-1)<<16) |               /* TSEG1=7                            */
            ((6-1)<<20) |               /* TSEG2=6                            */
            ((1-1)<<14) |               /* SJW=1                              */
            ((80000000/100000/(7+6+1))-1);   /* 100kbps & 80MHz CAN Clock     */
#ifdef CAN_LOOPBACK
  CAN0MOD = 4;                          /* Self-Test Mode                     */
#else
  CAN0MOD = 0;                          /* Normal Mode                        */
#endif
  
  /* Setup CAN0 Rx interrupt generation                                       */
  INT_REQUEST_37 = (1 << 28) |          /* Enable setting of priority level   */
                   (1 << 27) |          /* Enable setting interrupt target    */
                   (1 << 26) |          /* Write enable of new settings       */
                   (1 << 16) |          /* Enable interrupt request           */
                   (1 <<  8) |          /* Interrupt target is IRQ interrupt  */
                   (1 <<  0) ;          /* Set priority level to 1            */

  /* Enable CAN0 Rx interrupt */
  CAN0IER = 1;

  /* Setup CAN Acceptance Filter */
  CAN_AFMR = 2;                         /* Acceptance Filter Bypass           */

  LCD_init();
  LCD_cls();
  LCD_print (0, 0, "MCB2900 CAN DEMO");
  LCD_print (0, 1, "  www.keil.com  ");

  for (i = 0; i < 12000000; i++);       /* Wait for initial display           */

  for (;;) {                            /* Loop forever                       */

    if (Clock1s) {
      Clock1s  = 0;
      LED_val ^= 1 << 5;

      CAN0TFI1 = 1 << 16;               /* FF=0, RTR=0, DLC=1, TXPRIO=1       */
      CAN0TID1 = 0x123;                 /* ID=0x123                           */
      CAN0TDA1 = Tx_Data;               /* Tx Data Byte                       */
#ifdef CAN_LOOPBACK
      CAN0CMR    = 0x30;                /* Transmit (Buffer 1) with Loopback  */
#else
      CAN0CMR    = 0x21;                /* Transmit (Buffer 1)                */
#endif

      printf ("AD value = 0x%04x\n\r", AD_val);
      sprintf(buf, "Tx=0x%02X Rx=0x%02X", Tx_Data++, Rx_Data);
      LCD_print (0, 1, (unsigned char *)buf);
    }

    LED_val &= ~0x1F;
    LED_val |=  0x1F & ((1 << (AD_val / 200)) - 1);

    if (LED_val != LED_old) {
      LED_old = LED_val;
      GPIO1_OR &= ~(0x3F            << 24);
      GPIO1_OR |=  (0x3F & LED_val) << 24;
    }
  }
}
