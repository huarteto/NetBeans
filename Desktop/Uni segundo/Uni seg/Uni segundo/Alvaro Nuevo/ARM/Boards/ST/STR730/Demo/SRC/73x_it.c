/******************** (C) COPYRIGHT 2005 STMicroelectronics **********************
* File Name          : 73x_it.c
* Author             : MCD Application Team
* Date First Issued  : 09/27/2005 :  V1.0
* Description        : Main Interrupt Service Routines
**********************************************************************************
*    This file can be used to describe all the exceptions subroutines
*    that may occur within user application.
*    When an interrupt happens, the software will branch automatically
*    to the corresponding routine according to the interrupt vector
*    loaded in the PC register.
*    The following routines are all empty, user can write code for
*    exceptions handlers and peripherals IRQ interrupts.
**********************************************************************************
* History:
* 09/27/2005 :  V1.0
**********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH  CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT
* OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
* OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
* CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*********************************************************************************/

/* Include of other module interface headers ---------------------------------*/
#include "main.h"

/* Local includes ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
u8 LDirection = 0;
u16 Curr_Pin = 1;
u16 Curr2_Pin = 0x8000;
extern vu16 LCDDelay;
extern vu16 PeriodmeasureDelay;
extern vu16 TemperatureDelay;

/* Private function prototypes -----------------------------------------------*/
/* Interface functions -------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


/*******************************************************************************
* Function Name  : Undefined_Handler
* Description    : This function Undefined instruction exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Undefined_Handler(void)
{
}
/*******************************************************************************
* Function Name  : FIQ_Handler
* Description    : This function handles FIQ exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void FIQ_Handler(void)
{
}
/*******************************************************************************
* Function Name  : SWI_Handler
* Description    : This function handles SW exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SWI_Handler(void)
{
}
/*******************************************************************************
* Function Name  : Prefetch_Handler
* Description    : This function handles preftetch abort exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Prefetch_Handler(void)
{
}
/*******************************************************************************
* Function Name  : Abort_Handler
* Description    : This function handles data abort exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Abort_Handler(void)
{
}
/*******************************************************************************
* Function Name  : RCCUCMU_IRQHandler
* Description    : This function handlesthe RCCU and the CMU interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PRCCUCMU_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : EXTIT01_IRQHandler
* Description    : This function handles External line1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTIT01_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : EXTIT02_IRQHandler
* Description    : This function handles External line2 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTIT02_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : EXTIT03_IRQHandler
* Description    : This function handles External line3 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTIT03_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : EXTIT04_IRQHandler
* Description    : This function handles External line 4 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTIT04_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : EXTIT05_IRQHandler
* Description    : This function handles External line 5 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTIT05_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : EXTIT06_IRQHandler
* Description    : This function handles External line 6 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTIT06_IRQHandler (void)
{
  WDG_Cmd(DISABLE);
 
  NxtFunc();
  
  WDG_Cmd(ENABLE);
}

/*******************************************************************************
* Function Name  : EXTIT07_IRQHandler
* Description    : This function handles External line7 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTIT07_IRQHandler (void)
{
  WDG_Cmd(DISABLE);
 
  PrvFunc();

  WDG_Cmd(ENABLE);  
}

/*******************************************************************************
* Function Name  : EXTIT08_IRQHandler
* Description    : This function handles External line8 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTIT08_IRQHandler (void)
{
  WDG_Cmd(DISABLE);

  SelFunc();

  WDG_Cmd(ENABLE);  
}

/*******************************************************************************
* Function Name  : EXTIT09_IRQHandler
* Description    : This function handles External line9 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTIT09_IRQHandler (void)
{
  WDG_Cmd(DISABLE);

  BackFunc();

  WDG_Cmd(ENABLE);  
}

/*******************************************************************************
* Function Name  : EXTIT10_IRQHandler
* Description    : This function handles External line10 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTIT10_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : EXTIT11_IRQHandler
* Description    : This function handles External line11 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTIT11_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : EXTIT12_IRQHandler
* Description    : This function handles External line12 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTIT12_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : EXTIT13_IRQHandler
* Description    : This function handles External line13 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTIT13_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : EXTIT14_IRQHandler
* Description    : This function handles External line14 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTIT14_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : EXTIT15_IRQHandler
* Description    : This function handles External line15 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTIT15_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : DMATRERR_IRQHandler
* Description    : This function handles the DMA transfer error interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMATRERR_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : TIM1_IRQHandler
* Description    : This function handles the TIM1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : TIM2_IRQHandler
* Description    : This function handles the TIM2 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM2_IRQHandler (void)
{
  if(! LDirection)
  {
    GPIO_WordWrite(GPIO4, 0x0000);
  
    GPIO_BitWrite(GPIO4, Curr_Pin, Bit_SET);
    GPIO_BitWrite(GPIO4, Curr2_Pin, Bit_SET);
    Curr_Pin *= 4;
    Curr2_Pin /= 4;

    if(Curr_Pin== GPIO_PIN_14)
    {
      LDirection = 1;
      Curr_Pin = GPIO_PIN_15;
      Curr2_Pin = GPIO_PIN_0;
    }    
  }
  else
  {
    GPIO_WordWrite(GPIO4, 0x0000);
  
    GPIO_BitWrite(GPIO4, Curr_Pin, Bit_SET);
    GPIO_BitWrite(GPIO4, Curr2_Pin, Bit_SET);

    Curr_Pin /= 4;
    Curr2_Pin *= 4;

    if (Curr_Pin== GPIO_PIN_1)
    {
      LDirection = 0;
      Curr_Pin = GPIO_PIN_0;
      Curr2_Pin = GPIO_PIN_15;
    }    
  }

  TIM_FlagClear(TIM2, TIM_IT_OCB);	
}

/*******************************************************************************
* Function Name  : TIM3_IRQHandler
* Description    : This function handles the TIM3 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM3_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : TIM4_IRQHandler
* Description    : This function handles the TIM4 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM4_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : TB0_IRQHandler
* Description    : This function handles the TB0 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TB0_IRQHandler (void)
{
  PlayNextTone();

  /* clear TB1 EC flag */
  TB_FlagClear(TB0);
}

/*******************************************************************************
* Function Name  : TB1_IRQHandler
* Description    : This function handles the TB1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TB1_IRQHandler (void)
{
  if(TemperatureDelay == 0xFFFF)
  {
    TemperatureDelay = 0;
  }

  TemperatureDelay++;


  if(LCDDelay == 0xFFFF)
  {
    LCDDelay = 0;
  }

  LCDDelay++;


  if(PeriodmeasureDelay == 0xFFFF)
  {
    PeriodmeasureDelay = 0;
  }

  PeriodmeasureDelay++;

  /* clear TB1 EC flag */
  TB_FlagClear(TB1);
}

/*******************************************************************************
* Function Name  : TB2_IRQHandler
* Description    : This function handles the TB2 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TB2_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : TIM5_IRQHandler
* Description    : This function handles the TIM5 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM5_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : TIM6_IRQHandler
* Description    : This function handles the TIM6 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM6_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : TIM7_IRQHandler
* Description    : This function handles the TIM7 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM7_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : TIM8_IRQHandler
* Description    : This function handles the TIM8 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM8_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : TIM9_IRQHandler
* Description    : This function handles the TIM9 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM9_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : UART2_IRQHandler
* Description    : This function handles the UART2 interrupt request.
* Input          : detail input parameters
* Output         : detail output parameters
* Return         : detail return value
*******************************************************************************/
void UART2_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : UART3_IRQHandler
* Description    : This function handles the UART3 interrupt request.
* Input          : detail input parameters
* Output         : detail output parameters
* Return         : detail return value
*******************************************************************************/
void UART3_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : FlashEOP_IRQHandler
* Description    : This function handles the FLASH end of program interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void FLASHEOP_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : PWM0_IRQHandler
* Description    : This function handles the PWM0 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PWM0_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : PWM1_IRQHandler
* Description    : This function handles the PWM1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PWM1_IRQHandler (void)
{
  if((PWM1->PLS && 1) == 1)
  {
    PWM0->PLS &= ~1;
    PWM1->PLS &= ~1;
    PWM2->PLS &= ~1;
    PWM3->PLS |= 1;
    PWM4->PLS |= 1;
    PWM5->PLS |= 1;
  }
  else 
  {
    PWM0->PLS |= 1;
    PWM1->PLS |= 1;
    PWM2->PLS |= 1;
    PWM3->PLS &= ~1;
    PWM4->PLS &= ~1;
    PWM5->PLS &= ~1;
  }
  
  PWM_FlagClear(PWM1);
}

/*******************************************************************************
* Function Name  : PWM2_IRQHandler
* Description    : This function handles the PWM2 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PWM2_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : PWM3_IRQHandler
* Description    : This function handles the PWM3 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PWM3_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : PWM4_IRQHandler
* Description    : This function handles the PWM4 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PWM4_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : PWM5_IRQHandler
* Description    : This function handles the PWM5 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PWM5_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : WIU_IRQHandler
* Description    : This function handles the wake-up system interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void WIU_IRQHandler (void)
{
  /* clear wake-up line16 pending bit */
  WIU_PendingBitClear(WIU_Line16);
}

/*******************************************************************************
* Function Name  : WDGWUT_IRQHandler
* Description    : This function handles the watchdog and the wakeup timer interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void WDGWUT_IRQHandler (void)
{
  if(WUT_FlagStatus(WUT_FLAG_EC) == SET)
  {
    /* stop WUT */
    WUT_Cmd(DISABLE);
  
    /* clear WUT End of Count flag */
    WUT_FlagClear();
  }
  else if(WDG_FlagStatus() == SET) 
  {
    LCD_LineClear(1);
    LCD_SendString("      :  :      ", 2, 1);

    ScreenSaver_Show();
    LCD_Clear();
    DisplayMenu();
    WDG_FlagClear();
  } 
}

/*******************************************************************************
* Function Name  : BSPI0_IRQHandler
* Description    : This function handles the BSPI0 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void BSPI0_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  :  BSPI1_IRQHandler
* Description    : This function handles the BSPI1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void BSPI1_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : BSPI2_IRQHandler
* Description    : This function handles the BSPI2 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void BSPI2_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : UART0_IRQHandler
* Description    : This function handles the UART0 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART0_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : UART1_IRQHandler
* Description    : This function handles the UART1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART1_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : I2C0ITERR_IRQHandler
* Description    : This function handles the I2C0 and DMA Error Interrupt requests
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C0ITERR_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : I2C1ITERR_IRQHandler
* Description    : This function handles the I2C1 and DMA Error Interrupt requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C1ITERR_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : I2C0ITDDC_IRQHandler
* Description    : This function handles the I2C0 and the General Interrupt requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C0ITDDC_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : I2C1ITDDC_IRQHandler
* Description    : This function handles the I2C1 and the General Interrupt requests
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C1ITDDC_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : CAN0_IRQHandler
* Description    : This function handles the CAN0 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN0_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : CAN1_IRQHandler
* Description    : This function handles the CAN1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN1_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : CAN2_IRQHandler
* Description    : This function handles the CAN2 interrupt request.
* Input          : detail input parameters
* Output         : detail output parameters
* Return         : detail return value
*******************************************************************************/
void CAN2_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : DMA0_IRQHandler
* Description    : This function handles the DMA0 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA0_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : DMA1_IRQHandler
* Description    : This function handles the DMA1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : DMA2_IRQHandler
* Description    : This function handles the DMA2 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA2_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : DMA3_IRQHandler
* Description    : This function handles the DMA3 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA3_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : ADC_IRQHandler
* Description    : This function handles the ADC interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ADC_IRQHandler (void)
{
}

/*******************************************************************************
* Function Name  : RTC_IRQHandler
* Description    : This function handles the RTC interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RTC_IRQHandler (void)
{
  if((RTC_GetCounterValue()/3600==23)&&(((RTC_GetCounterValue()%3600)/60)==59)&&
     (((RTC_GetCounterValue()%3600)%60)==59)) /* 23*3600+59*60+59=86339 */
  {
    RTC_SetCounter(0x0);
    Date_Update();
  }

  if(RTC_FlagStatus(RTC_FLAG_ALA) == SET)
  { 
    PlayAlarmMelody();
  }  

  RTC_FlagClear(RTC_FLAG_GL | RTC_FLAG_SEC | RTC_FLAG_ALA);
}

/******************* (C) COPYRIGHT 2005 STMicroelectronics *****END OF FILE****/
