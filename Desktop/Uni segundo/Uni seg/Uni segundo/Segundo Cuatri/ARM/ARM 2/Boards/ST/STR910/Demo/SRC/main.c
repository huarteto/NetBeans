/******************** (C) COPYRIGHT 2006 STMicroelectronics ********************
* File Name          : main.c
* Author             : MCD Application Team
* Date First Issued  : 05/18/2006 : Version 1.0
* Description        : Main program body
********************************************************************************
* History:
* 07/20/2006 : version 1.2
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


/* Standard include ----------------------------------------------------------*/
#include "main.h"

/* Include of other module interface headers ---------------------------------*/
/* Local includes ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* ST Logo to display on all 122*32 dot matrix */
  u8 STLogo122[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8,
  0xFC, 0xFC, 0xFC, 0xFC, 0x7C, 0x7C, 0x7C, 0x7C, 0x7C, 0x7C, 0x3C, 0x3C, 0x3C,
  0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C,
  0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C,
  0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C,
  0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C,
  0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x1C, 0x0C,
  0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF,
  0xFF, 0xFF, 0xFF, 0xE7, 0xC1, 0xC1, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x18,
  0x3C, 0x3C, 0x3C, 0x7C, 0x7C, 0x7C, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC,
  0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC,
  0xFC, 0xFC, 0xFC, 0xFC, 0x7C, 0x3C, 0x1C, 0x0C, 0x04, 0x00, 0x00, 0x00, 0x00,
  0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC,
  0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0x7C, 0x3C, 0x1C, 0x0C, 0x04, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x40, 0x60, 0x70, 0x78, 0x7C, 0x7E, 0x7F, 0x7F, 0x7F, 0x7F,
  0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
  0x7F, 0x7F, 0x7E, 0x7E, 0x7C, 0x7C, 0x78, 0x78, 0x70, 0x70, 0x60, 0x60, 0x00,
  0x00, 0x01, 0x01, 0x01, 0x03, 0x03, 0x87, 0x87, 0xCF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01, 0x00, 0x00, 0x80, 0xC0, 0xE0,
  0xF0, 0xF8, 0xFC, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x81, 0xC0, 0xC0, 0xC0, 0x80,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x08, 0x08, 0x08, 0x0C, 0x0C, 0x0C, 0x0E, 0x0E, 0x0E, 0x0F, 0x0F, 0x0F, 0x0F,
  0x0F, 0x0F, 0x07, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x0C,
  0x0E, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
  0x0F, 0x0F, 0x0F, 0x07, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x1F,
  0x3F, 0x60, 0x7A, 0x65, 0x3F, 0x1F, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  /* ST Logo to display on master LCD 61*32 dot matrix */
  u8 STLogo61[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xF0,
  0xFC, 0x7C, 0x7C, 0x7C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C,
  0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C,
  0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x1C, 0x0C,
  0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x80, 0xC0, 0xF0, 0xFC, 0xFF, 0xFF, 0xC1, 0x80, 0x00, 0x00, 0x18,
  0x3C, 0x3C, 0x7C, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC,
  0xFC, 0x7C, 0x1C, 0x04, 0x00, 0x00, 0x80, 0xE0, 0xF8, 0xFC, 0xFC, 0xFC, 0xFC,
  0xFC, 0x7C, 0x1C, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x78, 0x7E, 0x7F, 0x7F,
  0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7E, 0x7C, 0x78, 0x70, 0x60, 0x40,
  0x01, 0x03, 0x07, 0x0F, 0xFF, 0xFF, 0x7F, 0x1F, 0x07, 0x01, 0x00, 0x80, 0xE0,
  0xF8, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x1F, 0x07, 0x01, 0x00, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x0C, 0x0C, 0x0E, 0x0F,
  0x07, 0x01, 0x00, 0x00, 0x00, 0x08, 0x0E, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
  0x07, 0x01, 0x00, 0x00, 0x00, 0x02, 0x07, 0x0F, 0x07, 0x02, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  GPIO_InitTypeDef GPIO_InitStructure;
  WIU_InitTypeDef WIU_InitStructure;
  bool LedShowStatus = FALSE;   /* Led show global Status */
  bool IntroMenu = FALSE;
  vu16 PlayIntroDelay = 20;     /* Play Intro delay: 2sec */
  extern  vu32 index;

/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : main
* Description    : Main program
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void main()
{

#ifdef DEBUG
    debug();
#endif

  /* Initialize the demonstration application */
  Demo_Init();

  /* Enable Led toggling during all the demonstration application */
  LedShow(ENABLE);

  /* Play the STR91x Intro: ST Logo and Osiris song */
  PlayIntro();

  /* Display the first menu item */
  DisplayMenu();

  /* Enable the Alarm */
  RTC_AlarmCmd(ENABLE);

  while(1);
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
  /*-------------------- System Clock Config --------------------*/
  FMI_BankRemapConfig(4, 2, 0, 0x80000); /* Set Flash banks size & address */
  FMI_Config(FMI_READ_WAIT_STATE_2, FMI_WRITE_WAIT_STATE_0, FMI_PWD_ENABLE,\
             FMI_LVD_ENABLE, FMI_FREQ_HIGH); /* FMI Waite States */

  /* Set the PCLK Clock to MCLK/2 */
  SCU_PCLKDivisorConfig(SCU_PCLK_Div2);
  /* USB clock = MCLK/2 = 48MHz */
  SCU_USBCLKConfig(SCU_USBCLK_MCLK2);

  /* Set the FMI Clock to MCLK/2 */
  //SCU_FMICLKDivisorConfig(SCU_FMICLK_Div2);

  SCU_PLLFactorsConfig(192, 25, 2); /* Configure Factors FPLL = 96MHz */
  SCU_PLLCmd(ENABLE);
  SCU_MCLKSourceConfig(SCU_MCLK_PLL);

  /* Enable USB clock */
  SCU_AHBPeriphClockConfig(__USB, ENABLE);
  SCU_AHBPeriphReset(__USB, DISABLE);

  SetCNTR(0);
  SetISTR(0);
  SCU_AHBPeriphClockConfig(__USB48M, ENABLE);

  /*------------------ Peripheral Clock Enable ------------------*/

  SCU_APBPeriphClockConfig(__TIM01, ENABLE);
  SCU_APBPeriphReset(__TIM01, DISABLE);

  SCU_APBPeriphClockConfig(__TIM23, ENABLE);
  SCU_APBPeriphReset(__TIM23, DISABLE);

  /* GPIO4 clock source enable */
  SCU_APBPeriphClockConfig(__GPIO4, ENABLE);
  SCU_APBPeriphReset(__GPIO4, DISABLE);

  /* GPIO8 clock source enable */
  SCU_APBPeriphClockConfig(__GPIO8, ENABLE);
  GPIO_DeInit(GPIO8);

  /* GPIO7 clock source enable */
  SCU_APBPeriphClockConfig(__GPIO7, ENABLE);
  GPIO_DeInit(GPIO7);

  /* GPIO 9 clock source enable */
  SCU_APBPeriphClockConfig(__GPIO9, ENABLE);
  GPIO_DeInit(GPIO9);

  /* Enable VIC clock */
  SCU_AHBPeriphClockConfig(__VIC, ENABLE);
  SCU_AHBPeriphReset(__VIC, DISABLE);

  /* Enable WIU clock */
  SCU_APBPeriphClockConfig(__WIU, ENABLE);
  SCU_APBPeriphReset(__WIU, DISABLE);

  /* Enable RTC Clock */
  SCU_APBPeriphClockConfig(__RTC,ENABLE);
  /* Disable RTC Reset */
  SCU_APBPeriphReset(__RTC,DISABLE);

  /* Configure GPIO7 */
  GPIO_DeInit(GPIO7);
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Direction = GPIO_PinInput;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Type = GPIO_Type_PushPull ;
  GPIO_Init (GPIO7, &GPIO_InitStructure);

  /*------------------- Drivers Initialization ------------------*/
  /* Initialize LCD driver */
  LCD_Init();
  /* Initialize the demonstration menu */
  Menu_Init();
  /* Initialize the USB */
  JoyStickHID_Init();
  /* Initialize Calendar driver */
  Calendar_Init();
  /* Initialize CAN */
  CANLoopBack_Init();
  /* Initialize wave player driver */
  WavePlayer_Init();
  /* Initialize the SPI FLASH driver */
  SystemTimer_Init();
  /* Initialize the LEDs pins */
  LedShow_Init();

  /* Initialize Webserver demo */
  Webserver_Init();

  WIU_ClearITPendingBit(WIU_Line28);
  WIU_InitStructure.WIU_Line = WIU_Line28;
  WIU_InitStructure.WIU_TriggerEdge = WIU_FallingEdge;
  WIU_Init(&WIU_InitStructure);
  WIU_Cmd(ENABLE);

  VIC_DeInit();
  /* VIC0 configuration */
  VIC_Config(TIM2_ITLine, VIC_IRQ, 0);
  VIC_ITCmd(TIM2_ITLine, ENABLE);
  VIC_Config(TIM0_ITLine, VIC_IRQ, 1);
  VIC_ITCmd(TIM0_ITLine, ENABLE);
  VIC_Config(RTC_ITLine, VIC_IRQ, 2);
  VIC_ITCmd(RTC_ITLine, ENABLE);
  VIC_Config(USBLP_ITLine, VIC_IRQ, 3);

  /* VIC1 configuration */
  VIC_Config(EXTIT3_ITLine, VIC_IRQ, 0);

  SCU_WakeUpLineConfig(28);
}

/*******************************************************************************
* Function Name  : SystemTimer_Init
* Description    : Implements a system timer that generates an event each 100
*                  millisecondes (100ms).
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SystemTimer_Init(void)
{
  TIM_InitTypeDef      TIM_InitStructure;

  TIM_StructInit(&TIM_InitStructure);

  TIM_InitStructure.TIM_Mode = TIM_OCM_CHANNEL_1;
  TIM_InitStructure.TIM_OC1_Modes = TIM_TIMING;
  TIM_InitStructure.TIM_Clock_Source = TIM_CLK_APB;
  TIM_InitStructure.TIM_Prescaler = 0xFF;
  TIM_InitStructure.TIM_Pulse_Length_1 = 0x493E;

  /* Initialize the Timer 1 */
  TIM_Init (TIM2, &TIM_InitStructure);
  /* Enable the Timer Overflow interrupt */
  TIM_ITConfig(TIM2, TIM_IT_OC1, ENABLE);
  /* Start the Timer counter */
  TIM_CounterCmd(TIM2, TIM_START);
}

/*******************************************************************************
* Function Name  : LedShow_Init
* Description    : Configure the leds pins as output pushpull: LED2, LED3, LED4
*                  and LED5
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void LedShow_Init(void)
{
  /* GPIO9 pin configuration */
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Direction = GPIO_PinOutput;
  GPIO_InitStructure.GPIO_Type = GPIO_Type_PushPull;
  GPIO_Init(GPIO9, &GPIO_InitStructure);
}

/*******************************************************************************
* Function Name  : LedShow
* Description    : Enables or disables LED2, LED3, LED4 and LED5 toggling
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void LedShow(FunctionalState NewState)
{
  /* Enable LEDs toggling */
  if(NewState == ENABLE)
  {
    LedShowStatus = TRUE;
  }
  /* Disable LEDs toggling */
  else
  {
    LedShowStatus = FALSE;
  }
}

/*******************************************************************************
* Function Name  : PlayIntro
* Description    : Draw ST Logo on all LCD and play Osiris wave song in
*                  demonstartion startup
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PlayIntro(void)
{
  /* Start playing Osiris wave */
  WavePlayer_Start();
  /* Initialize */
  PlayIntroDelay = 20;
  /* Clear entire display */
  LCD_Clear();
  /* Display ST Logo */
  LCD_DrawGraphic(STLogo122);
  /* Wait 2sec */
  while(PlayIntroDelay != 0);
  /* Reinitialize PlayIntroDelay value */
  PlayIntroDelay = 20;
  /* Clear entire display */
  LCD_Clear();
  /* Display "  Welcome to the " message */
  LCD_DisplayString(Line1, "  Welcome to the ", BlackText);
  /* Display "   STR91x Demo   " message */
  LCD_DisplayString(Line2, "STR91x Demo V 1.2", BlackText);
  /* Wait 2sec */
  while(PlayIntroDelay != 0);

  /* Reinitialize PlayIntroDelay value */
  PlayIntroDelay = 20;
  /* Clear entire display */
  LCD_Clear();
  /* Display "To Skip" message */
  LCD_DisplayString(Line1, "         To Skip:", BlackText);
  /* Display "Press SEL" message */
  LCD_DisplayString(Line2, "         Push SEL", BlackText);
  /* Display ST Logo on 32*61 pixels */
  LCD_DrawMasterGraphic(STLogo61);

  /* Wait any press on joystick pushbuttons to skip Intro */
  /* Or wait the end of Intro playing: no key was pressed */
  while((JoyState() != SEL) && (index < 424570));

  /* If all Osiris wave was played and stopped don't stop it again */
  if(index < 424574)
  {
    /* Stop playing Osiris wave */
    WavePlayer_Stop();
  }
  IntroMenu = TRUE;
  /* Enable the EXTIT3 interrupt */
  VIC_ITCmd(EXTIT3_ITLine, ENABLE);

}

/******************* (C) COPYRIGHT 2006 STMicroelectronics *****END OF FILE****/
