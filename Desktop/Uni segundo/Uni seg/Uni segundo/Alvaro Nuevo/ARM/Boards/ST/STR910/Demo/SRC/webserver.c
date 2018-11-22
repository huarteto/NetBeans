/******************** (C) COPYRIGHT 2006 STMicroelectronics ********************
* File Name          : webserver.c
* Author             : MCD Application Team
* Date First Issued  : 07/24/2006 Version 1.0
* Description        : This file includes the webserver demo functions
********************************************************************************
* History:
* 07/24/2006 : Version 1.0
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

extern void uIPMain(void);

/*******************************************************************************
* Function Name  : Webserver_Init
* Description    : Webserver demo intialization
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Webserver_Init(void)
{
  ENET_InitClocksGPIO();                     /* configure all GPIO */
  ENET_Init(PHY_FULLDUPLEX_100M);            /* Operating mode = Fullduplex 100Mbs */
}


/*******************************************************************************
* Function Name  : Webserver_Start
* Description    : Webserver demo start
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Webserver_Start(void)
{
  /* Disable the EXTIT3 interrupt */
  VIC_ITCmd(EXTIT3_ITLine, DISABLE);
  /* start Receive & Transmit */
  ENET_Start();
  LCD_Clear();
  LCD_DisplayString(Line1, "Webserver started", BlackText);
  LCD_DisplayString(Line2, "Press UP to exit ", BlackText);
  /* uIP stack main loop */
  uIPMain();

  /* Display the menu */
  DisplayMenu();
  /* Clear the WIU Line 28 pending bit */
  WIU_ClearITPendingBit(WIU_Line28);
  /* Enable the EXTIT3 interrupt */
  VIC_ITCmd(EXTIT3_ITLine, ENABLE);
}
