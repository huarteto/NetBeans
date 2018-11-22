/******************** (C) COPYRIGHT 2003 STMicroelectronics ********************
* File Name          : main.c
* Author             : MCD Application Team
* Date First Issued  : 16/05/2003
* Description        : GPIO library example
********************************************************************************
* History:
* 13/01/2006   : V3.1
* 24/05/2005   : V3.0
* 30/11/2004   : V2.0
* 16/05/2003   : Created
*******************************************************************************
 THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS WITH
 CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
 AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT
 OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
 OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
 CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

#include "71x_lib.h"


void delay(void)
{
  u16 i;
  for(i=0; i<0xFFFF; i++); /* delay */
}

int main(void)
{
 
  #ifdef DEBUG
  debug();
  #endif

/*   Configure P0.0 - P0.7 as Output Push-Pull */
  GPIO_Config(GPIO0, 0x00FF, GPIO_OUT_PP);

/*   Configure P0.7 - P0.15 as Input TTL */
  GPIO_Config(GPIO0, 0xFF00 , GPIO_IN_TRI_TTL);

/*   Configure P1.0 as Output Push-Pull */
  GPIO_Config(GPIO1, 0x0001, GPIO_OUT_PP);

/*   Configure P1.1 as Input TTL */
  GPIO_Config(GPIO1, 0x0001, GPIO_IN_TRI_TTL);

  GPIO_WordWrite(GPIO0, 0xFF00);

  /*  infinite loop */
  while(1)

  {

     delay();

     /*GPIO_ByteWrite(GPIO0, GPIO_LSB, GPIO_ByteRead(GPIO0, GPIO_MSB));*/ 

     /*GPIO_BitWrite(GPIO1, 0, GPIO_BitRead(GPIO1, 1));*/ 

     /*  Toggle the GPIO0 port pins */
       GPIO_WordWrite(GPIO0, ~GPIO_WordRead(GPIO0));
  }

}

/******************* (C) COPYRIGHT 2003 STMicroelectronics *****END OF FILE****/
