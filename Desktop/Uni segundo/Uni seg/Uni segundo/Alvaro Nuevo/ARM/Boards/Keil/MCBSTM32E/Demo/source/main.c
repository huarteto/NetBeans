/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : main.c
* Author             : MCD Application Team
* Version            : V1.1.1
* Date               : 06/13/2008
* Description        : Main program body
* Keil/ARM Update    : Added code to setup backlight control PA8 as push-pull to 
*                    ;   avoid flickering (EJH).
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_tim.h"
#include "stm32f10x_dac.h"
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
GPIO_InitTypeDef  GPIO_InitStructure;             //Added 08-18-2009
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static vu32 TimingDelay = 0;
static vu32 LedShowStatus = 0;
static volatile ErrorStatus HSEStartUpStatus = SUCCESS;
static vu32 SELStatus = 0;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void SetupBacklight (void) {								 // Added 08-18-2009

  /* Enable GPIOA clock                                                 */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		 // Added 08-18-2009

  /* Configure PA8 as outputs push-pull, max speed 50 MHz               */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8;				 // Added 08-18-2009
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;			 // Added 08-18-2009
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;			 // Added 08-18-2009
  GPIO_Init(GPIOA, &GPIO_InitStructure);					 // Added 08-18-2009
}
/*******************************************************************************
* Function Name  : main
* Description    : Main program.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int main(void)
{
#ifdef DEBUG
  debug();
#endif

  /* Initialize the Demo */
  Demo_Init();

  while (1)
  {
    /* If SEL pushbutton is pressed */
    if(SELStatus == 1)
    {
      /* External Interrupt Disable */
      IntExtOnOffConfig(DISABLE);

      /* Execute Sel Function */
      SelFunc();

      /* External Interrupt Enable */
      IntExtOnOffConfig(ENABLE);
      /* Reset SELStatus value */
      SELStatus = 0;
    } 
  }
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
  /* SYSCLK, HCLK, PCLK2 and PCLK1 configuration -----------------------------*/   
  /* RCC system reset(for debug purpose) */
  RCC_DeInit();

  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);

  /* Wait till HSE is ready */
  HSEStartUpStatus = RCC_WaitForHSEStartUp();

  if(HSEStartUpStatus == SUCCESS)
  {
    /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

    /* Flash 2 wait state */
    FLASH_SetLatency(FLASH_Latency_2);
    
    /* HCLK = SYSCLK */
    RCC_HCLKConfig(RCC_SYSCLK_Div1); 
  
    /* PCLK2 = HCLK */
    RCC_PCLK2Config(RCC_HCLK_Div1); 
 
    /* PCLK1 = HCLK/2 */
    RCC_PCLK1Config(RCC_HCLK_Div2);
 
    /* PLLCLK = 8MHz * 9 = 72 MHz */
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
 
    /* Enable PLL */ 
    RCC_PLLCmd(ENABLE);
 
    /* Wait till PLL is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    {
    }
 
    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
 
    /* Wait till PLL is used as system clock source */
    while(RCC_GetSYSCLKSource() != 0x08)
    {
    }
  }

  /* Enable GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG and AFIO clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |RCC_APB2Periph_GPIOC 
         | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG 
         | RCC_APB2Periph_AFIO, ENABLE);
  
  /* TIM1 Periph clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
 
/*------------------- Resources Initialization -----------------------------*/
  /* GPIO Configuration */
  GPIO_Config();

  /* Interrupt Configuration */
  InterruptConfig();

  /* Configure the systick */    
  SysTick_Config();

/*------------------- Drivers Initialization -------------------------------*/
  /* Initialize the LEDs toogling */
  LedShow_Init();

  /* Initialize the Low Power application */
  LowPower_Init();

  /* Set up PA8 for backlight control */
  SetupBacklight();								 // Added 08-18-2009

  /* Initialize the LCD */
  STM3210E_LCD_Init();

  /* Clear the LCD */ 
  LCD_Clear(White);

  /* Turn On LCD Backlight */
  GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_SET);     // Added 08-18-2009
  
  /* If HSE is not detected at program startup */
  if(HSEStartUpStatus == ERROR)
  {
    /* Generate NMI exception */
    NVIC_SetSystemHandlerPendingBit(SystemHandler_NMI);
  }  
   
#ifndef STM3210E
  /* Initialize the SPI FLASH driver */
  SPI_FLASH_Init();
#endif
 
  /* Checks the availability of the bitmap files */
  CheckBitmapFilesStatus();
  
  /* Display the STM32 introduction */
  STM32Intro();

  /* Clear the LCD */ 
  LCD_Clear(White);

  /* Initialize the Calendar */
  Calendar_Init();

  /* Enable Leds toggling */
  LedShow(ENABLE);
  
  /* Initialize the Low Power application*/ 
  LowPower_Init();

  /* Set the LCD Back Color */
  LCD_SetBackColor(Blue);

  /* Set the LCD Text Color */
  LCD_SetTextColor(White);
  
  /* Initialize the Menu */
  Menu_Init();

  /* Display the main menu icons */
  ShowMenuIcons();
}

/*******************************************************************************
* Function Name  : InterruptConfig
* Description    : Configures the used IRQ Channels and sets their priority.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void InterruptConfig(void)
{ 
  NVIC_InitTypeDef NVIC_InitStructure;
  EXTI_InitTypeDef EXTI_InitStructure;

  /* Deinitializes the NVIC */
  NVIC_DeInit();

  /* Set the Vector Table base address at 0x08000000 */
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x00);

  /* Configure the Priority Group to 2 bits */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

  /* Configure the SysTick handler priority */
  NVIC_SystemHandlerPriorityConfig(SystemHandler_SysTick, 0, 0);
  
  /* Enable the EXTI3 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable the EXTI9_5 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* Enable the EXTI15_10 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* Enable the RTC Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

 
  /* Enable the USB_LP_CAN_RX0 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN_RX0_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* Enable the USB_HP_CAN_TX Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USB_HP_CAN_TX_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable the TIM1 UP Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable the RTC Alarm Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = RTCAlarm_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable the EXTI Line17 Interrupt */
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Line = EXTI_Line17;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
}

/*******************************************************************************
* Function Name  : SysTick_Config
* Description    : Configure a SysTick Base time to 10 ms.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SysTick_Config(void)
{
  /* Configure HCLK clock as SysTick clock source */
  SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
 
  /* SysTick interrupt each 100 Hz with HCLK equal to 72MHz */
  SysTick_SetReload(720000);

  /* Enable the SysTick Interrupt */
  SysTick_ITConfig(ENABLE);
}

/*******************************************************************************
* Function Name  : IntExtOnOffConfig
* Description    : Enables or disables EXTI for the menu navigation keys :
*                  EXTI lines 3, 7 and 15 which correpond respectively
*                  to "DOWN", "SEL" and "UP".
* Input          : NewState: New state of the navigation keys. This parameter
*                  can be: ENABLE or DISABLE.
* Output         : None
* Return         : None
*******************************************************************************/
void IntExtOnOffConfig(FunctionalState NewState)
{
  EXTI_InitTypeDef EXTI_InitStructure;

  /* Initializes the EXTI_InitStructure */
  EXTI_StructInit(&EXTI_InitStructure);

  /* Disable the EXTI line 3, 7 and 15 on falling edge */
  if(NewState == DISABLE)
  {
    EXTI_InitStructure.EXTI_Line = EXTI_Line3 | EXTI_Line7 | EXTI_Line15;
    EXTI_InitStructure.EXTI_LineCmd = DISABLE;
    EXTI_Init(&EXTI_InitStructure);
  }
  /* Enable the EXTI line 3, 7 and 15 on falling edge */
  else
  {
    /* Clear the the EXTI line 3, 7 and 15 interrupt pending bit */
    EXTI_ClearITPendingBit(EXTI_Line3 | EXTI_Line7 | EXTI_Line15);

    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Line = EXTI_Line3 | EXTI_Line7 | EXTI_Line15;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
  }
}

/*******************************************************************************
* Function Name  : GPIO_Config
* Description    : Configures the different GPIO ports pins.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GPIO_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Configure PG.07, PG.08, PG.13, PG.14 and PG.15 as input floating */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOG, &GPIO_InitStructure);

  /* Configure PD.03 as input floating */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  /* RIGHT Button */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOG, GPIO_PinSource13);

  /* LEFT Button */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOG, GPIO_PinSource14);

  /* DOWN Button */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource3);

  /* UP Button */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOG, GPIO_PinSource15);

  /* SEL Button */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOG, GPIO_PinSource7);

  /* KEY Button */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOG, GPIO_PinSource8);
}

/*******************************************************************************
* Function Name  : Delay
* Description    : Inserts a delay time.
* Input          : nCount: specifies the delay time length (time base 10 ms).
* Output         : None
* Return         : None
*******************************************************************************/
void Delay(u32 nCount)
{
  TimingDelay = nCount;

  /* Enable the SysTick Counter */
  SysTick_CounterCmd(SysTick_Counter_Enable);
  
  while(TimingDelay != 0)
  {
  }

  /* Disable the SysTick Counter */
  SysTick_CounterCmd(SysTick_Counter_Disable);

  /* Clear the SysTick Counter */
  SysTick_CounterCmd(SysTick_Counter_Clear);
}

/*******************************************************************************
* Function Name  : DelayJoyStick
* Description    : Inserts a delay time while no joystick RIGHT, LEFT and SEL 
                   pushbuttons are pressed.
* Input          : nCount: specifies the delay time length (time base 10 ms).
* Output         : None
* Return         : Pressed Key.  This value can be: NOKEY, RIGHT, LEFT or SEL.
*******************************************************************************/
u32 DelayJoyStick(u32 nTime)
{
  vu32 keystate = 0;

  /* Enable the SysTick Counter */
  SysTick_CounterCmd(SysTick_Counter_Enable);

  TimingDelay = nTime;
 
  while((TimingDelay != 0) && (keystate != RIGHT) && (keystate != LEFT) && (keystate != SEL))
  {
    keystate = ReadKey();
  } 
  
  return keystate;
}

/*******************************************************************************
* Function Name  : Decrement_TimingDelay
* Description    : Decrements the TimingDelay variable.
* Input          : None
* Output         : TimingDelay
* Return         : None
*******************************************************************************/
void Decrement_TimingDelay(void)
{
  if (TimingDelay != 0x00)
  {
    TimingDelay--;
  }
}

/*******************************************************************************
* Function Name  : Set_SELStatus
* Description    : Sets the SELStatus variable.
* Input          : None
* Output         : SELStatus
* Return         : None
*******************************************************************************/
void Set_SELStatus(void)
{
  SELStatus = 1;
}

/*******************************************************************************
* Function Name  : LedShow_Init
* Description    : Configure the leds pins as output pushpull: LED1, LED2, LED3
*                  and LED4
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void LedShow_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;

  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  TIM_OCStructInit(&TIM_OCInitStructure);

#if STM3210E
  /* PF.06, PF.07, PF.08 and PF.09 as output push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOF, &GPIO_InitStructure);
#else
  /* PB8..PB13 as output push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8  | GPIO_Pin_9  | GPIO_Pin_10 | GPIO_Pin_11 |
                                GPIO_Pin_12 | GPIO_Pin_13 ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
#endif

  /* Time Base configuration */
  TIM_TimeBaseStructure.TIM_Prescaler = 719;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_Period = 0x270F;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0x0;

  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

  /* Channel 1 Configuration in Timing mode */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
  TIM_OCInitStructure.TIM_Pulse = 0x0;
  
  TIM_OC1Init(TIM1, &TIM_OCInitStructure); 
}

/*******************************************************************************
* Function Name  : LedShow
* Description    : Enables or disables LED1, LED2, LED3 and LED4 toggling.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void LedShow(FunctionalState NewState)
{
  /* Enable LEDs toggling */
  if(NewState == ENABLE)
  {
    LedShowStatus = 1;
    /* Enable the TIM1 Update Interrupt */
    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
    /* TIM1 counter enable */
    TIM_Cmd(TIM1, ENABLE);
  }
  /* Disable LEDs toggling */
  else
  {
    LedShowStatus = 0;
    /* Disable the TIM1 Update Interrupt */
    TIM_ITConfig(TIM1, TIM_IT_Update, DISABLE);
    /* TIM1 counter disable */
    TIM_Cmd(TIM1, DISABLE);
  }
}

/*******************************************************************************
* Function Name  : Get_LedShowStatus
* Description    : Get the LedShowStatus value.
* Input          : None
* Output         : None
* Return         : LedShowStatus Value.
*******************************************************************************/
u32 Get_LedShowStatus(void)
{
  return LedShowStatus;
}

/*******************************************************************************
* Function Name  : CheckBitmapFilesStatus
* Description    : Checks the bitmap files availability and display a warning 
*                  message if these files doesn't exit.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CheckBitmapFilesStatus(void)
{
  /* Checks if the Bitmap files are loaded */
  if(CheckBitmapFiles() != 0)
  {
    /* Set the LCD Back Color */
    LCD_SetBackColor(Blue);

    /* Set the LCD Text Color */
    LCD_SetTextColor(White);    
    LCD_DisplayStringLine(Line0, "      Warning       ");
    LCD_DisplayStringLine(Line1, "No loaded Bitmap    ");
    LCD_DisplayStringLine(Line2, "files. Demo can't be");
    LCD_DisplayStringLine(Line3, "executed.           ");
    LCD_DisplayStringLine(Line4, "Please be sure that ");
    LCD_DisplayStringLine(Line5, "all files are       ");
    LCD_DisplayStringLine(Line6, "correctly programmed");
    LCD_DisplayStringLine(Line7, "in the SPI FLASH and");
    LCD_DisplayStringLine(Line8, "restart the Demo.   ");
    LCD_DisplayStringLine(Line9, "                    ");
    
    /* Deinitializes the NVIC */
    NVIC_DeInit();
    
    /* Deinitializes the RCC */
    RCC_DeInit();
    
    /* Demo Can't Start */
    while(1)
    {
    }
  }
}

/*******************************************************************************
* Function Name  : Get_HSEStartUpStatus
* Description    : Returns the HSE StartUp Status.
* Input          : None
* Output         : None
* Return         : HSE StartUp Status.
*******************************************************************************/
ErrorStatus Get_HSEStartUpStatus(void)
{
  return (HSEStartUpStatus);
}

#ifdef  DEBUG
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(u8* file, u32 line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  LCD_Clear(White);
  LCD_DisplayStringLine(Line0, "   Assert Function  ");

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
