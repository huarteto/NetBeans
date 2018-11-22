/******************** (C) COPYRIGHT 2005 STMicroelectronics ********************
* File Name          : periodmeasure.c
* Author             : MCD Application Team
* Date First Issued  : 09/27/2005 :  V1.0
* Description        : This file provides function to measure the period and the
*                      duty cycle of an external signal.
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

/* Standard include ----------------------------------------------------------*/
#include "main.h"

/* Include of other module interface headers ---------------------------------*/
/* Local includes ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define PeriodSampling    1200  /* 150 ms */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
TIM_InitTypeDef  TIM_InitStructure4;
GPIO_InitTypeDef GPIO1_InitStructure;
vu16 PeriodmeasureDelay = 0;

/* Private function prototypes -----------------------------------------------*/
/* Interface functions -------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : PeriodPulseMeasure
* Description    : Measures the period and the duty cycle of an external signal
*                  connected to P1.3 pin. The signal measurement is performed
*                  each 150 ms.
* Input          : None
* Output         : PeriodmeasureDelay: incremented each 125 µs in TB1 interrupt
*                  handler routine and used to define the delay time between two
*                  consecutive measure.
* Return         : None
*******************************************************************************/
void PeriodPulseMeasure(void)
{  
  u16 SignalDuty = 1, TIMCounter = 1;
  u32 SignalFrequency = 1;
  
  /* disable screen saver mode */	
  WDG_Cmd(DISABLE);
  
  /* P1.3 configuration */
  GPIO1_InitStructure.GPIO_Mode = GPIO_Mode_IN_TRI_TTL;
  GPIO1_InitStructure.GPIO_Pins = GPIO_PIN_3;
  GPIO_Init(GPIO1, &GPIO1_InitStructure);

  /* TIM4 in PWMI Mode */  
  TIM_InitStructure4.TIM_Mode = TIM_PWMI;
  TIM_InitStructure4.TIM_Clock_Source = TIM_CLK_INTERNAL;
  TIM_InitStructure4.TIM_Prescaler = 0x00;
  TIM_InitStructure4.TIM_ICAPA_Modes = TIM_Rising;
  
  TIM_DeInit(TIM4);
  TIM_Init(TIM4, &TIM_InitStructure4);

  /* start TIM4 */
  TIM_CounterCmd(TIM4, TIM_START);

  LCD_SendString("duty cycle:   %" , 1, 1);
  LCD_SendString("freq:         Hz" , 2, 1);

  /* while the BACK push-button is not pressed, display the signal period
     and duty cycle  */
  while(ReadKey() != BACK)
  { 
    if(TIM_FlagStatus(TIM4, TIM_FLAG_TO) == SET)
    {
      if((TIM_FlagStatus(TIM4, TIM_FLAG_ICB) == RESET)&&
         (TIM_FlagStatus(TIM4, TIM_FLAG_ICA) == RESET))
      {
        LCD_SendString(" !!no signal!!  ", 1, 1);
        LCD_LineClear(2); 
      }
      else
      { 
        if(TIM_FlagStatus(TIM4, TIM_FLAG_ICA) == SET)
        {   
          LCD_SendString(" out of range!! " , 1, 1);
          LCD_SendString(" freq < 488 HZ  " , 2, 1);    	
        }
      }
    }  
    else
    { 
      if((TIM_FlagStatus(TIM4, TIM_FLAG_ICA) == SET) &&
         (TIM_FlagStatus(TIM4, TIM_FLAG_ICB) == SET))
      {	  
        TIMCounter = TIM_GetPWMIPeriod(TIM4);
  
        SignalFrequency = (32000000 / TIMCounter) + 1 ;
        SignalDuty = (TIM_GetPWMIPulse(TIM4)*100 / TIMCounter) + 1;
      
        if(SignalFrequency > 10000)
        {
          LCD_SendString(" out of range!! " , 1, 1);
          LCD_SendString(" freq > 10 KHz  " , 2, 1);
        }
        else
        {
          LCD_SendString("duty cycle:    %" , 1, 1);
          LCD_SendString("freq:         Hz" , 2, 1);
          LCD_SetPosCur(1, 13);
          LCD_SendData((SignalDuty/10)+48);
          LCD_SetPosCur(1, 14);
          LCD_SendData((SignalDuty%10)+48);           	
          LCD_SetPosCur(2, 9);
          LCD_SendData(((SignalFrequency%10000)/1000)+48);
          LCD_SendData(((SignalFrequency%1000)/100)+48);
          LCD_SendData(((SignalFrequency%100)/10)+48);
          LCD_SendData((SignalFrequency%10)+48);
        }
      }
    }     
  
    TIM_FlagClear(TIM4, TIM_FLAG_ICA | TIM_FLAG_ICB | TIM_FLAG_TO); 
    
    /* wait 150 ms */ 
    PeriodmeasureDelay = 0;
    while(PeriodmeasureDelay < PeriodSampling); 
}   

  /* enable screen saver mode */
  WDG_Cmd(ENABLE);
}    
    
/******************* (C) COPYRIGHT 2005 STMicroelectronics *****END OF FILE****/
