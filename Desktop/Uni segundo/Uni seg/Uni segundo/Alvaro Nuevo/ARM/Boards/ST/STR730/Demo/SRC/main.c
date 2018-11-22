/******************** (C) COPYRIGHT 2005 STMicroelectronics ********************
* File Name          : main.c
* Author             : MCD Application Team
* Date First Issued  : 09/27/2005 :  V1.0
* Description        : STR73x demonstration software main.
********************************************************************************
* History:
* 09/27/2005 :  V1.0
********************************************************************************
 THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS WITH
 CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
 AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT
 OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
 OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
 CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Standard include ----------------------------------------------------------*/
#include "main.h"

/* Include of other module interface headers ---------------------------------*/
/* Local includes ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define DemoStartUpmsg "  Welcome to      STR730 Demo  "
#define CFG_CLK_Peripherals0  0x79FFF1B6
#define CFG_CLK_Peripherals1  0x001D6000

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
GPIO_InitTypeDef  GPIO_InitStructure1;
TB_InitTypeDef TB1_InitStructure;

/* Private function prototypes -----------------------------------------------*/
/* Interface functions -------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : main
* Description    : main function, where program start. 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void main(void)
{

#ifdef DEBUG
  debug();
#endif
  
  /* initialize the demonstration application */
  Demo_Init();
  
  /* display the first menu item */ 
  DisplayMenu();

  /* start successive lighting of red leds connected to GPIO4 */
  LedShow(ENABLE);
  
  /* start successive lighting of bicolor leds connected to GPIO2 */
  BicolorShow(ENABLE);

  /* undless loop */
  while(TRUE);
}

/*******************************************************************************
* Function Name  : Demo_Init
* Description    : Initializes the demonstration application.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Demo_Init(void)
{
  /* enable peripherals clocks */
  CFG->PCGR0 |= CFG_CLK_Peripherals0;
  CFG->PCGR1 |= CFG_CLK_Peripherals1;

  /* set system clock(MCLK) to 32 MHz */
  SetClockTo32();

  /* set EXTCLK frequency to 31.25 KHz (8MHz/256) */
  PRCCU_SetExtClkDiv(256);
   
  /* GPIO4 configuration, used to drive red leds */
  GPIO_InitStructure1.GPIO_Mode = GPIO_Mode_OUT_PP; 
  GPIO_InitStructure1.GPIO_Pins = GPIO_PIN_ALL ;
  GPIO_Init(GPIO4, &GPIO_InitStructure1);
  
  /* NEXT, PREV, SEL and BACK push-butttons  configuration */
  GPIO_InitStructure1.GPIO_Mode = GPIO_Mode_IN_TRI_TTL;
  GPIO_InitStructure1.GPIO_Pins = GPIO_PIN_8 | GPIO_PIN_9 |GPIO_PIN_10 |GPIO_PIN_11;
  GPIO_Init(GPIO5, &GPIO_InitStructure1);

  /* enable IRQ output request to CPU */
  EIC_IRQCmd(ENABLE);

  /* initialize the demonstration menu */ 
  Menu_Init();

  /* initialize application system timer */
  SystemTimer_Init();

  /* initialize LCD driver */
  LCD_Init();
  
  /* display demo welcome screen */
  LCD_SendStringByStep(DemoStartUpmsg, 1, 1);
  
  /* store melodies data to the SPI EEPROM */  
  SaveMelodyToSPI();
  
  /* store alarm melody data to the I2C EEPROM */  
  SaveAlarmMelodyToI2C();
  
  /* enable menu navigation keys */
  IntExtOnOffConfig(ENABLE);
  
  /* initialize time application */ 
  Time_Init();
  
  /* initialize scren saver mode */
  ScreenSaver_Init();
}

/*******************************************************************************
* Function Name  : IntExtOnOffConfig
* Description    : Enables or disables the menu navigation keys.
* Input          : NewState: new state of the navigation keys. This parameter
*                  can be: ENABLE or DISABLE.
* Output         : None
* Return         : None
*******************************************************************************/
void IntExtOnOffConfig(FunctionalState NewState)
{
  if(NewState == DISABLE)
  {
    EIC_IRQChannelConfig(EXTIT06_IRQChannel, DISABLE);
    EIC_IRQChannelConfig(EXTIT07_IRQChannel, DISABLE);
    EIC_IRQChannelConfig(EXTIT08_IRQChannel, DISABLE);
    EIC_IRQChannelConfig(EXTIT09_IRQChannel, DISABLE);
  }
  else  
  {
    EIC_ExternalITTriggerConfig(EXTERNAL_IT6, EXTIT_TRIGGER_Falling);
    EIC_IRQChannelPriorityConfig(EXTIT06_IRQChannel, 1);
    EIC_IRQChannelConfig(EXTIT06_IRQChannel, ENABLE);

    EIC_ExternalITTriggerConfig(EXTERNAL_IT7, EXTIT_TRIGGER_Falling);
    EIC_IRQChannelPriorityConfig(EXTIT07_IRQChannel, 1);
    EIC_IRQChannelConfig(EXTIT07_IRQChannel, ENABLE );

    EIC_ExternalITTriggerConfig(EXTERNAL_IT8, EXTIT_TRIGGER_Falling);
    EIC_IRQChannelPriorityConfig(EXTIT08_IRQChannel, 1);
    EIC_IRQChannelConfig(EXTIT08_IRQChannel, ENABLE );

    EIC_ExternalITTriggerConfig(EXTERNAL_IT9, EXTIT_TRIGGER_Falling);
    EIC_IRQChannelPriorityConfig(EXTIT09_IRQChannel, 1);
    EIC_IRQChannelConfig(EXTIT09_IRQChannel, ENABLE );
  }
}

/*******************************************************************************
* Function Name  : SystemTimer_Init
* Description    : Implements a system timer that generates an event each 125
*                  microsecondes(µs). This is used by:
*                     - LCD_Wait() function to define the LCD timing.
*                     - PeriodPulseMeasure() function to define the time between 
*                       two consecutive measure. 
*                     - Temperature_Display() function to define the refresh of 
*                       the temperature value display.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SystemTimer_Init(void) 
{
/* configure TB1 to generate an interrupt each 125µs -------------------------*/
  TB1_InitStructure.TB_CLK_Source = TB_CLK_EXTERNAL; /* EXTCLK := 31.25 KHz */ 
  TB1_InitStructure.TB_Prescaler = 0x3; /* TB1 clocked at 7812Hz (31.25KHz / 4)*/
  TB1_InitStructure.TB_Preload =0 ;  /* TB1 period ~125µs */
  TB_Init(TB1, &TB1_InitStructure);

  /* enable TB1 End of Count(EC) interrupt */
  TB_ITConfig(TB1, ENABLE);

  /* enable TB1 interrupt in EIC controller */  
  EIC_IRQChannelConfig(TB1_IRQChannel, ENABLE);
  EIC_IRQChannelPriorityConfig(TB1_IRQChannel, 3);

  /* start TB1 counting */
  TB_Cmd(TB1, ENABLE);
}

/******************* (C) COPYRIGHT 2005 STMicroelectronics *****END OF FILE****/
