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
#include "GLCD.h"                       /* Graphic LCD function prototypes    */


/* I/O mapping of the LEDs                                                    */
struct {
  GPIO_TypeDef* gpio;
  u32           bit;
} LED_Map[4] = {
  { GPIO1,  1 },
  { GPIO0, 16 },
  { GPIO2, 18 },
  { GPIO2, 19 },
};

void LED_Init(void) {
  u32 i;
  u32 uMask;

  // I/O Ports configured as Output (Push-pull)
  for (i = 0; i < 4; i++) {
    uMask = 1 << LED_Map[i].bit;
    LED_Map[i].gpio->PC0 |= uMask;
    LED_Map[i].gpio->PC1 &=~uMask;
    LED_Map[i].gpio->PC2 |= uMask;
    LED_Map[i].gpio->PM  &=~uMask;
    LED_Map[i].gpio->PD  |= uMask;
  }
}

void LED_On(u32 num) {
  LED_Map[num].gpio->PD &= ~(1 << LED_Map[num].bit);
}

void LED_Off(u32 num) {
  LED_Map[num].gpio->PD |=  (1 << LED_Map[num].bit);
}


/* Function for displaying bargraph                                           */
void Disp_Bargraph(int pos_x, int pos_y, int value) {
  int i;
  char print_buf[32];

  for (i = 0; i < 16; i++)  {
    if (value > 6)  {
      print_buf[i] = 0x06;
      value -= 6;
    }  else  {
      if (value == 0)
        print_buf[i] = ' ';
      else 
        print_buf[i] = value;
      value = 0;
    }
  }
  i++;
  print_buf[i] = 0;
  glcd_print(pos_x, pos_y, 0, (unsigned char *) &print_buf);
}


extern short AD_last;


int main (void) {
  short AD_old, AD_value;

  LED_Init();                                       /* LED Initialization     */

  GPIO0->PC0 &= ~GPIO_Pin_17;                       /* AD (P0.17) Analog Input*/
  GPIO0->PC1 &= ~GPIO_Pin_17;
  GPIO0->PC2 &= ~GPIO_Pin_17;

  /* A/D Converter Configuration: Read Potenciometer Position                 */
  ADC->CLR1   = 0x8000 | (5<<5) | 1;                /* Setup prescalers:      */
                                                    /* Sample Presc enabled,  */
                                                    /* conversion presc = 5,  */
                                                    /* sample presc = 1       */
  ADC->CLR2   = 0x8000 | (0<<6) | 3;                /* Control Logic Setup:   */
                                                    /* Scan Mode, 1 channels  */
                                                    /* first channel 3        */
  ADC->IMR   |= ADC_IT_EOC;                         /* Enable AD EOC interrupt*/
  ADC->CLR4  &= 0x7FFF & 0xFFDF;                    /* Enable ADC + Enable    */
  ADC->CLR0  |= 0x0001;                             /* calibration + Start ADC*/

  /* Configure and enable IRQ for A/D Converter                               */
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
  EIC->SIRn[TIM2_UP_IRQChannel] |= (1 & 0x0F);      /* Setup priority 1       */
  EIC->IER   |= (1 << TIM2_UP_IRQChannel);          /* Enable TIM2 UP interupt*/

  EIC->ICR   |= 0x00000001;                         /* Global IRQ enable      */

  glcd_init();
  glcd_clear();

  glcd_print(0, 0, 0, "This is a Keil DEMO ");
  glcd_print(0, 1, 0, "for the STR75x Board");
  glcd_print(0, 2, 0, "   www.keil.com     ");
  glcd_print(0, 3, 0, "AD:                 ");

  while (1) {                           /* Loop forever                       */
    AD_value = AD_last;                 /* Read AD_last value                 */
    if (AD_value != AD_last)            /* Make sure that AD interrupt did    */
      AD_value = AD_last;               /* not interfere with value reading   */
    AD_value /= 10;                     /* Scale to AD_Value to 0 - 102       */
    if (AD_old != AD_value)  {          /* If AD value has changed            */
      AD_old = AD_value;
      Disp_Bargraph(3, 3, AD_value);    /* Display bargraph according to AD   */
    }
  }
}
