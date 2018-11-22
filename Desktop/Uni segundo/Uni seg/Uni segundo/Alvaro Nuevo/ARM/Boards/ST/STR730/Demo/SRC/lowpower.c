/******************** (C) COPYRIGHT 2005 STMicroelectronics ********************
* File Name          : lowpower.c
* Author             : MCD Application Team
* Date First Issued  : 09/27/2005 :  V1.0
* Description        : This file provides low power modes functions.
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

/* Standard includes -------------------------------------------------------- */
#include "main.h"

/* Include of other module interface headers ---------------------------------*/
/* Local includes ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define  WIU_STOP_Bit  0x04

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
WIU_InitTypeDef    WIU_InitStructure;
CMU_InitTypeDef    CMU_InitStructure;
PRCCU_InitTypeDef  PRCCU_InitStructure;
WUT_InitTypeDef    WUT_InitStructure;
GPIO_InitTypeDef   GPIO2_InitStructure2;

/* Private function prototypes -----------------------------------------------*/
/* Interface functions -------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : EnterStopMode
* Description    : Enters MCU Stop mode and select to wake-up using WAKE-UP 
*                  push-button (connected to P2.10 pin).
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EnterStopMode(void)
{
  u16 tmp0 = 0, tmp1 = 0, tmp2 = 0;	
  
  /* store the current GPIO2 configuration */
  tmp0 = GPIO2->PC0;
  tmp1 = GPIO2->PC1;
  tmp2 = GPIO2->PC2;

/* wake-up the MCU from Stop mode using WAKE-UP push-button ------------------*/  
  /* WAKE-UP push-button configuration */
  GPIO2_InitStructure2.GPIO_Mode = GPIO_Mode_IN_TRI_TTL;
  GPIO2_InitStructure2.GPIO_Pins = GPIO_PIN_10;
  GPIO_Init(GPIO2, &GPIO2_InitStructure2);
  
  /* WIU configuration */
  WIU_InitStructure.WIU_Mode = WIU_Mode_WakeUpInterrupt;
  WIU_InitStructure.WIU_TriggerEdge = WIU_FallingEdge;
  WIU_InitStructure.WIU_Line = WIU_Line16;  

  WIU_DeInit();
  WIU_Init(&WIU_InitStructure);
 
  EIC_IRQChannelConfig(WIU_IRQChannel , ENABLE);
  EIC_IRQChannelPriorityConfig(WIU_IRQChannel, 5);

/* Stop mode entering sequence -----------------------------------------------*/
  do
  {  
    WIU_EnterStopMode();
    
  }/* assure the Stop mode was really entered STOP_I = 1  STOP = 0 */
  while((PRCCU_FlagStatus(PRCCU_FLAG_STOP_I) == RESET)||
        ((WIU->CTRL & WIU_STOP_Bit) == SET));   


/* at this stage the MCU entered and exited Stop mode ------------------------*/
  /* return to "STOP Mode" menu */
  BackFunc();

  /* clear STOP_I flag */ 
  PRCCU_FlagClear(PRCCU_FLAG_STOP_I); 
 
  /* disable WIU interrupt */
  EIC_IRQChannelConfig(WIU_IRQChannel, DISABLE);
  
  /* set the system clock to 32 MHz */
  SetClockTo32(); 
 
  /* restore the GPIO2 configuration */
  GPIO2->PC0 = tmp0;
  GPIO2->PC1 = tmp1;
  GPIO2->PC2 = tmp2; 
}

/*******************************************************************************
* Function Name  : EnterStopMode_WUT
* Description    : Enters MCU Stop mode and select to wake-up by the WUT End of
*                  Count event.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EnterStopMode_WUT(void)
{
  /* RC oscillator frequency:= 29 KHz in Stop mode */  
  CMU_Lock(DISABLE);
  CMU_ModeOscConfig(CMU_Stop_Low); 

/* configure the WUT to wake-up the MCU from Stop mode in 10s ----------------*/
  /* WUT End of Count event connected to WIU input line 0 */
  WUT_InitStructure.WUT_Mode = WUT_Mode_WakeUp; 
  
  /* WUT clock 29 KHz (when the MCU enter Stop mode) */
  WUT_InitStructure.WUT_CLK_Source = WUT_CLK_EXTERNAL;   
  
  WUT_InitStructure.WUT_Prescaler = 0x63;
  WUT_InitStructure.WUT_Preload = 0xB53;
  WUT_DeInit();
  WUT_Init(&WUT_InitStructure);

  /* enable WUT End of Count interrupt */
  WUT_ITConfig(ENABLE); 

  /* select the WUT to wake-up the MCU from Stop mode */
  WIU_InitStructure.WIU_Mode = WIU_Mode_WakeUp;  
  WIU_InitStructure.WIU_TriggerEdge = WIU_FallingEdge;
  WIU_InitStructure.WIU_Line = WIU_Line0;  
  WIU_DeInit();
  WIU_Init(&WIU_InitStructure);  

  /* start WUT */
  WUT_Cmd(ENABLE);

/* Stop mode entering sequence -----------------------------------------------*/
  do
  {  
    WIU_EnterStopMode();
    
  }/* assure the Stop mode was really entered STOP_I = 1  STOP = 0 */
  while((PRCCU_FlagStatus(PRCCU_FLAG_STOP_I) == RESET)||
        ((WIU->CTRL & WIU_STOP_Bit) == SET));    

/* at this stage the MCU entered and exited Stop mode ------------------------*/
  /* return to "STOP Mode" menu */
  BackFunc();

  /* clear STOP_I flag */ 
  PRCCU_FlagClear(PRCCU_FLAG_STOP_I); 

  /* clear wake-up line 0 flag */
  WIU_PendingBitClear(WIU_Line0);                                                    

  /* set the system clock to 32 MHz */
  SetClockTo32();
}

/*******************************************************************************
* Function Name  : EnterHALTMode
* Description    : Enters MCU HALT mode. 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EnterHALTMode(void)
{
  PRCCU_EnterLPM(PRCCU_LPM_HALT); 
}

/*******************************************************************************
* Function Name  : SetClockTo32
* Description    : Sets the system clock to 32 MHz, using the external 8 MHz 
*                  oscillator.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SetClockTo32(void)
{
  /* select Quartz oscillator as PRCCU clock input */
  CMU_StructInit(&CMU_InitStructure);
  CMU_InitStructure.CMU_CKSEL0 = CMU_CKSEL0_CKOSC;
  CMU_Init(&CMU_InitStructure);
  
  /* set MCLK clock to 32 MHz */
  PRCCU_InitStructure.PRCCU_DIV2 = ENABLE;
  PRCCU_InitStructure.PRCCU_MCLKSRC_SRC = PRCCU_MCLKSRC_PLL ;
  PRCCU_InitStructure.PRCCU_PLLDIV = PRCCU_PLLDIV_2 ;
  PRCCU_InitStructure.PRCCU_PLLMUL = PRCCU_PLLMUL_16;
  PRCCU_Init(&PRCCU_InitStructure);
}

/*******************************************************************************
* Function Name  : SetClockTo16
* Description    : Sets the system clock to 16 MHz, using the external 8 MHz 
*                  oscillator.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SetClockTo16(void)
{
  /* select Quartz oscillator as PRCCU clock input */
  CMU_StructInit(&CMU_InitStructure);
  CMU_InitStructure.CMU_CKSEL0 = CMU_CKSEL0_CKOSC;
  CMU_Init(&CMU_InitStructure);
  
  /* set MCLK clock to 16 MHz */
  PRCCU_InitStructure.PRCCU_DIV2 = ENABLE;
  PRCCU_InitStructure.PRCCU_MCLKSRC_SRC = PRCCU_MCLKSRC_PLL ;
  PRCCU_InitStructure.PRCCU_PLLDIV = PRCCU_PLLDIV_3 ;
  PRCCU_InitStructure.PRCCU_PLLMUL = PRCCU_PLLMUL_12;
  PRCCU_Init(&PRCCU_InitStructure);
}

/*******************************************************************************
* Function Name  : SetClockTo8
* Description    : Sets the system clock to 8 MHz, using the external 8 MHz 
*                  oscillator.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SetClockTo8(void)
{
  /* select Quartz oscillator as PRCCU clock input */
  CMU_StructInit(&CMU_InitStructure);
  CMU_InitStructure.CMU_CKSEL0 = CMU_CKSEL0_CKOSC;
  CMU_Init(&CMU_InitStructure);
  
  /* set MCLK clock to 8 MHz */
  PRCCU_InitStructure.PRCCU_DIV2 = DISABLE;
  PRCCU_InitStructure.PRCCU_MCLKSRC_SRC = PRCCU_MCLKSRC_CLOCK2 ;
  PRCCU_Init(&PRCCU_InitStructure);
}

/*******************************************************************************
* Function Name  : SetClockTo4
* Description    : Sets the system clock to 4 MHz, using the external 8MHz 
*                  oscillator.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SetClockTo4(void)
{
  /* select Quartz oscillator as PRCCU clock input */
  CMU_StructInit(&CMU_InitStructure);
  CMU_InitStructure.CMU_CKSEL0 = CMU_CKSEL0_CKOSC;
  CMU_Init(&CMU_InitStructure);
  
  /* set MCLK clock to 4 MHz */
  PRCCU_InitStructure.PRCCU_DIV2 = ENABLE;
  PRCCU_InitStructure.PRCCU_MCLKSRC_SRC = PRCCU_MCLKSRC_CLOCK2 ;
  PRCCU_Init(&PRCCU_InitStructure);
}

/******************* (C) COPYRIGHT 2005 STMicroelectronics *****END OF FILE****/
