/******************************************************************************/
/* SETUP.C: Setup Functions                                                   */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2008 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <stm32f10x_lib.h>              /* STM32F10x Library Definitions      */

GPIO_InitTypeDef  GPIO_InitStructure;


void SetupClock (void)
{
  RCC_DeInit ();                        /* RCC system reset(for debug purpose)*/
  RCC_HSEConfig (RCC_HSE_ON);           /* Enable HSE                         */

  /* Wait till HSE is ready                                                   */
  while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET);

  RCC_HCLKConfig   (RCC_SYSCLK_Div1);   /* HCLK   = SYSCLK                    */
  RCC_PCLK2Config  (RCC_HCLK_Div1);     /* PCLK2  = HCLK                      */
  RCC_PCLK1Config  (RCC_HCLK_Div2);     /* PCLK1  = HCLK/2                    */
  RCC_ADCCLKConfig (RCC_PCLK2_Div4);    /* ADCCLK = PCLK2/4                   */

  *(vu32 *)0x40022000 = 0x01;           /* Flash 2 wait state                 */

  /* PLLCLK = 8MHz * 9 = 72 MHz                                               */
  RCC_PLLConfig (RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

  RCC_PLLCmd (ENABLE);                  /* Enable PLL                         */

  /* Wait till PLL is ready                                                   */
  while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

  /* Select PLL as system clock source                                        */
  RCC_SYSCLKConfig (RCC_SYSCLKSource_PLLCLK);

  /* Wait till PLL is used as system clock source                             */
  while (RCC_GetSYSCLKSource() != 0x08);

  /* Enable USART1 and GPIOA clock                                            */
  RCC_APB2PeriphClockCmd (RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

  /* SysTick event each 10 ms with input clock equal to 9MHz (HCLK/8)         */
  SysTick_SetReload(90000);

  SysTick_ITConfig(ENABLE);             /* Enable SysTick interrupt           */
}


void SetupLED (void) {

  /* Enable GPIOB clock                                                       */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

  /* Configure PB8..PB15 as outputs push-pull, max speed 50 MHz               */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8  | GPIO_Pin_9  | GPIO_Pin_10 | 
                                 GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 |
                                 GPIO_Pin_14 | GPIO_Pin_15 ;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* Enable the SysTick Counter                                               */
  SysTick_CounterCmd(SysTick_Counter_Enable);
}
