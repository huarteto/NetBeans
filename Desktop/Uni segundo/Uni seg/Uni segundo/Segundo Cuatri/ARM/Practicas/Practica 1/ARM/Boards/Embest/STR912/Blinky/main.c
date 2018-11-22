/******************** (C) COPYRIGHT 2006 STMicroelectronics ********************
* File Name          : main.c
* Author             : MCD Application Team
* Date First Issued  : 05/18/2006 : Version 1.0
* Description        : Main program body
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
#include "91x_lib.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
  GPIO_InitTypeDef GPIO_InitStructure;

/* Private function prototypes -----------------------------------------------*/
  void SCU_Configuration(void);
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
  SCU_Configuration();

/* GPIO Configuration --------------------------------------------------------*/
  GPIO_DeInit(GPIO0);
  GPIO_DeInit(GPIO3);
  GPIO_DeInit(GPIO6);
  GPIO_DeInit(GPIO9);



  GPIO_InitStructure.GPIO_Direction = GPIO_PinOutput;

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_Init (GPIO0, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_Init (GPIO3, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_Init (GPIO6, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_Init (GPIO9, &GPIO_InitStructure);


  GPIO_InitStructure.GPIO_Type = GPIO_Type_PushPull ;


  while(1)
  {
    /* Turn OFF eds connected to P6.6 pins */   
    GPIO_WriteBit(GPIO6, GPIO_Pin_6, Bit_RESET);
    GPIO_WriteBit(GPIO0, GPIO_Pin_1, Bit_SET);
    GPIO_WriteBit(GPIO3, GPIO_Pin_7, Bit_SET);
    GPIO_WriteBit(GPIO9, GPIO_Pin_6, Bit_SET);
	    /* Insert delay */

	    Delay(0x7FFFF);

    /* Turn OFF leds connected to P9.6 pins */    
	GPIO_WriteBit(GPIO3, GPIO_Pin_7, Bit_RESET);
	GPIO_WriteBit(GPIO0, GPIO_Pin_1, Bit_SET);
    GPIO_WriteBit(GPIO6, GPIO_Pin_6, Bit_SET);
    GPIO_WriteBit(GPIO9, GPIO_Pin_6, Bit_SET);
	      /* Insert delay */

	    Delay(0x7FFFF);

   /*Turn OFF leds connected to P3.7 pins */

	GPIO_WriteBit(GPIO9, GPIO_Pin_6, Bit_RESET);
    GPIO_WriteBit(GPIO0, GPIO_Pin_1, Bit_SET);
    GPIO_WriteBit(GPIO3, GPIO_Pin_7, Bit_SET);
    GPIO_WriteBit(GPIO6, GPIO_Pin_6, Bit_SET);
	       /* Insert delay */

	    Delay(0x7FFFF);

    /* Turn OFF leds connected to P0.1 pins */
    GPIO_WriteBit(GPIO0, GPIO_Pin_1, Bit_RESET);
    GPIO_WriteBit(GPIO3, GPIO_Pin_7, Bit_SET);
    GPIO_WriteBit(GPIO6, GPIO_Pin_6, Bit_SET);
    GPIO_WriteBit(GPIO9, GPIO_Pin_6, Bit_SET);
	       /* Insert delay */

	    Delay(0x7FFFF);






  }
}

/*******************************************************************************
* Function Name  : SCU_Configuration
* Description    : Configures the system clocks.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SCU_Configuration(void)
{
  /* Enable the __GPIO9 */
  SCU_APBPeriphClockConfig(__GPIO0 ,ENABLE);
  SCU_APBPeriphClockConfig(__GPIO3 ,ENABLE);
  SCU_APBPeriphClockConfig(__GPIO6 ,ENABLE);
  SCU_APBPeriphClockConfig(__GPIO9 ,ENABLE);

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

