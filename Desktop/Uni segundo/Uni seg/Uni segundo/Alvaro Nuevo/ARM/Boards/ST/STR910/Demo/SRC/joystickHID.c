/******************** (C) COPYRIGHT 2006 STMicroelectronics ********************
* File Name          : joystickHID.c
* Author             : MCD Application Team
* Date First Issued  : 05/18/2006 : Version 1.0
* Description        : This file includes the joystickHID driver for the
*                      STR91x-EVAL demonstration.
********************************************************************************
* History:
* 07/20/2006 : Version 1.2
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

#define CURSOR_STEP     10
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : JoyStickHID_Init
* Description    : Initializes JoyStick HID application.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void JoyStickHID_Init(void)
{
  wInterrupt_Mask = IMR_MSK;
}

/*******************************************************************************
* Function Name : Mouse_Send
* Description   : prepares buffer to be sent containing mouse event infos
* Input         : Keys: keys received from terminal
* Output        : None
* Return value  : None
*******************************************************************************/
void Mouse_Send(BYTE Keys)
{
  BYTE Mouse_Buffer[4] = {0,0,0,0};
  char X = 0, Y = 0;

  switch (Keys)
  {
	case Left:
	  X -= CURSOR_STEP;
	  break;
	case Right:

	  X += CURSOR_STEP;
	  break;
	case Up:
	  Y -= CURSOR_STEP;
	  break;
	case Down:
	  Y += CURSOR_STEP;
	  break;
	default:
	  return;
  }

  /* Prepare buffer to send */
  Mouse_Buffer[1] = X;
  Mouse_Buffer[2] = Y;
  /* Copy mouse position info in ENDP1 Tx Packet Memory Area */
  UserToPMABufferCopy(&Mouse_Buffer[0],ENDP1_TXADDR,4);
  /* Enable endpoint for transmission */
  SetEPTxValid(ENDP1);

} /* Mouse_Send */

/*******************************************************************************
* Function Name : JoyState
* Description   : Decodes the Joystick direction
* Output        : None
* Return value  : the direction value
*******************************************************************************/
u8 JoyState(void)
{
  u8 port;
  port = GPIO_Read(GPIO7);
  if ((port&0xE0) == 0x80) return(Up);
  if ((port&0xE0) == 0x20) return(Down);
  if ((port&0xE0) == 0x60) return(Right);
  if ((port&0xE0) == 0x40) return(Left);
  if ((port&0xE0) == 0x0) return(Sel);
  else return 0;
}

/*******************************************************************************
* Function Name  : JoyStickHID_Start
* Description    : Starts the joystick HID demo.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void JoyStickHID_Start (void)
{
  vu32 i = 0;
  /* Enable the USB Clock */
  SCU_AHBPeriphClockConfig(__USB, ENABLE);
  /* Disable the USB Reset mode */
  SCU_AHBPeriphReset(__USB, DISABLE);
  /* Enable the USB LP interrupt */
  VIC_ITCmd(USBLP_ITLine, ENABLE);
  /* Disable the EXTIT3 interrupt */
  VIC_ITCmd(EXTIT3_ITLine, DISABLE);
  /* Clear the LCD screen */
  LCD_Clear();
  /* Display the "  Plug the USB   " message */
  LCD_DisplayString(Line1, "  Plug the USB   ", BlackText);
  /* Display the "  cable          " message */
  LCD_DisplayString(Line2, "      cable      ", BlackText);
  /* Intialize the USB cell */
  USB_Init();
  /* Clear the LCD screen */
  LCD_Clear();
  /* Display the "Move the Joystick" message */
  LCD_DisplayString(Line1, "Move the Joystick", BlackText);
  /* Display the "To stop Press SEL" message */
  LCD_DisplayString(Line2, "To stop Press SEL", BlackText);

  /* Loop until SEL key pressed */
  while(JoyState()!= Sel)
  {
    for(i = 0; i<0x10000; i++);
    /* Move the cursor according the read Joystixk Key */
    Mouse_Send(JoyState());
  }

  /* Display the menu */
  DisplayMenu();
  /* Disable the USB LP interrupt */
  VIC_ITCmd(USBLP_ITLine, DISABLE);
  /* Disable the USB Clock */
  SCU_AHBPeriphClockConfig(__USB, DISABLE);
  /* Enable the USB Reset mode */
  SCU_AHBPeriphReset(__USB, ENABLE);
  /* Clear the WIU Line 28 pending bit */
  WIU_ClearITPendingBit(WIU_Line28);
  /* Enable the EXTIT3 interrupt */
  VIC_ITCmd(EXTIT3_ITLine, ENABLE);
}

/******************* (C) COPYRIGHT 2006 STMicroelectronics *****END OF FILE****/
