/******************** (C) COPYRIGHT 2006 STMicroelectronics ********************
* File Name          : waveplayer.c
* Author             : MCD Application Team
* Date First Issued  : 05/18/2006 : Version 1.0
* Description        : This file includes the waveplayer driver of STR91x-EVAL.
********************************************************************************
* History:
* 05/24/2006 : Version 1.1
* 05/18/2006 : Version 1.0
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

  extern vu32 index;
  extern u16 Pulse;
  extern bool IntroMenu;
  extern bool AlarmStatus;


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : WavePlay_Init
* Description    : Wave player Initialization
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void WavePlayer_Init(void)
{
  TIM_InitTypeDef      TIM_InitStructure;
  GPIO_InitTypeDef     GPIO_InitStructure;

  GPIO_DeInit(GPIO4);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Direction = GPIO_PinOutput;
  GPIO_InitStructure.GPIO_Type = GPIO_Type_PushPull;
  GPIO_InitStructure.GPIO_IPInputConnected = GPIO_IPInputConnected_Disable;
  GPIO_InitStructure.GPIO_Alternate = GPIO_OutputAlt2;
  GPIO_Init(GPIO4,&GPIO_InitStructure);

  TIM_StructInit(&TIM_InitStructure);

  /* 22.050 KHz generation */
  TIM_InitStructure.TIM_Mode = TIM_OCM_CHANNEL_1;
  TIM_InitStructure.TIM_OC1_Modes = TIM_TIMING;
  TIM_InitStructure.TIM_Clock_Source = TIM_CLK_APB;
  TIM_InitStructure.TIM_Prescaler = 0x1;
  TIM_InitStructure.TIM_Pulse_Length_1 = 0x440;

  /* Initialize the Timer 0 */
  TIM_Init (TIM0, &TIM_InitStructure);

  TIM_StructInit(&TIM_InitStructure);

  /* PWM configuration */
  TIM_InitStructure.TIM_Mode = TIM_PWM;
  TIM_InitStructure.TIM_Clock_Source = TIM_CLK_APB;
  TIM_InitStructure.TIM_Prescaler = 0x0;
  TIM_InitStructure.TIM_Pulse_Level_1 = TIM_HIGH;
  TIM_InitStructure.TIM_Period_Level = TIM_LOW;
  TIM_InitStructure.TIM_Pulse_Length_1 = 0x7F;
  TIM_InitStructure.TIM_Full_Period = 0xFF;

  /* Initialize the Timer 3 */
  TIM_Init (TIM3, &TIM_InitStructure);
}

/*******************************************************************************
* Function Name  : WavePlayer_Start
* Description    : Start wave playing: Osiris or recorded waves
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void WavePlayer_Start(void)
{
  /* Enable the Timer Overflow interrupt */
  TIM_ITConfig(TIM0, TIM_IT_OC1, ENABLE);
  /* Start the Timer counter */
  TIM_CounterCmd(TIM3, TIM_START);
  /* Start the Timer counter */
  TIM_CounterCmd(TIM0, TIM_START);

  if(IntroMenu != FALSE)
  {
    LeftFunc();
  }
}

/*******************************************************************************
* Function Name  : WavePlayer_Stop
* Description    : Stop wave playing: Osiris or recorded waves
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void WavePlayer_Stop(void)
{
  /* Disable the Timer Overflow interrupt */
  TIM_ITConfig(TIM0, TIM_IT_OC1, DISABLE);

  /* Stop the Timer counter */
  TIM_CounterCmd(TIM3, TIM_STOP);

  /* Stop the Timer counter */
  TIM_CounterCmd(TIM0, TIM_STOP);

  index = 0;
  Pulse = 0;
  if(IntroMenu != FALSE)
  {
    RightFunc();
  }
}

/*******************************************************************************
* Function Name  : WavePlayerAlarm_Start
* Description    : Start wave alarm playing
* Input          : None
* Output         : None
* Return         : None.
*******************************************************************************/
void WavePlayerAlarm_Start(void)
{
  /* Stop the Wave player and reset the counter */
  WavePlayer_Stop();

  /* Enable the Timer Overflow interrupt */
  TIM_ITConfig(TIM0, TIM_IT_OC1, ENABLE);
  /* Start the Timer counter */
  TIM_CounterCmd(TIM3, TIM_START);
  /* Start the Timer counter */
  TIM_CounterCmd(TIM0, TIM_START);

}

/*******************************************************************************
* Function Name  : WavePlayerAlarm_Stop
* Description    : Stop wave alarm playing
* Input          : None
* Output         : None
* Return         : None.
*******************************************************************************/
void WavePlayerAlarm_Stop(void)
{
  /* Disable the Timer Overflow interrupt */
  TIM_ITConfig(TIM0, TIM_IT_OC1, DISABLE);

  /* Stop the Timer counter */
  TIM_CounterCmd(TIM3, TIM_STOP);

  /* Stop the Timer counter */
  TIM_CounterCmd(TIM0, TIM_STOP);

  AlarmStatus = FALSE;
  RTC_AlarmCmd(ENABLE);
}

/******************* (C) COPYRIGHT 2006 STMicroelectronics *****END OF FILE****/

