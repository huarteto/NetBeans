/******************** (C) COPYRIGHT 2006 STMicroelectronics ********************
* File Name          : main.c
* Author             : MCD Application Team
* Date First Issued  : 05/18/2006 : Version 1.0
* Description        : Mouse Demo main program body
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
#include "USB_lib.h"
#include "USB_conf.h"
#include "USB_prop.h"
#include "USB_pwr.h"
#include "USB_mem.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* ujoystick direction */
#define DOWN	1
#define LEFT	2
#define RIGHT	3
#define UP      4

#define CURSOR_STEP     10
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
GPIO_InitTypeDef GPIO_InitStructure;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name : JoyState
* Description   : Decodes the Joystick direction
* Output        : None
* Return value  : the direction value
*******************************************************************************/
u8 JoyState(void)
{
  u8 port;
  port =GPIO_Read(GPIO7);
  if ((port&0xE0)==0x80) return(UP);
  if ((port&0xE0)==0x20) return(DOWN);
  if ((port&0xE0)==0x60) return(RIGHT);
  if ((port&0xE0)==0x40) return(LEFT);
  else return(0);
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
	case LEFT:
	  X -= CURSOR_STEP;
	  break;
	case RIGHT:

	  X += CURSOR_STEP;
	  break;
	case UP:
	  Y -= CURSOR_STEP;
	  break;
	case DOWN:
	  Y += CURSOR_STEP;
	  break;
	default:
	  return;
  }

  /* prepare buffer to send */
  Mouse_Buffer[1] = X;
  Mouse_Buffer[2] = Y;
  /*copy mouse position info in ENDP1 Tx Packet Memory Area*/
  UserToPMABufferCopy(&Mouse_Buffer[0],ENDP1_TXADDR,4);
  /* enable endpoint for transmission */
  SetEPTxValid(ENDP1);

} /* Mouse_Send */


int main()
{
  u32 i ;

  #ifdef DEBUG
  debug();
  #endif


  SCU_MCLKSourceConfig(SCU_MCLK_OSC);
  /*configure PLL factors for 48MHz*/
  SCU_PLLFactorsConfig(0xC0,0x19,0x3);
  /*Enable PLL*/
  SCU_PLLCmd(ENABLE);
  /*PLL as MCLK clock*/
  SCU_MCLKSourceConfig(SCU_MCLK_PLL);
  /*Enable VIC clock*/
  SCU_AHBPeriphClockConfig(__VIC,ENABLE);
  SCU_AHBPeriphReset(__VIC,DISABLE);
  /*USB clock = MCLK= 48MHz*/
  SCU_USBCLKConfig(SCU_USBCLK_MCLK);
  /*Enable USB clock*/
  SCU_AHBPeriphClockConfig(__USB,ENABLE);
  SCU_AHBPeriphReset(__USB,DISABLE);

  SetCNTR(0);
  SetISTR(0);
  SCU_AHBPeriphClockConfig(__USB48M,ENABLE);
  SCU_APBPeriphClockConfig(__GPIO7,ENABLE);
  SCU_APBPeriphReset(__GPIO7,DISABLE);


  /*Configure GPIO7*/
  GPIO_DeInit(GPIO7);
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Direction = GPIO_PinInput;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Type = GPIO_Type_PushPull ;
  GPIO_Init (GPIO7, &GPIO_InitStructure);



  /* Enable and configure the priority of the USB_LP IRQ Channel*/
  VIC_DeInit();
  VIC_Config(USBLP_ITLine, VIC_IRQ, 0);
  VIC_ITCmd(USBLP_ITLine, ENABLE);

  /* USB  initialization */
  wInterrupt_Mask = IMR_MSK;
  USB_Init();



  while(1)
  {
  	for(i=0;i<10000;i++);
  	if (JoyState()!=0)
  	Mouse_Send(JoyState());
  }

}
/******************* (C) COPYRIGHT 2006 STMicroelectronics *****END OF FILE****/
