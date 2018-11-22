/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : waveplayer.c
* Author             : MCD Application Team
* Version            : V1.1.1
* Date               : 06/13/2008
* Description        : Wave Player driver source file.
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

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define REPLAY  3

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
u8 DemoTitle[20] = "STM32 I2S Codec Demo"; 

u8 CmdTitle0[20] = "  Control Buttons:  "; 

u8 CmdTitle1Playing[20] = "KEY>Pause  UP  >Vol+";

u8 CmdTitle2Playing[20] = "SEL>Stop   DOWN>Vol-";

u8 CmdTitle1Paused[20] =  "KEY>Play   UP  >Spkr";

u8 CmdTitle2Paused[20] =  "SEL>Stop   DOWN>Head";

u8 CmdTitle1Stopped[20] = "    UP > Speaker    "; 

u8 CmdTitle2Stopped[20] = "  DOWN > Headphone  "; 

u8 StatusTitleStopped[20] = "      Stopped       ";

u8 StatusTitlePlaying[20] = "      Playing       ";

u8 StatusTitlePaused[20] = "       Paused       ";

u8 i2cerr[20] =  "ERROR:I2C com. ->RST";
u8 memerr[20] =  "ERROR: Memory  ->RST";
u8 fileerr[20] = "ERROR: No Wave File ";

static u8 previoustmp = 50;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : WavePlayer_StartSpeaker
* Description    : Starts the wave player application.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void WavePlayer_StartSpeaker(void)
{
  u8 MyKey = 0;
  u32 err = 0, Counter = 0x0;

  LCD_Clear(White);

  /* Disable the JoyStick interrupts */
  IntExtOnOffConfig(DISABLE);

  while(ReadKey() != NOKEY)
  {
  }  

  /* Display the welcome screen and the commands */
  LCD_Update(ALL);

#if STM3210E
  /* Choose number of repetitions: 0 => infinite repetitions */
  I2S_CODEC_ReplayConfig(0);
  I2S_CODEC_Init(OutputDevice_SPEAKER, AUDIO_FILE_ADDRESS);
#else
  DAC_CODEC_Init(AUDIO_FILE_ADDRESS);
#endif

  /* Endless loop */
  while(1)
  {
    /* Check which key is pressed */
    MyKey = ReadKey();
    
    if(Counter == 0)
    { /* Mask All Interrupts */
      NVIC_SETPRIMASK();
      /* Update the displayed progression information */
      LCD_Update(PROGRESS);
      Counter = 0x5FFFF;
      /* Disable mask of all interrupts */
      NVIC_RESETPRIMASK();
    }
    Counter--;
    /* If "UP" pushbutton is pressed */
    if(MyKey == UP)
    {
      /* Mask All Interrupts */
      NVIC_SETPRIMASK();
      /* Check if the Codec is PLAYING audio file */
      if (GetVar_AudioPlayStatus() == AudioPlayStatus_PLAYING)
      {
#if STM3210E
        I2S_CODEC_ControlVolume(VolumeDirection_HIGH, VOLStep);
#else
        DAC_CODEC_ControlVolume(VolumeDirection_HIGH, VOLStep);
#endif

        /* Update the display information */
        LCD_Update(VOL);
      }
      /* UP bottomn pushed in PAUSE mode => Enable the Speaker device output ---*/
      else
      {
        /* Update the display information */
        LCD_Update(PLAY);

#if STM3210E
        /* Configure the Speaker as output and reinitialize all devices */
        err = I2S_CODEC_SpeakerHeadphoneSwap(OutputDevice_SPEAKER, AUDIO_FILE_ADDRESS);
#else
#endif
  
        /* Error message display if failure */
        if (err != 0)
        {
          LCD_DisplayError(err);

          /* Enable the FSMC that share a pin w/ I2C1 (LBAR) */
          RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

          /* Clear the LCD */
          LCD_Clear(White);

          /* Display the previous menu */
          DisplayMenu();

          /* Disable mask of all interrupts */
          NVIC_RESETPRIMASK();

          /* Enable the JoyStick interrupts */
          IntExtOnOffConfig(ENABLE); 
          return; 
        }      
      } 
      /* Disable mask of all interrupts */
      NVIC_RESETPRIMASK();
    }

    /* If "DOWN" pushbutton is pressed */
    if(MyKey == DOWN)
    {
      /* Mask All Interrupts */
      NVIC_SETPRIMASK();

      /* If the Codec is PLAYING => Decrease Volume*/
      if (GetVar_AudioPlayStatus() == AudioPlayStatus_PLAYING)
      {
#if STM3210E
        /* Increase the audio codec digital volume */
        I2S_CODEC_ControlVolume(VolumeDirection_LOW, VOLStep);
#else
        DAC_CODEC_ControlVolume(VolumeDirection_LOW, VOLStep);
#endif

        /* Update the LCD display */ 
        LCD_Update(VOL); 
      }
      else /* If the Codec is PAUSED => Headphone Enable */
      {
        /* Update the LCD display */ 
        LCD_Update(PLAY);
      
#if STM3210E
        /* Enable the Headphone output and reinitialize all devices */ 
        err = I2S_CODEC_SpeakerHeadphoneSwap(OutputDevice_HEADPHONE, AUDIO_FILE_ADDRESS);
#endif

        /* Error message display if failure */
        if (err != 0)
        {
          LCD_DisplayError(err);

          /* Enable the FSMC that share a pin w/ I2C1 (LBAR) */
          RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

          /* Clear the LCD */
          LCD_Clear(White);

          /* Display the previous menu */
          DisplayMenu();

          /* Disable mask of all interrupts */
          NVIC_RESETPRIMASK();

          /* Enable the JoyStick interrupts */
          IntExtOnOffConfig(ENABLE); 
          return; 
        }  
      }
      /* Disable mask of all interrupts */
      NVIC_RESETPRIMASK();
    }

    /* If "RIGHT" pushbutton is pressed */
    if(MyKey == RIGHT)
    {
      /* Mask All Interrupts */
      NVIC_SETPRIMASK();

      /* Check if the Codec is PLAYING audio file */
      if (GetVar_AudioPlayStatus() == AudioPlayStatus_PLAYING)
      {
#if STM3210E
        I2S_CODEC_ForwardPlay(STEP_FORWARD); 
#else
        DAC_CODEC_ForwardPlay(STEP_FORWARD); 
#endif
        /* Update the display information */
        LCD_Update(FRWD); 
      }
      /* Disable mask of all interrupts */
      NVIC_RESETPRIMASK(); 
    }
    /* If "LEFT" pushbutton is pressed */
    if(MyKey == LEFT)
    {
      /* Mask All Interrupts */
      NVIC_SETPRIMASK();

      /* Check if the Codec is PLAYING audio file */
      if (GetVar_AudioPlayStatus() == AudioPlayStatus_PLAYING)
      {
#if STM3210E
        I2S_CODEC_RewindPlay(STEP_BACK);
#else
        DAC_CODEC_RewindPlay(STEP_BACK);
#endif
        /* Update the display information */
        LCD_Update(FRWD);  
      } 
      /* Disable mask of all interrupts */
      NVIC_RESETPRIMASK();
    }

    /* If "SEL" pushbutton is pressed */
    if(MyKey == SEL)
    {
      /* Mask All Interrupts */
      NVIC_SETPRIMASK();

      /* Update the display information */
      LCD_Update(STOP);

      /* Command the Stop of the current audio stream */
      SetVar_AudioPlayStatus(AudioPlayStatus_STOPPED);

      /* Disable mask of all interrupts */
      NVIC_RESETPRIMASK();

#if STM3210E
      I2S_CODEC_Stop();
      SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, DISABLE);
#else
      DAC_CODEC_Stop();
#endif

      /* Enable the FSMC that share a pin w/ I2C1 (LBAR) */
      RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

      /* Clear the LCD */
      LCD_Clear(White);

      /* Display the previous menu */
      DisplayMenu();
      /* Enable the JoyStick interrupts */
      IntExtOnOffConfig(ENABLE); 
      return;     
    }
    /* If "KEY" pushbutton is pressed */
    if(MyKey == KEY)
    {
      /* Mask All Interrupts */
      NVIC_SETPRIMASK();

      /* If the Codec is Playing => PAUSE */
      if (GetVar_AudioPlayStatus() == AudioPlayStatus_PLAYING)
      {
        /* Update the display information */
        LCD_Update(PAUSE);

        /* Command the Pause of the current stream */
        SetVar_AudioPlayStatus(AudioPlayStatus_PAUSED);
      }

      /* If the Codec is PAUSED => Resume PLAYING */
      else if (GetVar_AudioPlayStatus() == AudioPlayStatus_PAUSED)
      {
        /* Update the LCD display */ 
        LCD_Update(PLAY); 

#if STM3210E
        /* Start playing from the last saved position */
        I2S_CODEC_Play(GetVar_AudioDataIndex());
#else
        DAC_CODEC_Play(GetVar_AudioDataIndex());
#endif
      }
      /* If the Codec is STOPPED => PLAY from the file start address */
      else if (GetVar_AudioPlayStatus() == AudioPlayStatus_STOPPED)
      {
        /* Update the display information */
        LCD_Update(PLAY);

#if STM3210E
        /* Initialize all devices w/choosen parameters */
        err = I2S_CODEC_Init(GetVar_CurrentOutputDevice(), AUDIO_FILE_ADDRESS);
#else
        err = DAC_CODEC_Init(AUDIO_FILE_ADDRESS);
#endif
  
        /* Error message display if failure */
        if (err != 0)
        {
          LCD_DisplayError(err);

          /* Enable the FSMC that share a pin w/ I2C1 (LBAR) */
          RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

          /* Clear the LCD */
          LCD_Clear(White);

          /* Display the previous menu */
          DisplayMenu();

          /* Disable mask of all interrupts */
          NVIC_RESETPRIMASK();

          /* Enable the JoyStick interrupts */
          IntExtOnOffConfig(ENABLE); 
          return; 
        }  
          
#if STM3210E
        /* Enable Playing the audio file */
        I2S_CODEC_Play(GetVar_DataStartAddr());
#else
        DAC_CODEC_Play(GetVar_DataStartAddr());
#endif
      }
      /* Disable mask of all interrupts */
      NVIC_RESETPRIMASK();
    }
  }
}

/*******************************************************************************
* Function Name  : LCD_Update
* Description    : Controls the wave player application LCD display messages.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_Update(u32 Status)
{
  u8 tmp = 0;
  u32 counter = 0;

  /* Enable the FSMC that share a pin w/ I2C1 (LBAR) */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

  switch (Status)
  {
   case PROGRESS:
         tmp = (u8) ((u32)((GetVar_AudioDataIndex()) * 100) / GetVar_AudioDataLength());
         if (tmp == 0)
         { 
           LCD_SetTextColor(Magenta);
           LCD_ClearLine(Line8);
           LCD_DrawRect(Line8, 310, 16, 300);
         }
         else
         {
           LCD_SetTextColor(Magenta);
           LCD_DrawLine(Line8, 310 - (tmp * 3), 16, Vertical);
         }         
         break;
   case FRWD:
         tmp = (u8) ((u32)((GetVar_AudioDataIndex()) * 100) / GetVar_AudioDataLength());

         LCD_SetTextColor(Magenta);
         LCD_ClearLine(Line8);
         LCD_DrawRect(Line8, 310, 16, 300);
         LCD_SetTextColor(Magenta);

         for (counter = 0; counter <= tmp; counter++)
         {
           LCD_DrawLine(Line8, 310 - (counter * 3), 16, Vertical);
         }          
         break;
   case STOP:
         /* Display the stopped status menu */ 
         LCD_SetTextColor(White); 
         LCD_DisplayStringLine(Line3, CmdTitle1Stopped);
         LCD_DisplayStringLine(Line4, CmdTitle2Stopped);
         LCD_SetTextColor(Red);
         LCD_DisplayStringLine(Line6, StatusTitleStopped);
         LCD_ClearLine(Line9);
         LCD_SetTextColor(Black);
         LCD_DisplayChar(Line9, 250, 'v'); 
         LCD_DisplayChar(Line9, 235, 'o'); 
         LCD_DisplayChar(Line9, 220, 'l'); 
         LCD_DisplayChar(Line9, 200, '-'); 
         LCD_DisplayChar(Line9, 85, '+'); 
         LCD_DrawRect(Line9 + 8, 185, 10, 100); 
         break; 
   case PAUSE: 
         /* Display the paused status menu */ 
         LCD_SetTextColor(White);
         LCD_DisplayStringLine(Line3, CmdTitle1Paused);
         LCD_DisplayStringLine(Line4, CmdTitle2Paused);
         LCD_SetTextColor(Red);
         LCD_DisplayStringLine(Line6, StatusTitlePaused);
         break;
   case PLAY:
         /* Display the Titles */   
         LCD_SetTextColor(Black);
         LCD_DisplayStringLine(Line0, DemoTitle);
         LCD_DisplayStringLine(Line2, CmdTitle0); 

         /* Display the Playing status menu */ 
         LCD_SetTextColor(White);
         LCD_DisplayStringLine(Line3, CmdTitle1Playing);
         LCD_DisplayStringLine(Line4, CmdTitle2Playing);
         LCD_SetTextColor(Red);
         LCD_DisplayStringLine(Line6, StatusTitlePlaying);
         LCD_ClearLine(Line9);
         LCD_SetTextColor(Black);
         LCD_DisplayChar(Line9, 250, 'v'); 
         LCD_DisplayChar(Line9, 235, 'o'); 
         LCD_DisplayChar(Line9, 220, 'l'); 
         LCD_DisplayChar(Line9, 200, '-'); 
         LCD_DisplayChar(Line9, 85, '+'); 
         LCD_DrawRect(Line9 + 8, 185, 10, 100); 
         break;
   case ALL: 
         I2S_CODEC_LCDConfig();
         /* Display the stopped status menu */ 
         LCD_SetTextColor(White); 
         LCD_DisplayStringLine(Line3, CmdTitle1Stopped);
         LCD_DisplayStringLine(Line4, CmdTitle2Stopped);
         LCD_SetTextColor(Red);
         LCD_DisplayStringLine(Line6, StatusTitleStopped);
         LCD_ClearLine(Line9);
         LCD_SetTextColor(Black);
         LCD_DisplayChar(Line9, 250, 'v'); 
         LCD_DisplayChar(Line9, 235, 'o'); 
         LCD_DisplayChar(Line9, 220, 'l'); 
         LCD_DisplayChar(Line9, 200, '-'); 
         LCD_DisplayChar(Line9, 85, '+'); 
         LCD_DrawRect(Line9 + 8, 185, 10, 100); 
         break;
  }
  /* Update the volume bar in all cases except when progress bar is to be apdated */
  if (Status != PROGRESS)
  {
    /* Compute the current volume percentage */
    tmp = (u8) ((u16)((0xFF - GetVar_CurrentVolume()) * 100) / 0xFF) ;
 
    /* Clear the previuos volume bar */
    LCD_SetTextColor(Blue);
    LCD_DrawLine(Line9 + 10, 185 - previoustmp , 8, Vertical);
    LCD_DrawLine(Line9 + 10, 185 - previoustmp + 1 , 8, Vertical);    
 
    /* Draw the new volume bar */
    LCD_SetTextColor(Red);
    LCD_DrawLine(Line9 + 10, 185 - tmp , 8, Vertical);
    LCD_DrawLine(Line9 + 10, 185 - tmp + 1 , 8, Vertical);
 
    /* save the current position */
    previoustmp = tmp;
  }
  /* Disable the FSMC that share a pin w/ I2C1 (LBAR) */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, DISABLE);
}

/*******************************************************************************
* Function Name  : LCD_DisplayError
* Description    : Displays error message on the LCD screen and prompt user to 
*                  reset the application.
* Input          : err: Error code.
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_DisplayError(u32 err)
{
  I2S_CODEC_LCDConfig();

  /* Enable the FSMC that share a pin w/ I2C1 (LBAR) */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

  LCD_SetTextColor(Red); 

  /* Clear the LCD */
  LCD_Clear(White);


  /* The memory initialazation failed */
  if (err == 1)
  {
    LCD_DisplayStringLine(Line7, memerr); 
  }

  /* The audio file initialization failed (wrong audio format or wrong file) */
  if (err == 2)
  {
    LCD_DisplayStringLine(Line7, fileerr); 
  }

  /* I2C communication failure occured */
  if (err == 3)
  {
    LCD_DisplayStringLine(Line7, i2cerr); 
  }

  LCD_DisplayStringLine(Line8, "Push JoyStick to    ");
  LCD_DisplayStringLine(Line9, "exit.               ");

  /* Disable the FSMC that share a pin w/ I2C1 (LBAR) */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, DISABLE);

  while(ReadKey() == NOKEY)
  {
  }
}

/*******************************************************************************
* Function Name  : I2S_CODEC_LCDConfig
* Description    : Initialize the LCD device and display the welcome screen.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2S_CODEC_LCDConfig(void)
{
  /* Set the text and the background color */
  LCD_SetBackColor(Blue);
  LCD_SetTextColor(Black);
  LCD_Clear(White);

  /* Display the Titles */  
  LCD_DisplayStringLine(Line0, DemoTitle);
  LCD_DisplayStringLine(Line2, CmdTitle0); 
}

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
