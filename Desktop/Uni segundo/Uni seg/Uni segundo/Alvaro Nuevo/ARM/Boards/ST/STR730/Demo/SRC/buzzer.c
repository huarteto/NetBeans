/******************** (C) COPYRIGHT 2005 STMicroelectronics ********************
* File Name          : buzzer.c
* Author             : MCD Application Team
* Date First Issued  : 09/27/2005 :  V1.0
* Description        : This file provides functions to play melodies.
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
#define EE_SPILoadAddr  0x00
#define EE_I2CLoadAddr  0x00

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
GPIO_InitTypeDef  GPIO1_InitStructure1;
TB_InitTypeDef    TB_InitStructure;
TIM_InitTypeDef   TIM1_InitStructure;

bool MelodySingle = FALSE;
u8 MelodyData[256];
u8 AlarmData[256];
u8 ToneCount = 0;
u8 MelodyLength;
u16 MelodyLoadAddr = 0;
u16 Tempo = Tempo225;

/* Star Wars */
u8 MelodyData1[] = {_Do(o2),  _c , _cut,     _qq, _Do(o2), _c,  _cut,    _qq,
                    _Do(o2),  _c , _cut,     _qq, _Do(o2), _sb, _Sol(o2), _m,
                    _Fa(o2),  _c , _Mi(o2),  _c,  _Re(o2), _c,  _Do(o3),  _sb,
                    _Sol(o2), _m , _Fa(o2),  _c,  _Mi(o2), _c,  _Re(o2),  _c,
                    _Do(o3),  _sb, _Sol(o2), _m,  _Fa(o2), _c,  _Mi(o2),  _c,
                    _Fa(o2),  _c , _Re(o2),  _m};                                                                                                                                

/* Indiana Jones */
u8 MelodyData2[] = {_Mi(o1),  _qp, _Mi(o1),  _qp, _Mi(o1),  _qp, _Fa(o1),  _qp,
                    _Sol(o1), _cp, _cut,     _qq, _Do(o2),  _sb, _Re(o1),  _qp,
                    _Re(o1),  _qp, _Re(o1),  _qp, _Mi(o1),  _qp, _Fa(o1),  _cp,
                    _Fa(o1),  _sb, _La(o1),  _qp, _La(o1),  _qp, _La(o1),  _qp,
                    _Si(o1),  _qp, _Do(o2),  _cp, _cut,     _qq, _Fa(o2),  _sb,
                    _La(o1),  _qp, _La(o1),  _qp, _La(o1),  _qp, _Si(o1),  _qp,
                    _Do(o2),  _m,  _Re(o2),  _m,  _Mi(o2),  _m,  _Mi(o1),  _qp,
                    _Mi(o1),  _qp, _Mi(o1),  _qp, _Fa(o1),  _qp, _Sol(o1), _cp,
                    _cut,     _qq, _Do(o2),  _sb, _Re(o2),  _qp, _Re(o2),  _qp,
                    _Re(o2),  _qp, _Mi(o2),  _qp, _Fa(o2),  _m,  _Fa(o2),  _sb,
                    _Sol(o1), _qp, _Sol(o1), _qp, _Sol(o1), _qp, _cut,     _qq,
                    _Sol(o1), _qp, _Fa(o2),  _m,  _Mi(o2),  _qp, _Mi(o2),  _qp,
                    _Mi(o2),  _qp, _Do(o2),  _qp, _Fa(o2),  _m,  _Mi(o2),  _qp,
                    _Mi(o2),  _qp, _Mi(o2),  _qp, _Re(o2),  _qp, _Do(o2),  _m};                                                                                                                                                                                                                                                                            


/* dragon de tei */ 
u8 MelodyData3[] = {_Do(o2),  _c,  _Si(o1),  _q,  _Do(o2),  _cp, _La(o1), _q,
                    _cut,     _q,  _La(o1),  _q,  _Do(o2),  _q,  _La(o1), _sq,
                    _cut,     _sq, _Sol(o1), _sq, _cut,     _sq, _Mi(o1), _cp,
                    _cut,     _q,  _Do(o2),  _q,  _Mi(o2),  _sq, _cut,    _sq,
                    _Re(o2),  _sq, _cut,     _sq, _Si(o1),  _sq, _cut,    _sq, 
                    _Sol(o1), _c,  _cut,     _c,  _Sol(o1), _q,  _Re(o2), _sq,
                    _cut,     _sq, _Do(o2),  _q,  _Si(o1),  _q,  _Do(o2), _q,
                    _Si(o1),  _sq, _cut,     _sq, _La(o1),  _q,  _cut,    _q,
                    _Do(o2),  _c,  _Si(o1),  _q,  _Do(o2),  _c,  _cut,    _q,
                    _La(o1),  _q,  _cut,     _q,  _La(o1),  _q,  _Do(o2), _q,
                    _La(o1),  _sq, _cut,     _sq, _Sol(o1), _sq, _cut,    _sq,
                    _Mi(o1),  _q};                                                                                                                                                                                                                                                                                                                                                    


/* Alarm melody */
u8 AlarmData1[] = {_Sol(o3), _qq, _cut, _qq, _Sol(o3), _m, _cut, _c,
                   _Sol(o3), _c,  _cut, _qq, _Sol(o3), _m, _cut, _c,
                   _Sol(o3), _c,  _cut, _qq, _Sol(o3), _m, _cut, _c}; 

/* Private function prototypes -----------------------------------------------*/
/* Interface functions -------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : Melody_Init 
* Description    : Initializes the peripherals used in melody player.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Melody_Init(void)
{
  /* ADC and P3.0 (trimmer input) configuration */
  Converter_Init();

  /* enable and start ADC */
  ADC_Cmd(ENABLE);
  ADC_ConversionCmd(ADC_ConversionStart);

  /* P1.7 configuration */
  GPIO1_InitStructure1.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO1_InitStructure1.GPIO_Pins = GPIO_PIN_7;
  GPIO_Init(GPIO1, &GPIO1_InitStructure1);

/* TIM1 configuration ---------------------------------------------------*/
  TIM_DeInit(TIM1);
  TIM_StructInit( &TIM1_InitStructure);
  TIM1_InitStructure.TIM_Mode = TIM_PWM;
  TIM1_InitStructure.TIM_Pulse_Level_A = TIM_Low; 
  TIM1_InitStructure.TIM_Clock_Source = TIM_CLK_INTERNAL;
  TIM1_InitStructure.TIM_Period_Level = TIM_High;
  TIM1_InitStructure.TIM_Prescaler = 0x7;
 
  TIM1_InitStructure.TIM_Pulse_Length_A = 0x35;
  TIM_Init (TIM1, &TIM1_InitStructure);

/* TB0 configuration ---------------------------------------------------*/
  TB_InitStructure.TB_Prescaler = 0xF; 
  TB_InitStructure.TB_CLK_Source = TB_CLK_EXTERNAL;
  TB_InitStructure.TB_Preload = 0xF;
  TB_Init(TB0, &TB_InitStructure);
  TB_ITConfig(TB0, ENABLE);

  EIC_IRQChannelConfig(TB0_IRQChannel, ENABLE );
  EIC_IRQChannelPriorityConfig(TB0_IRQChannel,4);
} 

/*******************************************************************************
* Function Name  : PlayNextTone
* Description    : Plays the tone contained in MelodyData buffer.
* Input          : None
* Output         : ToneCount: gives the position of the tone to be played 
* Return         : None
*******************************************************************************/
void PlayNextTone(void)
{
  u16 Note = 0;

  /* disable screen saver mode */
  WDG_Cmd(DISABLE);

  /* stop both TB0 and TIM1 */
  TIM_CounterCmd(TIM1, TIM_STOP);
  TB_Cmd(TB0, DISABLE);

  if(ToneCount < MelodyLength)
  {
    Note = MelodyData[ToneCount++];
    Note = (Note << 8)| MelodyData[ToneCount++];
    
    /* set tone frequency */
    TIM1_InitStructure.TIM_Full_Period = Note;
    
    /* adjust volume */    
    TIM1_InitStructure.TIM_Pulse_Length_A = (ADC_GetConversionValue(ADC_CHANNEL0)/3); 
   
    /* set tone delay */
    TB_SetPeriodValue(TB0, Tempo/MelodyData[ToneCount++]);

    TIM_Init(TIM1, &TIM1_InitStructure);

    /* start TIM1 to generate the modulated sound */
    TIM_CounterCmd(TIM1, TIM_START); 

    /* start TB0 to give rhythm to the melody */
    TB_Cmd(TB0, ENABLE); 
  }
  else
  { 
    ToneCount = 0; 
    
    if(MelodySingle == TRUE)
    {       
      StopMelody();
    }
    else
    { /* repeat the current melody */
      PlayNextTone();
    }
  }
}

/*******************************************************************************
* Function Name  : PlayMelody1
* Description    : Plays melody1.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PlayMelody1(void)
{
  MelodyLoadAddr = EE_SPILoadAddr;
  MelodyLength = countof(MelodyData1);
  Tempo = Tempo160;
  
  /* load melody1 data from the SPI EEPROM to MelodyData buffer */
  SPI_EE_DMABufferRead(MelodyData, MelodyLoadAddr, MelodyLength);
  
  /* play melody1 */
  PlayNextTone();
}

/*******************************************************************************
* Function Name  : PlayMelody2
* Description    : Plays melody2.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PlayMelody2(void)
{
  MelodyLoadAddr = EE_SPILoadAddr + countof(MelodyData1) + 1;
  MelodyLength = countof(MelodyData2);
  Tempo = Tempo112;
  
  /* load melody2 data from the SPI EEPROM to MelodyData buffer */
  SPI_EE_DMABufferRead(MelodyData, MelodyLoadAddr, MelodyLength);
  
  /* play melody2 */
  PlayNextTone ();
}

/*******************************************************************************
* Function Name  : PlayMelody3
* Description    : Plays melody3.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PlayMelody3(void)
{
  MelodyLoadAddr = EE_SPILoadAddr + countof(MelodyData1) + countof(MelodyData2) + 1;
  MelodyLength = countof(MelodyData3);
  Tempo = Tempo200;
  
  /* load melody3 data from the SPI EEPROM to MelodyData buffer */
  SPI_EE_DMABufferRead(MelodyData, MelodyLoadAddr, MelodyLength);
  
  /* play melody3 */
  PlayNextTone ();
}

/*******************************************************************************
* Function Name  : PlayAlarmMelody
* Description    : Plays alarm melody.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PlayAlarmMelody(void)
{
  u8 AlarmToneCount = 0;
  u8 AlarmMelodyLength = 0;
  u16 AlarmTempo = Tempo200;
  u16 Note = 0;

  Melody_Init();
  
  AlarmMelodyLength = countof(AlarmData1);
  
  /* load alarm melody data from the I2C EEPROM to AlarmData buffer */
  I2C_EE_BufferRead(AlarmData, EE_I2CLoadAddr, AlarmMelodyLength);
 
  TB_Init(TB2, &TB_InitStructure);
    
  GPIO_WordWrite(GPIO4, 0x000);
  
  while(AlarmToneCount < AlarmMelodyLength)
  { 
    TIM_CounterCmd(TIM1, TIM_STOP);
 
    Note = AlarmData[AlarmToneCount++];
    Note = (Note << 8)| AlarmData[AlarmToneCount++];
    
    /* set tone frequency */
    TIM1_InitStructure.TIM_Full_Period = Note;
    
    /* adjust volume */
    TIM1_InitStructure.TIM_Pulse_Length_A = (ADC_GetConversionValue( ADC_CHANNEL0)/5);
    
    /* set tone delay */
    TB_SetPeriodValue(TB2, AlarmTempo/AlarmData[AlarmToneCount++]);
 
    TIM_Init(TIM1, &TIM1_InitStructure);    
    TIM_CounterCmd(TIM1, TIM_START);
    TB_Cmd(TB2, ENABLE);
    
    GPIO_WordWrite(GPIO4, ~GPIO_WordRead(GPIO4));
    
    /* wait until TB2 End of Count (EC) event occurs */
    while(TB_FlagStatus(TB2) == RESET); 
    
    /* clear EC flag */
    TB_FlagClear(TB2);

    TB_Cmd(TB2, DISABLE);
  }

  TB_Cmd(TB2, DISABLE);
}

/*******************************************************************************
* Function Name  : StopMelody
* Description    : Stops melody playing.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void StopMelody(void)
{
  /* stop TIM1 */
  TIM_CounterCmd(TIM1, TIM_STOP);

  /* stop TB0 */
  TB_Cmd(TB0, DISABLE);

  /* enable screen saver mode */
  WDG_Cmd(ENABLE);
}

/*******************************************************************************
* Function Name  : Single
* Description    : Sets melody play mode to single. 
* Input          : None
* Output         : - ToneCount: gives the position of the tone to be played
*                  - MelodySingle: indicates the play mode  
* Return         : None
*******************************************************************************/
void Single(void)
{
  ToneCount = 0;
  
  MelodySingle = TRUE;
}

/*******************************************************************************
* Function Name  : SingleRepeat
* Description    : Sets melody play mode to single repeat.
* Input          : None
* Output         : - ToneCount: gives the position of the tone to be played  
*                  - MelodySingle: indicates the play mode  
* Return         : None
*******************************************************************************/
void SingleRepeat(void)
{
  ToneCount = 0;
  
  MelodySingle = FALSE;
}

/*******************************************************************************
* Function Name  : SaveMelodyToSPI
* Description    : Saves melodies data to SPI EEPROM.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SaveMelodyToSPI(void)
{
  /* initialize the SPI EEPROM driver */
  SPI_EE_Init();

  /* write melody1 data */
  SPI_EE_BufferWrite(MelodyData1, EE_SPILoadAddr, countof(MelodyData1)); 
  
  /* write melody2 data */
  SPI_EE_BufferWrite(MelodyData2, (EE_SPILoadAddr + countof(MelodyData1) + 1), 
                    countof(MelodyData2)); 
  
  /* write melody3 data */                  
  SPI_EE_BufferWrite(MelodyData3, (EE_SPILoadAddr + countof(MelodyData1) + countof(MelodyData2) + 1),
                    countof(MelodyData3));
}

/*******************************************************************************
* Function Name  : SaveAlarmMelodyToI2C
* Description    : Saves alarm melody data to I2C EEPROM.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SaveAlarmMelodyToI2C(void)
{	
  /* initialize the I2C EEPROM driver */
  I2C_EE_Init();

  /* write alarm melody data */
  I2C_EE_BufferWrite(AlarmData1, EE_I2CLoadAddr, countof(AlarmData1));    
}

/*******************************************************************************
* Function Name  : TB_SetPeriodValue
* Description    : Sets TBx period value. Before calling this function the TBx 
*                  must be stopped.
* Input          : - TBx: where x can be 0, 1 or 2 to select the TB peripheral. 
*                  - Time: Amount of time needed in ms. The allowed values are 
*                    from 1 ms to 65012 ms.
* Output         : None
* Return         : None
*******************************************************************************/
void TB_SetPeriodValue(TB_TypeDef *TBx, u16 nTime)
{
  TB_InitStructure.TB_CLK_Source = TB_CLK_EXTERNAL; /* EXTCLK := 31.25 KHz */
  TB_InitStructure.TB_Prescaler = 0x1E; /* TBx clocked at 1008Hz (31.25KHz / 31)*/
  TB_InitStructure.TB_Preload = nTime-1; /* TBx period = nTime * 1 ms */

  TB_Init(TBx, &TB_InitStructure);
}

/******************* (C) COPYRIGHT 2005 STMicroelectronics *****END OF FILE****/
