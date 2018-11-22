/*----------------------------------------------------------------------------
 * Name:    DAC_CODEC.c
 * Purpose: Speaker driver using DAC 
 * Version: V1.00
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * Copyright (c) 2005-2008 Keil Software. All rights reserved.
 *---------------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi_flash.h"
#include "DAC_CODEC.h"

/* Private typedef -----------------------------------------------------------*/
typedef enum
{
  LittleEndian,
  BigEndian
}Endianness;

/* Private define ------------------------------------------------------------*/

/* Audio file header size */
#define HEADER_SIZE          100

/* Uncomment this line to enable verifying data sent to codec after each write opeation */
//#define VERIFY_WRITTENDATA

/* Audio Parsing Constants */
#define  ChunkID             0x52494646  /* correspond to the letters 'RIFF' */
#define  FileFormat          0x57415645  /* correspond to the letters 'WAVE' */
#define  FormatID            0x666D7420  /* correspond to the letters 'fmt ' */
#define  DataID              0x64617461  /* correspond to the letters 'data' */
#define  FactID              0x66616374  /* correspond to the letters 'fact' */

#define  WAVE_FORMAT_PCM     0x01
#define  FormatChunkSize     0x10
#define  Channel_MONO        0x01
#define  Channel_STEREO      0x02

#define  SampleRate_8000     8000
#define  SampleRate_11025    16000
#define  SampleRate_16000    16000
#define  SampleRate_22050    22050
#define  SampleRate_44100    44100
#define  SampleRate_48000    48000
#define  Bits_Per_Sample_8   8
#define  Bits_Per_Sample_16  16

#define  DUMMY_DATA           0x1111
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Audio Frequency value */
u16 dacaudiofreq = SampleRate_8000;

/* Header Table containing the audio file information */
u8 HeaderTabIndex = 0;
u8 AudioFileHeader[HEADER_SIZE]; 

/* Audio Codec variables */
vu32 AudioFileAddress = AUDIO_FILE_ADDRESS; 
u32  AudioDataLength = 0;
u32  DataStartAddr = 0x0;
vu32 AudioDataIndex = 0;
static vu16 TIM2ARRValue = 0x00;
static vu32 AudioReplay = 0xFFFF;
static u32 AudioReplayCount = 0xFFFF;
static vu32 SendDummyData = 0;
static vu32 AudioPlayStatus = AudioPlayStatus_STOPPED;
static u8 CurrentVolume = DEFAULT_VOL;
static u32 errorcode = 0xFF;
static vu32 monovar = 0, tmpvar = 0;
u8     AudioDataBuffer[32];

/* Wave details names table */
WAVE_FormatTypeDef  WAVE_Format;
volatile ErrorCode  WaveFileStatus = Unvalid_RIFF_ID;   
 
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static void NVIC_Config(void); 
static void DAC_GPIO_Config(void);
static void DAC_Config(u16 AudioFreq);
static u32 ReadUnit(u8 NbrOfBytes, Endianness BytesFormat);
u32 AudioFile_Init(void);

/*******************************************************************************
* Function Name  : DAC_CODEC_Init
* Description    : Initializes the DAC audio codec.
* Input          : - Address: Specifies the location of the audio file in the memory.   
* Output         : None
* Return         : - 0: if all initializations are OK.
*                  - 1: if memory initialization failed (LD2 is turned on).
*                  - 2: if audio file initialization failed (LD2 is turned on).
*                  - 3: if Codec initialization failed (LD1 is turned on).
*******************************************************************************/
u32 DAC_CODEC_Init(u32 Address)
{
	u32 count = 0;

  /* Set the audio file address */
  AudioFileAddress = (vu32) Address;

  /* Configure I2S interrupt Channel */
  NVIC_Config();

  /* Configure the DAC pin (PA4) */
  DAC_GPIO_Config(); 

	/* Read the Audio file to extract the audio data length and frequency */
  errorcode = AudioFile_Init();

  if (errorcode < 3)
  {
    /* Turn on LD2 connected to PB.09 */
    GPIO_SetBits(GPIOB, GPIO_Pin_9);
  
    return errorcode;
  }

  /* Configure the DAC peripheral */
  DAC_Config(dacaudiofreq); 

  if (count != 0)
  {
    /* Turn on LD1 connected to PB.08 */
    GPIO_SetBits(GPIOB, GPIO_Pin_8);

    return 3;
  }

  /* Turn on LD4 connected to PB.11 */
  GPIO_SetBits(GPIOB, GPIO_Pin_11);

  return 0; /* Configuration is OK */
}

/*******************************************************************************
* Function Name  : DAC_CODEC_ReplayConfig
* Description    : Set AudioReplay variable value .
* Input          : Repetions: Number of repetitions
* Output         : None
* Return         : None
*******************************************************************************/
void DAC_CODEC_ReplayConfig(u32 Repetions)
{ 
  /* Audio Replay number set by user */
  AudioReplay = Repetions;
  
  /* Audio Replays number remaining (if AudioReplay != 0) */
  AudioReplayCount = Repetions;
}

/*******************************************************************************
* Function Name  : DAC_CODEC_Play
* Description    : Plays the audio file.
* Input          : - AudioStartPosition: Adress from which the wave data begin
* Output         : None
* Return         : AudioDataIndex value.
*******************************************************************************/
u32 DAC_CODEC_Play(u32 AudioStartPosition)
{   
	/* Send the read command to the media */
  Media_StartReadSequence(AudioFileAddress + AudioStartPosition + 1); 

  /* Set Playing status to inform other modules about the codec status */
  SetVar_AudioPlayStatus(AudioPlayStatus_PLAYING);

  /* Set autoreload value */ 
  TIM_SetAutoreload(TIM2, TIM2ARRValue);

  /* Start TIM2 */
  TIM_Cmd(TIM2, ENABLE);

  /* Enable TIM2 interrupt used for sound wave amplitude update */
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
 
  return AudioDataIndex;
}

/*******************************************************************************
* Function Name  : DAC_CODEC_Pause
* Description    : Pause playing the audio file.
* Input          : None
* Output         : None
* Return         : Current Position.
*******************************************************************************/
u32 DAC_CODEC_Pause()
{
  TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

  /* Disable TIM2 update interrupt */
  TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);

  /* Set Paused status to inform other modules about the codec status */
  SetVar_AudioPlayStatus(AudioPlayStatus_PAUSED);
  
  /* Reset local variables */
  monovar = 0;
  tmpvar = 0;

  if (WAVE_Format.NumChannels == Channel_MONO) 
  {
    /* Force the parity of the address */ 
    AudioDataIndex &= 0xFFFFFFFE; 
  }
  /* Return the current data pointer position */
  return AudioDataIndex;
}

/*******************************************************************************
* Function Name  : DAC_CODEC_Stop
* Description    : Stop playing the audio file, reset the pointer and power off 
*                : the Codec.
* Input          : None
* Output         : None
* Return         : 0 if operation complete.
*******************************************************************************/
u32 DAC_CODEC_Stop()
{
  /* Disable TIM2 update interrupt */
  TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
  /* Disable TIM2 */
  TIM_Cmd(TIM2, DISABLE);

	/* Reinitialize the audio data pointer */
  AudioDataIndex = 0;

  /* Set Paused status to inform other modules about the codec status */
  SetVar_AudioPlayStatus(AudioPlayStatus_STOPPED);

  /* Reset local variables */
  monovar = 0;
  tmpvar = 0;

  return 0;
}

/*******************************************************************************
* Function Name  : DAC_CODEC_ControlVolume
* Description    : Controls the audio volume.
* Input          :  - direction: VolumeDirection_HIGH (0xF) to increase the volume, 
*                :     VolumeDirection_LOW (0xA) to decrease  the volume or 
*                :     VolumeDirection_LEVEL (0x0) to set a defined level of volume.
*                :  -	Volume: the step of volume increqing/decreasing (when direction == 0)
*                :     or the volume level to be set (when direction != 0). 
* Output         : None
* Return         : 0-> correct communication, else wrong communication
*******************************************************************************/
u32 DAC_CODEC_ControlVolume(u32 direction, u8 Volume)
{
	if (direction == VolumeDirection_HIGH)
  {    
    /* Check if the volume high limit is reached */
		if (CurrentVolume < VOLStep)
    {
      CurrentVolume = 0;
    }
    else
    {
	    /* Save the current volume level */
      CurrentVolume -= Volume;
    }
  }
	else if (direction == VolumeDirection_LOW)
  {    
    /* Check if the volume low limit is reached */
		if (CurrentVolume > (0xFF - VOLStep))
    {
      CurrentVolume = 0xFF; 
    }
    else
    {
      /* Save the current volume level */
      CurrentVolume += Volume;
    }
  }
  else if (direction == VolumeDirection_LEVEL) 
  {
    CurrentVolume = Volume;
  }
  else
  {
    return 0xFF; //Error verifying the Codec registers
  }
  
  return 0;
}

/*******************************************************************************
* Function Name  : DAC_CODEC_ForwardPlay
* Description    : Forward function.
* Input          : - Step: number of steps forward
* Output         : None.
* Return         : None.
*******************************************************************************/
void DAC_CODEC_ForwardPlay(u32 Step)
{   
  /* Pause Playing the audio file */
  DAC_CODEC_Pause();

  /* Increment the Audio pointer */
  IncrementVar_AudioDataIndex((AudioDataLength / 100) * Step);

  /* Insure the index parity */
  AudioDataIndex &= 0xFFFFFFFE;

  /* Resume playing from the new position */
  DAC_CODEC_Play((GetVar_AudioDataIndex()));
}

/*******************************************************************************
* Function Name  : DAC_CODEC_RewindPlay
* Description    : Rewind function.
* Input          : - Step: number of steps back
* Output         : None.
* Return         : None.
*******************************************************************************/
void DAC_CODEC_RewindPlay(u32 Step)
{   
  /* Pause Playing the audio file */
  DAC_CODEC_Pause();

  /* Increment the Audio pointer */
  DecrementVar_AudioDataIndex((AudioDataLength / 100) * Step);

  /* Insure the index parity */
  AudioDataIndex &= 0xFFFFFFFE;

  /* Resume playing from the new position */
  DAC_CODEC_Play((GetVar_AudioDataIndex()));
}

/*******************************************************************************
* Function Name  : DAC_CODEC_UpdateStatus
* Description    : Check if STOP or PAUSE command are generated and performs the 
*                :  relative action (STOP or PAUSE playing)
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void DAC_CODEC_UpdateStatus(void)
{ 
  /* STOP command is generated => Stop playing the audio file */
  if (GetVar_AudioPlayStatus() == AudioPlayStatus_STOPPED)
  {
    DAC_CODEC_Stop();
  }
  
  /* PAUSE Command is generated => PAUSE playing the audio File */
  if (GetVar_AudioPlayStatus() == AudioPlayStatus_PAUSED)
  {
    DAC_CODEC_Pause();
  }
}

/*******************************************************************************
* Function Name  : GetVar_DataStartAddr
* Description    : returns DataStartAddr variable value (used by stm32f10x_it.c file).
* Input          : None
* Output         : None
* Return         : AudioDataIndex
*******************************************************************************/
u32 GetVar_DataStartAddr(void)
{ 
  return DataStartAddr;
}


/*******************************************************************************
* Function Name  : GetVar_CurrentVolume
* Description    : returns CurrentVolume variable value (used by extern files).
* Input          : None
* Output         : None
* Return         : CurrentVolume
*******************************************************************************/
u8 GetVar_CurrentVolume(void)
{ 
  return CurrentVolume;
}

/*******************************************************************************
* Function Name  : DAC_CODEC_WaveParsing
* Description    : Checks the format of the .WAV file and gets information about
*                  the audio format. This is done by reading the value of a
*                  number of parameters stored in the file header and comparing
*                  these to the values expected authenticates the format of a
*                  standard .WAV  file (44 bytes will be read). If  it is a valid
*                  .WAV file format, it continues reading the header to determine
*                  the  audio format such as the sample rate and the sampled data
*                  size. If the audio format is supported by this application,
*                  it retrieves the audio format in WAVE_Format structure and
*                  returns a zero value. Otherwise the function fails and the
*                  return value is nonzero.In this case, the return value specifies
*                  the cause of  the function fails. The error codes that can be
*                  returned by this function are declared in the header file.
* Input          : None
* Output         : None
* Return         : Zero value if the function succeed, otherwise it return
*                  a nonzero value which specifies the error code.
*******************************************************************************/
ErrorCode DAC_CODEC_WaveParsing(u8* HeaderTab)
{
  u32 Temp = 0x00;
  u32 ExtraFormatBytes = 0; 
  
  /* Initialize the HeaderTabIndex variable */
  HeaderTabIndex = 0;

  /* Read chunkID, must be 'RIFF'	----------------------------------------------*/
  Temp = ReadUnit(4, BigEndian);
  if(Temp != ChunkID)
  {
    return(Unvalid_RIFF_ID);
  }
  /* Read the file length ----------------------------------------------------*/
  WAVE_Format.RIFFchunksize = ReadUnit(4, LittleEndian);
	
  /* Read the file format, must be 'WAVE' ------------------------------------*/
  Temp = ReadUnit(4, BigEndian);
  if(Temp != FileFormat)
  {
    return(Unvalid_WAVE_Format);
  }
  /* Read the format chunk, must be 'fmt ' -----------------------------------*/
  Temp = ReadUnit(4, BigEndian);
  if(Temp != FormatID)
  {
    return(Unvalid_FormatChunk_ID);
  }
  /* Read the length of the 'fmt' data, must be 0x10 -------------------------*/
  Temp = ReadUnit(4, LittleEndian);
  if(Temp != 0x10)
  {
    ExtraFormatBytes = 1;
  }
  /* Read the audio format, must be 0x01 (PCM) -------------------------------*/
  WAVE_Format.FormatTag = ReadUnit(2, LittleEndian);
  if(WAVE_Format.FormatTag != WAVE_FORMAT_PCM)
  {
    return(Unsupporetd_FormatTag);	
  }	
  /* Read the number of channels: 0x02->Stereo 0x01->Mono --------------------*/
  WAVE_Format.NumChannels = ReadUnit(2, LittleEndian);

  /* Read the Sample Rate ----------------------------------------------------*/
  WAVE_Format.SampleRate = ReadUnit(4, LittleEndian);

  /* Update the OCA value according to the .WAV file Sample Rate */
  switch(WAVE_Format.SampleRate)
  {
    case SampleRate_8000 : TIM2ARRValue = 9000; break; /* 8KHz = 72MHz / 9000 */
    case SampleRate_11025: TIM2ARRValue = 6531; break; /* 11.025KHz = 72MHz / 6531 */
    case SampleRate_22050: TIM2ARRValue = 3265; break; /* 22.05KHz = 72MHz / 3265 */  
    case SampleRate_44100: TIM2ARRValue = 1633; break; /* 44.1KHz = 72MHz / 1633 */    
    default: return(Unsupporetd_Sample_Rate);
  }

  /* Read the Byte Rate ------------------------------------------------------*/
  WAVE_Format.ByteRate = ReadUnit(4, LittleEndian);
	
  /* Read the block alignment ------------------------------------------------*/
  WAVE_Format.BlockAlign = ReadUnit(2, LittleEndian);
	
  /* Read the number of bits per sample --------------------------------------*/
  WAVE_Format.BitsPerSample = ReadUnit(2, LittleEndian);
  if(WAVE_Format.BitsPerSample != Bits_Per_Sample_16)
  {
    return(Unsupporetd_Bits_Per_Sample);
  }
  /* If there are Extra format bytes, these bytes will be defined in "Fact Chunk" */
  if(ExtraFormatBytes == 1)
  {
    /* Read th Extra format bytes, must be 0x00 ------------------------------*/
    Temp = ReadUnit(2, LittleEndian);
    if(Temp != 0x00)
    {
      return(Unsupporetd_ExtraFormatBytes);
    }
    /* Read the Fact chunk, must be 'fact' -----------------------------------*/
    Temp = ReadUnit(4, BigEndian);
    if(Temp != FactID)
    {
      return(Unvalid_FactChunk_ID);
    }
    /* Read Fact chunk data Size ---------------------------------------------*/
    Temp = ReadUnit(4, LittleEndian);

    /* Set the index to start reading just after the header end */
    HeaderTabIndex += Temp;
  }
  /* Read the Data chunk, must be 'data' -------------------------------------*/
  Temp = ReadUnit(4, BigEndian);
  if(Temp != DataID)
  {
    return(Unvalid_DataChunk_ID);
  }
  /* Read the number of sample data ------------------------------------------*/
  WAVE_Format.DataSize = ReadUnit(4, LittleEndian);

  /* Set the data pointer at the beginning of the effective audio data */
	DataStartAddr += HeaderTabIndex;
  
  return(Valid_WAVE_File);	
}

/*******************************************************************************
* Function Name  : GetVar_AudioDataIndex
* Description    : returns AudioDataIndex variable value (used by stm32f10x_it.c file).
* Input          : None
* Output         : None
* Return         : AudioDataIndex
*******************************************************************************/
u32 GetVar_AudioDataIndex(void)
{ 
  return AudioDataIndex;
}

/*******************************************************************************
* Function Name  : SetVar_AudioDataIndex
* Description    : Sets AudioDataIndex variable value (used by stm32f10x_it.c file).
* Input          : None
* Output         : None
* Return         : AudioDataIndex
*******************************************************************************/
void SetVar_AudioDataIndex(u32 value)
{ 
  AudioDataIndex = value;
}

/*******************************************************************************
* Function Name  : IncrementVar_AudioDataIndex
* Description    : Increment the AudioDataIndex variable.
* Input          : - IncrementNumber: number of incrementations.
* Output         : None
* Return         : None
*******************************************************************************/
void IncrementVar_AudioDataIndex(u32 IncrementNumber)
{ 
  AudioDataIndex += (u32)IncrementNumber;
  
  if (AudioDataIndex >= AudioDataLength)
  {
    ResetVar_AudioDataIndex();
    Decrement_AudioReplay();
  }
}

/*******************************************************************************
* Function Name  : DecrementVar_AudioDataIndex
* Description    : Set the AudioDataIndex variable to 1.
* Input          : None
* Output         : None
* Return         : None.
*******************************************************************************/
void DecrementVar_AudioDataIndex(u32 DecrementNumber)
{ 
  if (DecrementNumber >= AudioDataIndex)
  {
    ResetVar_AudioDataIndex();
  }
  else
  { 
    AudioDataIndex -= (u32)DecrementNumber;
  }
}

/*******************************************************************************
* Function Name  : ResetVar_AudioDataIndex
* Description    : Reset the AudioDataIndex variable.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ResetVar_AudioDataIndex(void)
{ 
  AudioDataIndex = DataStartAddr;

  /* Send the read command to the media */
  Media_StartReadSequence(AudioFileAddress + DataStartAddr + 1);
}

/*******************************************************************************
* Function Name  : GetVar_SendDummyData
* Description    : returns SendDummyData variable value (used by stm32f10x_it.c file).
* Input          : None
* Output         : None
* Return         : SendDummyData
*******************************************************************************/
u32 GetVar_SendDummyData(void)
{ 
  return SendDummyData;
}

/*******************************************************************************
* Function Name  : SetVar_SendDummyData
* Description    : Set the SendDummyData variable to 1.
* Input          : None
* Output         : None
* Return         : SendDummyData
*******************************************************************************/
u32 SetVar_SendDummyData(void)
{ 
  SendDummyData = (u32)0x1;
 
  return SendDummyData;
}

/*******************************************************************************
* Function Name  : ResetVar_SendDummyData
* Description    : Reset the SendDummyData variable to 0.
* Input          : None
* Output         : None
* Return         : SendDummyData
*******************************************************************************/
u32 ResetVar_SendDummyData(void)
{ 
  SendDummyData = (u32)0;
 
  return SendDummyData;
}

/*******************************************************************************
* Function Name  : GetVar_AudioPlayStatus
* Description    : returns AudioPlayStatus variable value (used by stm32f10x_it.c file).
* Input          : None
* Output         : None
* Return         : AudioPlayStatus value.
*******************************************************************************/
u32 GetVar_AudioPlayStatus(void)
{ 
  return AudioPlayStatus;
}

/*******************************************************************************
* Function Name  : SetVar_AudioPlayStatus
* Description    : Set the AudioDataIndex variable to Status.
* Input          : Status: could be AudioPlayStatus_STOPPED, AudioPlayStatus_PLAYING
*                :        or AudioPlayStatus_PAUSED.
* Output         : None
* Return         : AudioPlayStatus value.
*******************************************************************************/
u32 SetVar_AudioPlayStatus(u32 Status)
{ 
  AudioPlayStatus = (u32)Status;
 
  return AudioPlayStatus;
}

/*******************************************************************************
* Function Name  : GetVar_AudioReplay
* Description    : returns AudioReplay variable value.
* Input          : None
* Output         : None
* Return         : AudioReplay	value.
*******************************************************************************/
u32 GetVar_AudioReplay(void)
{ 
  return AudioReplay;
}

/*******************************************************************************
* Function Name  : SetVar_AudioReplay
* Description    : Decrement the AudioReplayCount variable if AudioReplay is different
*                :  from zero (infinite replaying).
* Input          : None.
* Output         : None
* Return         : AudioPlayStatus value.
*******************************************************************************/
void Decrement_AudioReplay(void)
{ 
  if (AudioReplay != 0)
  {
    AudioReplayCount--;

		if (AudioReplayCount == 0)
	  {
	    /* Command the Stop of the audio playing */ 
	    SetVar_AudioPlayStatus(AudioPlayStatus_STOPPED);
	    
	    /* Reset the counter */
	    AudioReplayCount = AudioReplay;
	  }
  }
}

/*******************************************************************************
* Function Name  : GetVar_AudioDataLength
* Description    : Get the current audio file data length .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
u32 GetVar_AudioDataLength(void)
{ 
  return AudioDataLength;
}

/*******************************************************************************
* Function Name  : GetVar_i2saudiofreq
* Description    : Get the current audio frequency .
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
u16 GetVar_i2saudiofreq(void)
{ 
  return dacaudiofreq;
}

/*******************************************************************************
* Function Name  : AudioFile_Init
* Description    : Initializes the SPI_Flash and returns the Wavadatalength variable.
* Input          : None
* Output         : None
* Return         : - The length of the wave file read from the SPI_Flash
*                  - 1 if an error occured when initializing the memory.
*                  - 2 if an error occured on the audio file intialization.
*******************************************************************************/
u32 AudioFile_Init(void)
{
  u32 err = 0;

  /* Initialize the media support */
  err = Media_Init();

  /* Check if Memory initialization is OK */
  if (err != 0)
  {
    return 1;
  }

  /* Read a Byte buffer and store it in the Header table*/
  Media_BufferRead(AudioFileHeader, AudioFileAddress, HEADER_SIZE);

  /* Read and check the audio file Header */
	WaveFileStatus = DAC_CODEC_WaveParsing(AudioFileHeader);

	/* Check if the selected file is a correct wave file */
  if(WaveFileStatus == Valid_WAVE_File)
  {
	 	/* Read and set the audio data length (/!\ data are counted as BYTES /!\) */ 
	  AudioDataLength = WAVE_Format.DataSize ;  
	  
	  /* Read and set the audio frequency */
	  dacaudiofreq = (u16)WAVE_Format.SampleRate;

    /* Return the audio file length */
		return AudioDataLength;
  }
	else /* Wrong wave file */
  {
   	return 2;
  }
}

/*******************************************************************************
* Function Name  : NVIC_Config
* Description    : Configure the TIM2s NVIC channel.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void NVIC_Config(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

	/* TIM2 IRQ Channel configuration */
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/*******************************************************************************
* Function Name  : DAC_GPIO_Config
* Description    : Initializes the GPIO pins used by the codec application.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void DAC_GPIO_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIOA, GPIOB and AFIO clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | 
                         RCC_APB2Periph_AFIO, ENABLE);

  /* DAC pin configuration (PA4) */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* LEDs pins configuration */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8  | GPIO_Pin_9  | GPIO_Pin_10 | GPIO_Pin_11 |
                                GPIO_Pin_12 | GPIO_Pin_13 ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Enable the DAC APB1 clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

	/* Turn Off All LEDs */
  GPIO_ResetBits(GPIOB, GPIO_Pin_8  | GPIO_Pin_9  | GPIO_Pin_10 | GPIO_Pin_11 |
                        GPIO_Pin_12 | GPIO_Pin_13 );
}

/*******************************************************************************
* Function Name  : DAC_Config
* Description    : Configure the DAC Peripheral.
* Input          :  - AudioFreq: AudioFreq_8K, AudioFreq_16K, AudioFreq_22K,
*                                AudioFreq_44K or AudioFreq_48K
* Output         : None
* Return         : None
*******************************************************************************/
static void DAC_Config(u16 AudioFreq)
{
  DAC_InitTypeDef          DAC_InitStructure;
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef        TIM_OCInitStructure;

	/* Enable DAC APB1 clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

  DAC_StructInit(&DAC_InitStructure);
  DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable;
  DAC_Init(DAC_Channel_1, &DAC_InitStructure);
  DAC_Cmd (DAC_Channel_1, ENABLE);

	/* Enable TIM2 APB1 clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  TIM_OCStructInit(&TIM_OCInitStructure);

  /* TIM2 used for timing, the timing period depends on wav file sample rate */
  TIM_TimeBaseStructure.TIM_Prescaler = 0x00;    /* TIM2CLK = 72 MHz */
  TIM_TimeBaseStructure.TIM_Period = TIM2ARRValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
  /* Channel 1 Configuration in Timing mode */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
  TIM_OCInitStructure.TIM_Pulse = 0x0;
  TIM_OC1Init(TIM2, &TIM_OCInitStructure); 
}

/*******************************************************************************
* Function Name  : ReadUnit
* Description    : Reads a number of bytes from the SPI Flash and reorder them
*                  in Big or little endian.
* Input          : - NbrOfBytes : number of bytes to read.
*                    This parameter must be a number between 1 and 4.
*                  - ReadAddr : external memory address to read from.
*                  - Endians : specifies the bytes endianness.
*                    This parameter can be one of the following values:
*                          - LittleEndian
*                          - BigEndian
* Output         : None
* Return         : Bytes read from the SPI Flash.
*******************************************************************************/
static u32 ReadUnit(u8 NbrOfBytes, Endianness BytesFormat)
{
  u32 index = 0;
  u32 Temp = 0;

  if(BytesFormat == LittleEndian)
  {
    for(index = 0; index < NbrOfBytes; index++)
    {
      Temp |= AudioFileHeader[HeaderTabIndex++] << (index * 8);
    }
  }
  else
  {
    for(index = NbrOfBytes; index != 0; index--)
    {
      Temp |= AudioFileHeader[HeaderTabIndex++] << ((index-1) * 8);
    }
  }

  return Temp;
}

/*******************************************************************************
* Function Name  : Media_ReadHalfWord
* Description    : Read one half word from the media (SPI_Flash/NOR/NAND memories..)
* Input          : - Offset: the adress offset for read operation
* Output         : None.
* Return         : Data read from the media memory.
*******************************************************************************/
u16 Media_ReadHalfWord(u32 Offset)
{ 
  vu16 tmp;
  Offset <<= 1;

  Media_BufferRead((u8*)(&tmp), AudioFileAddress + Offset, 2);  

  /* Return the read value */
  return tmp;
}

/*******************************************************************************
* Function Name  : Media_Init
* Description    : Initialize media (SPI_Flash/NOR/NAND memories..)
* Input          : None.
* Output         : None.
* Return         : - 0 if initialization is OK
*                  - 1 if initialization failed..
*******************************************************************************/
u32 Media_Init(void)
{ 
  /* Initialize the SPI FLASH driver */
  SPI_FLASH_Init();

  return 0;
}

/*******************************************************************************
* Function Name  : Media_BufferRead
* Description    : Read a buffer from the memory media 
* Input          : - pBuffer: Destination buffer address
*                : - ReadAddr: start reading position
*                : - NumByteToRead: size of the buffer to read
* Output         : None.
* Return         : None.
*******************************************************************************/
void Media_BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{ 
  SPI_FLASH_BufferRead(pBuffer, ReadAddr, NumByteToRead);
}

/*******************************************************************************
* Function Name  : Media_StartReadSequence
* Description    : Initialize reading sequence on the media.
* Input          : - ReadAddr: start reading position
* Output         : None.
* Return         : None.
*******************************************************************************/
void Media_StartReadSequence(u32 ReadAddr)
{
  /* This function could be used for memories needing a start read sequence
      like SPI_Flash memory */ 
}

/*******************************************************************************
* Function Name  : DAC_CODEC_DataTransfer
* Description    : Sets the audio data using DAC peripheral and checks the 
*                :   audio playing status (if a command (Pause/Stop) is pending 
*                :   the playing status is updated). If the interrupt 
*                :   is used to synchronize data sending, this function should be 
*                :   called in the TIM2 ISR.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void DAC_CODEC_DataTransfer(void)
{
  static int data = 0;

  DAC_SetChannel1Data (DAC_Align_12b_L, data);

  /* Increment the index */
  IncrementVar_AudioDataIndex(WAVE_Format.NumChannels << 1); 

  Media_BufferRead((u8*)(&data), AudioFileAddress + AudioDataIndex, 2);  

  /* Offset data for half of DAC area */
  data +=  1 << 15;

  /* Check and update the stream playing status */
  DAC_CODEC_UpdateStatus();
}

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
