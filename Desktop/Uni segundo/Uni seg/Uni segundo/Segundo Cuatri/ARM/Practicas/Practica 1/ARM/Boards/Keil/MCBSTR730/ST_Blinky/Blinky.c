/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <73x_lib.h>                    /* STR730 library functions           */
#include <stdio.h>                      /* Standard input/output functions    */

/* Global variable declared in 73x_it.c module                                */
extern unsigned short AD_value;         /* AD value from interrupt handler    */

                                        /* LED segments for numbers 0 .. 9    */
const unsigned char LED7SEG[] = { 0xC0, 0xF9, 0xA4, 0xB0, 
                                  0x99, 0x92, 0x82, 0xF8,
                                  0x80, 0x90, 0x88, 0x83,
                                  0xC6, 0xA1, 0x86, 0x8E
                                } ;

GPIO_InitTypeDef  GPIO_InitStruct;      /* Declare GPIO  structure            */
UART_InitTypeDef  UART_InitStruct;      /* Declare UART  structure            */
TIM_InitTypeDef   TIM_InitStructure;    /* Declare TIM   structure            */
CMU_InitTypeDef   CMU_InitStruct;       /* Declare CMU   structure            */
ADC_InitTypeDef   ADC_InitStruct;       /* Declare ADC   structure            */
PRCCU_InitTypeDef PRCCU_InitStruct;     /* Declare PRCCU structure            */


int main (void) {
  int i;

  /* Setup GPIO6 - UART0 Tx pin setup (P6.9)                                  */
  CFG_PeripheralClockConfig(CFG_CLK_GPIO6 , ENABLE);
  GPIO_InitStruct.GPIO_Mode             = GPIO_Mode_AF_PP;
  GPIO_InitStruct.GPIO_Pins             = GPIO_PIN_9;
  GPIO_Init(GPIO6, &GPIO_InitStruct);

  /* Setup GPIO3 - AD converter pin setup (P3.0)                              */
  CFG_PeripheralClockConfig(CFG_CLK_GPIO3, ENABLE);
  GPIO_InitStruct.GPIO_Mode             = GPIO_Mode_HI_AIN_TRI;
  GPIO_InitStruct.GPIO_Pins             = GPIO_PIN_0;
  GPIO_Init(GPIO3, &GPIO_InitStruct);

  /* Setup GPIO2 - Buzzer pin (P2.3)                                          */
  CFG_PeripheralClockConfig(CFG_CLK_GPIO2, ENABLE);
  GPIO_InitStruct.GPIO_Mode             = GPIO_Mode_OUT_PP;
  GPIO_InitStruct.GPIO_Pins             = GPIO_PIN_3;
  GPIO_Init(GPIO2, &GPIO_InitStruct);

  /* Setup GPIO1 - Key S2 (P1.8)                                              */
  CFG_PeripheralClockConfig(CFG_CLK_GPIO1, ENABLE);
  GPIO_InitStruct.GPIO_Mode             = GPIO_Mode_IN_TRI_TTL;
  GPIO_InitStruct.GPIO_Pins             = GPIO_PIN_8;
  GPIO_Init(GPIO1, &GPIO_InitStruct);

  /* Setup GPIO0 - 7 segment LED displays (P0.0 .. P0.9)                      */
  CFG_PeripheralClockConfig(CFG_CLK_GPIO0, ENABLE);
  GPIO_InitStruct.GPIO_Mode             = GPIO_Mode_OUT_PP;
  GPIO_InitStruct.GPIO_Pins             = GPIO_PIN_ALL;
  GPIO_Init(GPIO0, &GPIO_InitStruct);

  /* Enable Enhanced Interrupt Controller (EIC)                               */
  CFG_PeripheralClockConfig(CFG_CLK_EIC, ENABLE);

  /* Enable clocks for Timer 3 and Timer 4                                    */
  CFG_PeripheralClockConfig(CFG_CLK_TIM3 , ENABLE);
  CFG_PeripheralClockConfig(CFG_CLK_TIM4 , ENABLE);

  /* Clock Monitor Unit (CMU) Setup                                           */
  /* Setup CMU to use oscillator                                              */
  CMU_DeInit();
  CMU_StructInit(&CMU_InitStruct);
  CMU_InitStruct.CMU_CKSEL0             = CMU_CKSEL0_CKOSC;
  CMU_Init(&CMU_InitStruct);

  /* Power, Reset and Clock Control Unit (PRCCU) Setup                        */
  /* Setup clock to frequency of oscillator                                   */
  PRCCU_DeInit();
  PRCCU_StructInit(&PRCCU_InitStruct);  
  PRCCU_InitStruct.PRCCU_DIV2           = DISABLE;
  PRCCU_Init(&PRCCU_InitStruct);

  /* Setup UART0 configuration                                                */
  /* UART0 = 9600 bps, 8 data bits, no parity, 1 stop bit; Loopback off       */
  CFG_PeripheralClockConfig(CFG_CLK_UART0 , ENABLE);
  UART_StructInit(&UART_InitStruct);
  UART_InitStruct.UART_BaudRate         = 9600;
  UART_InitStruct.UART_Mode             = UART_Mode_8D;
  UART_InitStruct.UART_Loop_Standard    = UART_Standard;
  UART_InitStruct.UART_StopBits         = UART_StopBits_1;
  UART_Cmd(UART0, ENABLE);
  UART_Init(UART0, &UART_InitStruct);

  /* Setup TIM3 timer configuration                                           */
  /* Used for buzzer driving                                                  */
  TIM_InitStructure.TIM_Mode            = TIM_PWM;
  TIM_InitStructure.TIM_Pulse_Level_A   = TIM_High ;
  TIM_InitStructure.TIM_Clock_Source    = TIM_CLK_INTERNAL;
  TIM_InitStructure.TIM_Period_Level    = TIM_Low;
  TIM_InitStructure.TIM_Prescaler       = 0x01;
  TIM_InitStructure.TIM_Pulse_Length_A  = 0x007F;
  TIM_InitStructure.TIM_Full_Period     = 0x00FF;
  TIM_Init(TIM3, &TIM_InitStructure);
  TIM_ITConfig(TIM3,TIM_IT_OCB ,ENABLE);
  TIM_CounterCmd(TIM3, TIM_START);

  /* Setup TIM4 timer configuration                                           */
  /* Used for 7-segment LED display multiplexing                              */
  TIM_InitStructure.TIM_Mode            = TIM_PWM;
  TIM_InitStructure.TIM_Pulse_Level_A   = TIM_High ;
  TIM_InitStructure.TIM_Clock_Source    = TIM_CLK_INTERNAL;
  TIM_InitStructure.TIM_Period_Level    = TIM_Low;
  TIM_InitStructure.TIM_Prescaler       = 0xFF;
  TIM_InitStructure.TIM_Pulse_Length_A  = 0x007F;
  TIM_InitStructure.TIM_Full_Period     = 0x00FF;
  TIM_Init(TIM4, &TIM_InitStructure);
  TIM_ITConfig(TIM4,TIM_IT_OCB ,ENABLE);
  TIM_CounterCmd(TIM4, TIM_START);

  /* Setup AD converter configuration                                         */
  CFG_PeripheralClockConfig(CFG_CLK_ADC, ENABLE);
  ADC_StructInit(&ADC_InitStruct);
  ADC_InitStruct.ADC_Calibration        = ADC_Calibration_ON;
  ADC_InitStruct.ADC_CalibAverage       = ADC_CalibAverage_Enable;
  ADC_InitStruct.ADC_AutoClockOff       = ADC_AutoClockOff_Disable ;
  ADC_InitStruct.ADC_ConversionMode     = ADC_ConversionMode_Scan ;
  ADC_InitStruct.ADC_SamplingPrescaler  = 0x7;
  ADC_InitStruct.ADC_ConversionPrescaler= 0x0;
  ADC_InitStruct.ADC_FirstChannel       = ADC_CHANNEL0 ;
  ADC_InitStruct.ADC_ChannelNumber      = 1;
  ADC_Init(&ADC_InitStruct);

  ADC_ITConfig(ADC_IT_EOC, ENABLE);     /* Enable end of conversion interrupt */
  ADC_Cmd(ENABLE);                      /* Enable AD converter                */
  ADC_ConversionCmd (ADC_ConversionStart);  /* Start AD conversion            */

  /* Configure and enable IRQ for Timer 3                                     */
  EIC_IRQChannelConfig(TIM3_IRQChannel, ENABLE);
  EIC_IRQChannelPriorityConfig(TIM3_IRQChannel, 2);

  /* Configure and enable IRQ for Timer 4                                     */
  EIC_IRQChannelConfig(TIM4_IRQChannel, ENABLE);
  EIC_IRQChannelPriorityConfig(TIM4_IRQChannel, 2);

  /* Configure and enable IRQ for AD Converter                                */
  EIC_IRQChannelConfig(ADC_IRQChannel, ENABLE);
  EIC_IRQChannelPriorityConfig(ADC_IRQChannel, 1);

  EIC_IRQCmd(ENABLE);                   /* Enable interrupts                  */
  
  while (1) {                           /* Infinite loop                      */
    for (i = 0; i < 10000; i++);        /* Delay for printf                   */

    /* Change frequency for buzzer                                            */
    TIM_InitStructure.TIM_Prescaler = (AD_value >> 3) + 1;
    TIM_Init(TIM3, &TIM_InitStructure);

    /* Print analog value to UART0                                            */
    printf ("Measured analog value (10 bits) = %i \r\n", AD_value);
  }
}
