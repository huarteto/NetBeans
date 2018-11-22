/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : mass_storage.c
* Author             : MCD Application Team
* Version            : V1.1.1
* Date               : 06/13/2008
* Description        : This file provides a set of functions needed to manage the
*                      communication between the STM32F10x USB and the SD Card 
*                      and NAND Flash .
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_lib.h"
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_istr.h"
#include "sdcard.h"
#include "fsmc_nand.h"
#include "nand_if.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : Mass_Storage_Init
* Description    : Initializes the peripherals used by the mass storage driver.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Mass_Storage_Init(void)
{
  /* Disable the Pull-Up*/
  USB_Cable_Config(DISABLE);

}

/*******************************************************************************
* Function Name  : Mass_Storage_Start
* Description    : Starts the mass storage demo.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Mass_Storage_Start (void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Disble the JoyStick interrupts */
  IntExtOnOffConfig(DISABLE);


  while(ReadKey() != NOKEY)
  {
  }

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
  /* Clear the LCD screen */
  LCD_Clear(White);

  LCD_SetDisplayWindow(160, 223, 128, 128);
 
#if STM3210E
  LCD_NORDisplay(USB_ICON);
#else
  LCD_SPIDisplay(USB_ICON);
#endif
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
  /* Disable LCD Window mode */
  LCD_WindowModeDisable();
  
  /* Set the Back Color */
  LCD_SetBackColor(Blue);
  /* Set the Text Color */
  LCD_SetTextColor(White); 

  
  /* Display the "  Plug the USB   " message */
  LCD_DisplayStringLine(Line8, " Plug the USB Cable ");
  LCD_DisplayStringLine(Line9, "Exit:  Push JoyStick");
    
  /* Enable and GPIOD clock */
  USB_Disconnect_Config();
  
  /* MAL configuration */
  MAL_Config();

  Set_USBClock();

  NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN_RX0_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  NVIC_InitStructure.NVIC_IRQChannel = USB_HP_CAN_TX_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  USB_Init();

  while (bDeviceState != CONFIGURED)
  {
    if(ReadKey() != NOKEY)
    {
      PowerOff();
      LCD_Clear(White);
      DisplayMenu();
      IntExtOnOffConfig(ENABLE);
      return;
    }
  }

  LCD_ClearLine(Line9);
  /* Display the "To stop Press SEL" message */
  LCD_DisplayStringLine(Line8, "  To stop Press SEL ");

  /* Loop until SEL key pressed */
  while(ReadKey() != SEL)
  {
  }

  PowerOff();
  
  LCD_Clear(White);
  DisplayMenu();
  IntExtOnOffConfig(ENABLE);
}

/*******************************************************************************
* Function Name  : Mass_Storage_Recovery
* Description    : Erases the NAND Flash Content.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Mass_Storage_Recovery (void)
{
  /* Disble the JoyStick interrupts */
  IntExtOnOffConfig(DISABLE);


  while(ReadKey() != NOKEY)
  {
  }

  LCD_Clear(White);

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
  
  /* Set the Back Color */
  LCD_SetBackColor(Blue);
  /* Set the Text Color */
  LCD_SetTextColor(White); 

  LCD_DisplayStringLine(Line4, " Erase NAND Content ");
  LCD_DisplayStringLine(Line5, "Please wait...      ");
 
  /* FSMC Initialization */
  FSMC_NAND_Init();

  NAND_Format();

  /* Display the "To stop Press SEL" message */
  LCD_DisplayStringLine(Line4, "     NAND Erased    ");
  LCD_DisplayStringLine(Line5, "  To exit Press SEL ");

  /* Loop until SEL key pressed */
  while(ReadKey() != SEL)
  {
  }
  
  LCD_Clear(White);
  DisplayMenu();
  IntExtOnOffConfig(ENABLE);
}

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
