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

/*   Configure P0.1 and P0.4 as Output Push-Pull */
  GPIO_Config(GPIO0, 0x0012, GPIO_OUT_PP);

/*   Configure P1.4 and p1.6 as Output Push-Pull */
  GPIO_Config(GPIO1, 0x0050, GPIO_OUT_PP);


/*turn off all the leds */
  GPIO_WordWrite(GPIO1, 0x0);

   GPIO_WordWrite(GPIO0, 0x0);

 /*  infinite loop led switching */ 
  while(1)

  {
     
     GPIO_BitWrite(GPIO1, 0x4, ~GPIO_BitRead(GPIO1, 0x4));

     delay();
	 
     GPIO_BitWrite(GPIO1, 0x4, ~GPIO_BitRead(GPIO1, 0x4));

     delay();
	      
     GPIO_BitWrite(GPIO1, 0x6, ~GPIO_BitRead(GPIO1, 0x6)); 

     delay();
	 
	 GPIO_BitWrite(GPIO1, 0x6, ~GPIO_BitRead(GPIO1, 0x6)); 

     delay();

	 GPIO_BitWrite(GPIO0, 0x1, ~GPIO_BitRead(GPIO0, 0x1)); 

	 delay();

	 GPIO_BitWrite(GPIO0, 0x1, ~GPIO_BitRead(GPIO0, 0x1)); 

	 delay();

  	 GPIO_BitWrite(GPIO0, 0x4, ~GPIO_BitRead(GPIO0, 0x4)); 

	 delay();

	 GPIO_BitWrite(GPIO0, 0x4, ~GPIO_BitRead(GPIO0, 0x4)); 

	 delay();
  
  
  
  
  }

}

/******************* (C) COPYRIGHT 2003 STMicroelectronics *****END OF FILE****/
