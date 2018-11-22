/******************************************************************************/
/* SETUP.C: Setup Functions                                                   */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <stm32f10x_lib.h>              /* STM32F10x Library Definitions */


void SetupClock (void)
{
  RCC_DeInit ();                        /* RCC system reset(for debug purpose)*/
  RCC_HSEConfig (RCC_HSE_ON);           /* Enable HSE */

  /* Wait till HSE is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET);

  RCC_HCLKConfig   (RCC_SYSCLK_Div1);   /* HCLK   = SYSCLK  */
  RCC_PCLK2Config  (RCC_HCLK_Div1);     /* PCLK2  = HCLK    */
  RCC_PCLK1Config  (RCC_HCLK_Div2);     /* PCLK1  = HCLK/2  */
  RCC_ADCCLKConfig (RCC_PCLK2_Div6);    /* ADCCLK = PCLK2/6 */

  FLASH_SetLatency(FLASH_Latency_2);    /* Flash 2 wait state */
  FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

  /* PLLCLK = 8MHz * 9 = 72 MHz */
  RCC_PLLConfig (RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

  RCC_PLLCmd (ENABLE);                  /* Enable PLL */

  /* Wait till PLL is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

  /* Select PLL as system clock source */
  RCC_SYSCLKConfig (RCC_SYSCLKSource_PLLCLK);

  /* Wait till PLL is used as system clock source */
  while (RCC_GetSYSCLKSource() != 0x08);

  /* SysTick event each 10 ms with input clock equal to 9MHz (HCLK/8) */
  SysTick_SetReload(90000);

  /* Enable the SysTick Counter */
  SysTick_CounterCmd(SysTick_Counter_Enable);

  /* Enable SysTick interrupt */
  SysTick_ITConfig(ENABLE);
}


GPIO_InitTypeDef  GPIO_InitStructure;

void SetupLED (void) {

  /* Enable GPIOC clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

  /* Configure PC6..PC9 as outputs push-pull, max speed 50 MHz */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7  | 
                                 GPIO_Pin_8 | GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void SetupKBD (void) {

  /* Enable GPIOB, GPIOD, and GPIOE clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | 
                         RCC_APB2Periph_GPIOD | 
                         RCC_APB2Periph_GPIOE, ENABLE);
  
  /* Configure PD.08, PD.12 and PD.14 as input floating */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_12 | GPIO_Pin_14;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  /* Configure PE.00 and PE.01 as input floating */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOE, &GPIO_InitStructure);

  /* Configure PB.09 as input floating */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* RIGHT Button */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource0);

  /* LEFT Button */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource1);

  /* DOWN Button */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource14);

  /* UP Button */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource8);

  /* SEL Button */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource12);

  /* KEY Button */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource9);
}
