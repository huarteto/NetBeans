/******************** (C) COPYRIGHT 2006 STMicroelectronics ********************
* File Name          : can.c
* Author             : MCD Application Team
* Date First Issued  : 05/18/2006 : Version 1.0
* Description        : This file includes the CAN loop back driver of STR91x-EVAL.
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
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
  CAN_InitTypeDef     CAN_InitStructure;
  
  /* Buffer for receive messages */
  canmsg RxCanMsg;

  /* Used message object numbers */
  enum
  {
    CAN_TX_MSGOBJ = 0,
    CAN_RX_MSGOBJ = 1
  };

  /* Array of pre-defined transmit messages */
  canmsg TxCanMsg[3] = {
	{ CAN_STD_ID,      0x123, 4, { 0x01, 0x02, 0x04, 0x08 } },
	{ CAN_STD_ID,      0x321, 4, { 0xAA, 0x55, 0xAA, 0x55 } },
	{ CAN_EXT_ID, 0x12345678, 8, { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17 } }
  };

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : CANLoopBack_Init
* Description    : Initializes CAN loop back mode application.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CANLoopBack_Init(void)
{
  /* Reset the CAN registers values*/
  CAN_DeInit();

  /* CAN clock source enable */
  SCU_APBPeriphClockConfig(__CAN, ENABLE);
}

/*******************************************************************************
* Function Name  : CANLoopBack_Start
* Description    : Start CAN send/receive in loop back mode.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CANLoopBack_Start(void)
{
  /* Initialize the CAN at a standard bitrate, interrupts disabled */
  CAN_InitStructure.CAN_ConfigParameters=0x0;
  CAN_InitStructure.CAN_Bitrate=0x0;
  CAN_Init(&CAN_InitStructure);

  /* Set CAN clock */
  CAN_EnterInitMode(CAN_CR_CCE);
  CAN_SetTiming(4, 3, 1, 6);
  CAN_LeaveInitMode();

  /* Switch into Loopback+Silent mode (self-test) */
  CAN_EnterTestMode(CAN_TESTR_LBACK | CAN_TESTR_SILENT);

  /* Configure the message objects */
  CAN_SetUnusedAllMsgObj();
  CAN_SetTxMsgObj(CAN_TX_MSGOBJ, CAN_STD_ID, DISABLE);
  CAN_SetRxMsgObj(CAN_RX_MSGOBJ, CAN_STD_ID, 0, CAN_LAST_STD_ID, TRUE);

  /* Send the pre-defined answer */
  CAN_SendMessage(CAN_TX_MSGOBJ, &TxCanMsg[1]);

  /* Wait until end of transmission */
  CAN_WaitEndOfTx();


  /* Wait for reception of a data frame */
  while (!CAN_ReceiveMessage(CAN_RX_MSGOBJ, FALSE, &RxCanMsg));

  /* Test Received Msg */
  if((RxCanMsg.IdType == CAN_STD_ID)&&(RxCanMsg.Id == 0x321)&&(RxCanMsg.Dlc == 4)
    &&(RxCanMsg.Data[0]==0xAA)&&(RxCanMsg.Data[1]==0x55)&&(RxCanMsg.Data[2]==0xAA)&&(RxCanMsg.Data[3]==0x55))
  {
    /* Display "     SUCCESS     " message: data received */
    LCD_DisplayString(Line2, "SUCCESS: Press UP", BlackText);
  }
  else
  {
    /* Display "       FAIL      " message: data non received */
    LCD_DisplayString(Line2, " FAIL: Press UP  ", BlackText);
  }

  /* Release the message objects */
  CAN_ReleaseTxMessage(CAN_TX_MSGOBJ);
  CAN_ReleaseRxMessage(CAN_RX_MSGOBJ);

  /* Switch back into Normal mode */
  CAN_LeaveTestMode();
}

/******************* (C) COPYRIGHT 2006 STMicroelectronics *****END OF FILE****/
