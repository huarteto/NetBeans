/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <75x_lib.H>                    /* STR750 definitions                 */
#include "LCD.h"                        /* Graphic LCD function prototypes    */


/* I/O mapping of the LEDs                                                    */
struct {
  GPIO_TypeDef* gpio;
  u32           bit;
} LED_Map[8] = {
  { GPIO2,  0 },
  { GPIO2,  1 },
  { GPIO2,  2 },
  { GPIO2,  3 },
  { GPIO2,  4 },
  { GPIO2,  5 },
  { GPIO2,  6 },
  { GPIO2,  7 },
};

/* Function that initializes LEDs                                             */
void LED_Init(void) {
  u32 i;
  u32 uMask;

  // I/O Ports configured as Output (Push-pull)
  for (i = 0; i < 8; i++) {
    uMask = 1 << LED_Map[i].bit;
    LED_Map[i].gpio->PC0 |= uMask;
    LED_Map[i].gpio->PC1 &=~uMask;
    LED_Map[i].gpio->PC2 |= uMask;
    LED_Map[i].gpio->PM  &=~uMask;
    LED_Map[i].gpio->PD  |= uMask;
  }
}

/* Function that turns on requested LED                                       */
void LED_On(u32 num) {
  LED_Map[num].gpio->PD |=  (1 << LED_Map[num].bit);
}

/* Function that turns off requested LED                                      */
void LED_Off(u32 num) {
  LED_Map[num].gpio->PD &= ~(1 << LED_Map[num].bit);
}


/* Function for displaying bargraph on the LCD display                        */
void Disp_Bargraph(int pos_x, int pos_y, int value) {
  int i;

  set_cursor (pos_x, pos_y);
  for (i = 0; i < 16; i++)  {
    if (value > 5)  {
      lcd_putchar (0x05);
      value -= 5;
    }  else  {
      lcd_putchar (value);
      value = 0;
    }
  }
}

/* Import external IRQ handlers from IRQ.c file                               */
extern __irq void TIM2_UP_IRQHandler(void);
extern __irq void ADC_IRQHandler (void);

/* Import external variables from IRQ.c file                                  */
extern short AD_last;


int main (void) {
  int i;

  short AD_old, AD_value;

  LED_Init();                                       /* LED Initialization     */

  GPIO0->PC0 &= ~GPIO_Pin_2;                        /* AD (P0.2) Analog Input */
  GPIO0->PC1 &= ~GPIO_Pin_2;
  GPIO0->PC2 &= ~GPIO_Pin_2;

  /* A/D Converter Configuration: Read Potenciometer Position                 */
  ADC->CLR1   = 0x8000 | (5<<5) | 1;                /* Setup prescalers:      */
                                                    /* Sample Presc enabled,  */
                                                    /* conversion presc = 5,  */
                                                    /* sample presc = 1       */
  ADC->CLR2   = 0x8000 | (0<<6) | 0;                /* Control Logic Setup:   */
                                                    /* Scan Mode, 1 channels  */
                                                    /* first channel 0        */
  ADC->IMR   |= ADC_IT_EOC;                         /* Enable AD EOC interrupt*/
  ADC->CLR4  &= 0x7FFF & 0xFFDF;                    /* Enable ADC + Enable    */
  ADC->CLR0  |= 0x0001;                             /* calibration + Start ADC*/

  /* Configure and enable IRQ for A/D Converter                               */
  EIC->SIRn[ADC_IRQChannel] = ((u32)ADC_IRQHandler << 16);
  EIC->SIRn[ADC_IRQChannel] |= (2 & 0x0F);          /* Setup priority 2       */
  EIC->IER   |= (1 << ADC_IRQChannel);              /* Enable ADC interrupt   */

  /* Timer 2 Configuration: Periodic Interrupts at aprox. 100ms
     Default: Count-up counter
              CK_TIM used as clock                                            */
  TIM2->PSC   =  3199;                              /* 32MHz/(3199+1) = 10kHz */
  TIM2->ARR   =  100;                               /* 100/10kHz = 100 ms     */
  TIM2->RSR  |=  TIM_IT_Update;                     /* Enable TIM2 update int */
  TIM2->RER  |=  TIM_IT_Update;
  TIM2->CR   |=  0x0004;                            /* Start TIM2             */

  /* Configure and enable IRQ for Timer 2                                     */
  EIC->IVR  = (u32)TIM2_UP_IRQHandler;
  EIC->SIRn[TIM2_UP_IRQChannel] = ((u32)TIM2_UP_IRQHandler << 16);
  EIC->SIRn[TIM2_UP_IRQChannel] |= (1 & 0x0F);      /* Setup priority 1       */
  EIC->IER   |= (1 << TIM2_UP_IRQChannel);          /* Enable TIM2 UP interupt*/

  EIC->ICR   |= 0x00000001;                         /* Global IRQ enable      */

  lcd_init();
  lcd_clear();
  lcd_print (" MCBSTR750 DEMO ");
  set_cursor (0, 1);
  lcd_print ("  www.keil.com  ");

  for (i = 0; i < 10000000; i++);       /* Wait for initial display           */

  while (1) {                           /* Loop forever                       */
    AD_value = AD_last;                 /* Read AD_last value                 */
    if (AD_value != AD_last)            /* Make sure that AD interrupt did    */
      AD_value = AD_last;               /* not interfere with value reading   */
    AD_value /= 13;                     /* Scale to AD_Value to 0 - 78        */
    if (AD_old != AD_value)  {          /* If AD value has changed            */
      AD_old = AD_value;
      Disp_Bargraph(0, 1, AD_value);    /* Display bargraph according to AD   */
    }
  }
}
