/******************** (C) COPYRIGHT 2003 STMicroelectronics ********************
* File Name          : main.c
* Author             : MCD Application Team
* Date First Issued  : 16/05/2003
* Description        : This program demonstrates how to use the UART with
*                      interrupt.
********************************************************************************
* History:
* 13/01/06 : V3.1
* 24/05/05 : V3.0
* 30/11/04 : V2.0
* 16/05/03 : Created
*******************************************************************************
 THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS WITH
 CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
 AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT
 OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
 OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
 CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

#include "71x_lib.h"

#define UART0_Rx_Pin (0x0001<<8)   /*  TQFP 64: pin N° 63 , TQFP 144 pin N° 143 */
#define UART0_Tx_Pin (0x0001<<9)   /*  TQFP 64: pin N° 64 , TQFP 144 pin N° 144 */



int main(void)
{
  #ifdef DEBUG
  debug();
  #endif
  /* GPIO peripheral configuration -------------------------------------------*/

  /*  Configure the GPIO pins */
  GPIO_Config(GPIO0, UART0_Tx_Pin, GPIO_AF_PP);
  GPIO_Config(GPIO0, UART0_Rx_Pin, GPIO_IN_TRI_CMOS);

  /* UART peripheral configuration -------------------------------------------*/

  /*  Configure the UART X */
  /*  Turn UART0 on */
  UART_OnOffConfig(UART0, ENABLE); 
  /*  Disable FIFOs */      
  UART_FifoConfig (UART0, DISABLE);
  /*  Reset the UART_RxFIFO */      
  UART_FifoReset  (UART0 , UART_RxFIFO);
  /*  Reset the UART_TxFIFO */ 
  UART_FifoReset  (UART0 , UART_TxFIFO);
  /*  Disable Loop Back */ 
  UART_LoopBackConfig(UART0 , DISABLE);                                           
 /* Configure the UART0 as following:
                              - Baudrate = 9600 Bps
                              - No parity
                              - 8 data bits
                              - 1 stop bit */
  UART_Config(UART0,9600,UART_NO_PARITY,UART_1_StopBits,UARTM_8D);
  /*  Enable Rx */
  UART_RxConfig(UART0 ,ENABLE);
          
  /* EIC peripheral configuration --------------------------------------------*/

  /* Configure the EIC channel interrupt */
  EIC_IRQChannelPriorityConfig(UART0_IRQChannel, 1);
  EIC_IRQChannelConfig(UART0_IRQChannel, ENABLE);
  EIC_IRQConfig(ENABLE);

  UART_ItConfig(UART0,UART_RxBufFull, ENABLE);

  while(1);

}

/******************* (C) COPYRIGHT 2003 STMicroelectronics *****END OF FILE****/
