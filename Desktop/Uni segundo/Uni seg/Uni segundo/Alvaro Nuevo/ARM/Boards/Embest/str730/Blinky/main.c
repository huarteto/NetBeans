/******************** (C) COPYRIGHT 2005 STMicroelectronics ********************
* File Name          : main.c
* Author             : MCD Application Team
* Date First Issued  : 09/27/2005 :  V1.0
* Description        : Main program body
**********************************************************************************
* History:
* 09/27/2005 :  V1.0
**********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS WITH
* CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT
* OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
* OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
* CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*********************************************************************************/

/* Standard include ------------------------------------------------------------*/
#include "73x_lib.h"

/* Include of other module interface headers -----------------------------------*/
/* Local includes --------------------------------------------------------------*/
/* Private typedef -------------------------------------------------------------*/
/* Private define --------------------------------------------------------------*/
/* Private macro ---------------------------------------------------------------*/
/* Private variables -----------------------------------------------------------*/ 

/* Define GPIO Init Structure */
GPIO_InitTypeDef  GPIO2_InitStructure;

/* Functions Prototypes-------------------------------------------------------*/
void Delay(u32 Xtime);

/* Variables------------------------------------------------------------------*/

bool Direction = FALSE;

/*---------------------------------main---------------------------------------*/

int main(void)
{
  /* GPIO2 Clock Enable */
  CFG_PeripheralClockConfig(CFG_CLK_GPIO2 , ENABLE);            
  /* GPIO2 Configuration */ 
  GPIO2_InitStructure.GPIO_Mode = GPIO_Mode_OUT_PP;
  GPIO2_InitStructure.GPIO_Pins = GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_8 | GPIO_PIN_10 | GPIO_PIN_12 | GPIO_PIN_15;
  GPIO_Init (GPIO2, &GPIO2_InitStructure);

  /* GPIO2 Write */
  GPIO_BitWrite(GPIO2, GPIO_PIN_4, Bit_SET);
  Delay(0x2FFFF);
  GPIO_BitWrite(GPIO2, GPIO_PIN_6, Bit_SET);
  Delay(0x2FFFF);
  GPIO_BitWrite(GPIO2, GPIO_PIN_8, Bit_SET);
  Delay(0x2FFFF);
  GPIO_BitWrite(GPIO2, GPIO_PIN_10, Bit_SET);
  Delay(0x2FFFF);
  GPIO_BitWrite(GPIO2, GPIO_PIN_12, Bit_SET);
  Delay(0x2FFFF);						
  GPIO_BitWrite(GPIO2, GPIO_PIN_15, Bit_SET);
  Delay(0x2FFFF);

  /* Loop write on each GPIO2 pin */
  while (1)
  {
    if (! Direction)
    {
      GPIO_BitWrite(GPIO2, GPIO_PIN_4, Bit_RESET);
      GPIO_BitWrite(GPIO2, GPIO_PIN_6, Bit_SET);
      Delay(0x9FFF);
      GPIO_BitWrite(GPIO2, GPIO_PIN_6, Bit_RESET);
      GPIO_BitWrite(GPIO2, GPIO_PIN_8, Bit_SET);
      Delay(0x9FFF);
      GPIO_BitWrite(GPIO2, GPIO_PIN_8, Bit_RESET);
      GPIO_BitWrite(GPIO2, GPIO_PIN_10, Bit_SET);
      Delay(0x9FFF);
      GPIO_BitWrite(GPIO2, GPIO_PIN_10, Bit_RESET);
      GPIO_BitWrite(GPIO2, GPIO_PIN_12, Bit_SET);
      Delay(0x9FFF);
      GPIO_BitWrite(GPIO2, GPIO_PIN_12, Bit_RESET);
      GPIO_BitWrite(GPIO2, GPIO_PIN_15, Bit_SET);
      Delay(0x9FFF); 
      Direction = TRUE;
    }
    else
    {
      GPIO_BitWrite(GPIO2, GPIO_PIN_15, Bit_RESET);
      GPIO_BitWrite(GPIO2, GPIO_PIN_12, Bit_SET);
      Delay(0x9FFF);
      GPIO_BitWrite(GPIO2, GPIO_PIN_12, Bit_RESET);
      GPIO_BitWrite(GPIO2, GPIO_PIN_10, Bit_SET);
      Delay(0x9FFF);
      GPIO_BitWrite(GPIO2, GPIO_PIN_10, Bit_RESET);
      GPIO_BitWrite(GPIO2, GPIO_PIN_8, Bit_SET);
      Delay(0x9FFF);
      GPIO_BitWrite(GPIO2, GPIO_PIN_8, Bit_RESET);
      GPIO_BitWrite(GPIO2, GPIO_PIN_6, Bit_SET);
      Delay(0x9FFF);
      GPIO_BitWrite(GPIO2, GPIO_PIN_6, Bit_RESET);
      GPIO_BitWrite(GPIO2, GPIO_PIN_4, Bit_SET);
      Delay(0x9FFF);
      Direction = FALSE;
    }
  }
}

/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
Function name : void Delay(u32 Xtime)
Description   : Add a dealy
Input param   : u32 Xtime
Output param  : None
*******************************************************************************/
void Delay(u32 Xtime)
{
  u32 j;

  for(j=Xtime;j!=0;j--);
}


/******************* (C) COPYRIGHT 2005 STMicroelectronics *****END OF FILE****/
