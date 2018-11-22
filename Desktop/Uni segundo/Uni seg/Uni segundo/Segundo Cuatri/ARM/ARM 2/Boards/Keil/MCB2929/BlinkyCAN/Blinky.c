/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2010 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <stdio.h>
#include <LPC29xx.H>
#include "GLCD.h"                        /* LCD function prototypes            */

#define CAN_LOOPBACK
#define __ADC0                           /* If __ADC0 is not defined ADC2 will be selected as default! */

#define AD_VALUE_MAX 0x3FF               /*10 bit resolution*/

#define TMR_CLK      96000000            /* In Hz                              */

#define PB_S1        0x04000000          /* P2.26 */
#define PB_S2        0x08000000          /* P2.27 */

volatile unsigned char  Clock1s;
volatile          short AD0_val, AD2_val;

volatile unsigned char  Rx_Data;
volatile unsigned char  Tx_Data;

                  char  text[40];

/* Import external functions from Serial.c file                               */
extern void init_serial (void);


/*----------------------------------------------------------------------------
  Additional LCD defines and functions used to draw a bargraph
 *----------------------------------------------------------------------------*/
#define FONT_WIDTH      16
#define FONT_HEIGHT    	24
#define DISPLAY_WIDTH 1024



int main (void) {
  int   LED_val, ButVal, LED_old = -1;
  int   i;
  char  buf[20];
  int barLen;
  int AD_valueOld = 0;
#ifdef __ADC0
  #define AD_value AD0_val
#else
  #define AD_value AD2_val
#endif

  /* Configure the BUTTON's GPIO pins as Inputs with internal pull-up         */
  SFSP2_26  =  (0x03   << 2);
  SFSP2_27  =  (0x03   << 2);
  GPIO2_DR &= ~(0x03UL << 26);

  /* Configure the LED's GPIO pins as Outputs                                 */
  GPIO2_DR |=  (0xFF << 16);


  /* Configure MSCSS clock = PLL clock / 2 = 96 MHz (max allowed 125 MHz)      */
  MSCSS_CLK_CONF   = (2UL << 24) | (1 << 11) | (1 << 2);

  MTIM0_PR   = (96000000 / 1000) - 1;   /* Prescaler to drive timer with 1 ms */
  MTIM0_MR0  = (10 - 1);                /* Match value = 10 * 1ms = 10 ms     -> ADC0*/
  MTIM0_MR2  = (10 - 1);                /* Match value = 10 * 1ms = 10 ms     -> ADC2*/
  MTIM0_MCR  = (1 << 4);                /* On Match 2 event reset timer       */
  MTIM0_EMR |= (3 << 4);                /* Match 0 event generate logic 1     -> ADC0*/
  MTIM0_EMR |= (3 << 8);                /* Match 2 event generate logic 1     -> ADC2*/
  MTIM0_TCR  = 1;                       /* Start Timer 0                             */



  /* Configure ADC clock = FDIV0 / 1 = 4 MHz (max allowed 4.5 MHz)            */
  ADC_CLK_CONF     = (5UL << 24) | (1 << 11) | (0 << 2);

  /* Setup ADC2 IN0 to convert in 10-bit resolution and generate interrupt    */
  ADC0_CTRL  = (1<<1);                  /* Stop  ADC conversion               */

  SFSP0_4         = 5;                  /* Configure P0.4 as analog input    */

  /* Setup ADC0 interrupt generation                                          */
  INT_REQUEST_16   = (1  << 28) |       /* Enable setting of priority level   */
                     (1  << 27) |       /* Enable setting interrupt target    */
                     (1  << 26) |       /* Write enable of new settings       */
                     (1  << 16) |       /* Enable interrupt request           */
                     (1  <<  8) |       /* Interrupt target is IRQ interrupt  */
                     (15 <<  0) ;       /* Priority level 15, disable nesting */

  /* Setup ADC0 IN0 to convert in 10-bit resolution and generate interrupt    */
  ADC0_ACC0        = 0x0000000A;        /* Set ADC0 IN0 10-bit resolution     */
  ADC0_CONFIG      = (1 << 14);         /* ADC single scan, start on pos edge */
  ADC0_CTRL        = 4;                 /* Copy the configuration             */
  ADC0_INT_SET_ENABLE = 1;              /* Enable end of scan interrupt       */


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

  /* Configure timer clock = PLL clock / 2 = 96 MHz                           */
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

  init_serial();                        /* Init UART                          */

  /* Configure CAN/LIN clock = PLL clock / 2 = 96 MHz                         */
  IVNSS_CLK_CONF = (2UL << 24) | (1 << 11) | (1 << 2);

  /* Configure CAN pins */
  SFSP0_0 = 0x06;                       /* Configure P0.0  as CAN0 TxD        */
  SFSP0_1 = 0x06;                       /* Configure P0.1  as CAN0 RxD        */

  /* Configure CAN0 */
  CAN0MOD = 1;                          /* Start in Reset Mode                */
  CAN0BTR = ((7-1)<<16) |               /* TSEG1=7                            */
            ((6-1)<<20) |               /* TSEG2=6                            */
            ((1-1)<<14) |               /* SJW=1                              */
            ((96000000/100000/(7+6+1))-1);   /* 100kbps & 96MHz CAN Clock     */
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

  GLCD_Init ();                         /* GLCD initialization */ 
  GLCD_Clear(White);                    /* Clear the GLCD */

  GLCD_SetBackColor(Blue);              /* Set the Back Color */
  GLCD_SetTextColor(White);             /* Set the Text Color */

  GLCD_DisplayString(0, 0, "  MCB2929     Demo    ");
  GLCD_DisplayString(1, 0, "    Blinky & CAN      ");
  GLCD_DisplayString(2, 0, "    www.keil.com      ");

  GLCD_SetBackColor(White);             /* Set the Back Color */
  GLCD_SetTextColor(Blue);              /* Set the Text Color */
  sprintf(text, "AD value = 0x%04X", AD_value);
  GLCD_DisplayString(4, 0, (unsigned char *)text);

  GLCD_DisplayString(7, 0, " CAN Comm. Status    ");

  for (i = 0; i < 12000000; i++);       /* Wait for initial display           */

  for (;;) {                            /* Loop forever                       */

    if (AD_value != AD_valueOld)
    {
      GLCD_SetTextColor(Blue);          /* Set the Text Color */
      sprintf(text, "AD value = 0x%04X", AD_value);
      GLCD_DisplayString(4, 0, (unsigned char *)text);

      barLen = (DISPLAY_WIDTH*AD_value)/AD_VALUE_MAX;

      GLCD_SetTextColor(Red);
      GLCD_Bargraph (0, 120, 320, 24, barLen);

      AD_valueOld = AD_value;
    }
    if (Clock1s) 
    {
      Clock1s  = 0;
      LED_val ^= 1 << 5;

      CAN0TFI1 = 1 << 16;                /* FF=0, RTR=0, DLC=1, TXPRIO=1       */
      CAN0TID1 = 0x123;                  /* ID=0x123                           */
      CAN0TDA1 = Tx_Data;                /* Tx Data Byte                       */
#ifdef CAN_LOOPBACK
      CAN0CMR    = 0x30;                 /* Transmit (Buffer 1) with Loopback  */
#else
      CAN0CMR    = 0x21;                 /* Transmit (Buffer 1)                */
#endif
      GLCD_SetTextColor(Blue);           /* Set the Text Color */
      sprintf(buf, "  Tx=0x%02X Rx=0x%02X    ", Tx_Data++, Rx_Data);
      GLCD_DisplayString(8, 0, (unsigned char *)buf);

    }

    
    ButVal = ~(GPIO2_PINS & (PB_S1 | PB_S2)) >> 26;  /*  LED6 -- LED7 (Button S1 & S2 Value) */
    LED_val &= ~0xC0;
    LED_val |=  ButVal << 6; 
    
    LED_val &= ~0x1F;                                /* LED0 -- LED4(ADC Value) */
    LED_val |=  0x1F & ((1 << (AD_value / 200)) - 1);

    if (LED_val != LED_old) {
      LED_old = LED_val;
      GPIO2_OR &= ~(0xFF            << 16);
      GPIO2_OR |=  (0xFF & LED_val) << 16;
    }
  }
}
