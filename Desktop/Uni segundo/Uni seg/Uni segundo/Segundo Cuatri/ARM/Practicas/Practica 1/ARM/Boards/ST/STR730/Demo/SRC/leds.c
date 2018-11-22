/******************** (C) COPYRIGHT 2005 STMicroelectronics ********************
* File Name          : leds.c
* Author             : MCD Application Team
* Date First Issued  : 09/27/2005 :  V1.0
* Description        : This file provides functions to manage red leds connected 
*                      to GPIO4 and bicolor leds connected GPIO2.
********************************************************************************
* History:
* 09/27/2005 :  V1.0
********************************************************************************
 THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS WITH
 CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
 AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT
 OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
 OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
 CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* includes-------------------------------------------------------------------*/
#include "main.h"

/* Include of other module interface headers ---------------------------------*/
/* Local includes ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define PWM_PERIOD  0x1FFF

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
GPIO_InitTypeDef  GPIO2_InitStructure;
TIM_InitTypeDef   TIM2_InitStructure;
PWM_InitTypeDef   PWM_InitStructure1;

/* Private function prototypes -----------------------------------------------*/
/* Interface functions -------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : LedShow
* Description    : Enables or disables the successive lighting of red leds  
*                  connected to GPIO4. The leds lighting sequence is performed
*                  in the TIM2 interrupt handler routine.
* Input          : NewState: new state of red leds lighting.
* Output         : None
* Return         : None
*******************************************************************************/
void LedShow(FunctionalState NewState)
{
  if(NewState == ENABLE)
  { 
    TIM2_InitStructure.TIM_Clock_Source = TIM_CLK_INTERNAL;
    TIM2_InitStructure.TIM_Prescaler = 0x0F;
    TIM2_InitStructure.TIM_Mode = TIM_OCM_CHANNELB;
    TIM2_InitStructure.TIM_OCB_Modes = TIM_Wave ;
    TIM2_InitStructure.TIM_Pulse_Level_B = TIM_High;
    TIM2_InitStructure.TIM_Pulse_Length_B = 0xF0;

    TIM_Init(TIM2, &TIM2_InitStructure);
    
    TIM_ITConfig(TIM2, TIM_IT_OCB , ENABLE);
    TIM_CounterCmd(TIM2, TIM_START);

    EIC_IRQChannelConfig(TIM2_IRQChannel, ENABLE);
    EIC_IRQChannelPriorityConfig(TIM2_IRQChannel, 3);    
  }
  else
  {
    GPIO_WordWrite(GPIO4, 0x0000);
    TIM_CounterCmd(TIM2, TIM_STOP);
    EIC_IRQChannelConfig(TIM2_IRQChannel, DISABLE);
  }
}

/*******************************************************************************
* Function Name  : BicolorShow
* Description    : Enables or disables the successive lighting of bicolor leds  
*                  connected to GPIO2. The bicolor leds lighting sequence is 
*                  performed in the PWM1 interrupt handler routine.
* Input          : NewState: new state of red leds lighting.
* Output         : None
* Return         : None
*******************************************************************************/
void BicolorShow(FunctionalState NewState)
{
  /* GPIO2 Configuration  */
  GPIO2_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO2_InitStructure.GPIO_Pins = GPIO_PIN_0 | GPIO_PIN_3 | GPIO_PIN_4 | 
                                  GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 ;
                                  
  GPIO_Init(GPIO2, &GPIO2_InitStructure);

  if(NewState == ENABLE)
  {
    /* PWM0..5 Configuration */
    PWM_StructInit(&PWM_InitStructure1);

    PWM_InitStructure1.PWM_Prescaler0 = 0x07 ;
    PWM_InitStructure1.PWM_Prescaler1 = 0x4;
    PWM_InitStructure1.PWM_DutyCycle = (PWM_PERIOD/6);
    PWM_InitStructure1.PWM_Period = PWM_PERIOD;
    PWM_InitStructure1.PWM_Polarity_Level = PWM_POLARITY_High;

    PWM_DeInit(PWM0);
    PWM_Init(PWM0, &PWM_InitStructure1);

    PWM_InitStructure1.PWM_DutyCycle = 2*(PWM_PERIOD/6);
    PWM_DeInit(PWM1);
    PWM_Init(PWM1, &PWM_InitStructure1);

    PWM_InitStructure1.PWM_DutyCycle = 3*(PWM_PERIOD/6);

    PWM_DeInit(PWM2);
    PWM_Init(PWM2, &PWM_InitStructure1);

    PWM_InitStructure1.PWM_DutyCycle = 4*(PWM_PERIOD/6);
    PWM_InitStructure1.PWM_Polarity_Level = PWM_POLARITY_Low;
    PWM_DeInit(PWM3);
    PWM_Init(PWM3, &PWM_InitStructure1);

    PWM_InitStructure1.PWM_DutyCycle = 5*(PWM_PERIOD/6);
    PWM_DeInit(PWM4);
    PWM_Init(PWM4, &PWM_InitStructure1);

    PWM_InitStructure1.PWM_DutyCycle = 6*(PWM_PERIOD/6);
    PWM_DeInit(PWM5);
    PWM_Init(PWM5, &PWM_InitStructure1);

    PWM_Cmd(PWM0, ENABLE);
    PWM_Cmd(PWM1, ENABLE);
    PWM_Cmd(PWM2, ENABLE);
    PWM_Cmd(PWM3, ENABLE);
    PWM_Cmd(PWM4, ENABLE);
    PWM_Cmd(PWM5, ENABLE);

    PWM_ITConfig(PWM1, ENABLE);
    
    EIC_IRQChannelConfig(PWM1_IRQChannel, ENABLE );
    EIC_IRQChannelPriorityConfig(PWM1_IRQChannel, 3);
  }
  else
  {
    PWM0->PLS |= 1;
    PWM1->PLS |= 1;
    PWM2->PLS |= 1;
    PWM3->PLS |= 1;
    PWM4->PLS |= 1;
    PWM5->PLS |= 1;

    PWM_Cmd(PWM0, DISABLE);
    PWM_Cmd(PWM1, DISABLE);
    PWM_Cmd(PWM2, DISABLE);
    PWM_Cmd(PWM3, DISABLE);
    PWM_Cmd(PWM4, DISABLE);
    PWM_Cmd(PWM5, DISABLE);
    EIC_IRQChannelConfig(PWM1_IRQChannel, DISABLE);
  }
}

/******************* (C) COPYRIGHT 2005 STMicroelectronics *****END OF FILE****/
