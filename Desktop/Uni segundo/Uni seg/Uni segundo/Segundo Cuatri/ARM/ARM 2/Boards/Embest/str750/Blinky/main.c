/******************** (C) COPYRIGHT 2006 STMicroelectronics ********************
* File Name          : main.c
* Author             : MCD Application Team
* Date First Issued  : 03/10/2006
* Description        : Main program body
********************************************************************************

********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "75x_lib.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
GPIO_InitTypeDef GPIO_InitStructure;
ErrorStatus OSC4MStartUpStatus;

/* Private function prototypes -----------------------------------------------*/
void MRCC_Configuration(void);
static void Delay(u32 nCount);

/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : main
* Description    : Main program
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int main()
{
  #ifdef DEBUG
    debug();
  #endif

  /* Configure the system clocks */
  MRCC_Configuration();

/* GPIO Configuration --------------------------------------------------------*/
  /* Configure P2.07and P2.09and p2.05 sh-pull */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_9 | GPIO_Pin_7 | GPIO_Pin_5;
  GPIO_Init(GPIO2, &GPIO_InitStructure);

  /* Configure P1.10and p1.06+ as output push-pull */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10 | GPIO_Pin_6;
  GPIO_Init(GPIO1, &GPIO_InitStructure);

  
  /* Mask the unused pins */
  GPIO_PinMaskConfig(GPIO2, ~(GPIO_Pin_9 | GPIO_Pin_7 | GPIO_Pin_5), ENABLE);
  GPIO_PinMaskConfig(GPIO1, ~(GPIO_Pin_10 | GPIO_Pin_6), ENABLE);


  while(1)
  {
   

     /* Turn on led connected to P1.10 */
    GPIO_Write(GPIO2, 0x00000000);
    GPIO_Write(GPIO1, 0x00000400);

    /* Insert delay */
    Delay(0xFFFFF);

   /* Turn on led connected to P2.09 */
	GPIO_Write(GPIO2, 0x00000200);
    GPIO_Write(GPIO1, 0x00000000);
    
	/* Insert delay */
    Delay(0xFFFFF);

	/* Turn on led connected to P2.07 */
	GPIO_Write(GPIO2, 0x00000080);
    GPIO_Write(GPIO1, 0x00000000);
    
	/* Insert delay */
    Delay(0xFFFFF);

	/* Turn on led connected to P2.05 */
	GPIO_Write(GPIO2, 0x00000020);
    GPIO_Write(GPIO1, 0x00000000);
    
	/* Insert delay */
    Delay(0xFFFFF);




   /* Turn on led connected to P1.06*/
    GPIO_Write(GPIO2, 0x00000000);
    GPIO_Write(GPIO1, 0x00000040);

    /* Insert delay */
    Delay(0xFFFFF);
   }
}

/*******************************************************************************
* Function Name  : MRCC_Configuration
* Description    : Configures the different system clocks.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MRCC_Configuration(void)
{
  /* MRCC system reset(for debug purpose) */
  MRCC_DeInit();

  /* Wait for OSC4M start-up */
  OSC4MStartUpStatus = MRCC_WaitForOSC4MStartUp();

  if(OSC4MStartUpStatus == SUCCESS)
  {
    /* Set HCLK to 30 MHz */
    MRCC_HCLKConfig(MRCC_CKSYS_Div2);

    /* Set CKTIM to 15 MHz */
    MRCC_CKTIMConfig(MRCC_HCLK_Div2);

    /* Set PCLK to 15 MHz */
    MRCC_PCLKConfig(MRCC_CKTIM_Div1);

    /* Set CKSYS to 60 MHz */
    MRCC_CKSYSConfig(MRCC_CKSYS_OSC4MPLL, MRCC_PLL_Mul_15);
  }

  /* GPIO pins optimized for 3V3 operation */
  MRCC_IOVoltageRangeConfig(MRCC_IOVoltageRange_3V3); 
  
  /* Enable GPIO clock */
  MRCC_PeripheralClockConfig(MRCC_Peripheral_GPIO, ENABLE);
}

/*******************************************************************************
* Function Name  : Delay
* Description    : Inserts a delay time.
* Input          : nCount: specifies the delay time length.
* Output         : None
* Return         : None
*******************************************************************************/
static void Delay(u32 nCount)
{
  u32 j = 0;

  for(j = nCount; j != 0; j--);
}

/******************* (C) COPYRIGHT 2006 STMicroelectronics *****END OF FILE****/
