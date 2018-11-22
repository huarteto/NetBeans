/******************** (C) COPYRIGHT 2005 STMicroelectronics ********************
* File Name          : temperature.c
* Author             : MCD Application Team
* Date First Issued  : 09/27/2005 :  V1.0
* Description        : This file provides functions to display the temperature 
*                      measured by the LM35 sensor. 
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
#define TemperatureRefresh   1600

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
GPIO_InitTypeDef  GPIO3_InitStructure;
ADC_InitTypeDef   ADC_InitStructure;
vu8  TemperatureUnit = 0;
vu16 TemperatureDelay = 0;

struct {
  u8 hundreds;
  u8 tens;
  u8 ones;
  u8 deci;
}digit_temp;
 
/* Private function prototypes -----------------------------------------------*/
/* Interface functions -------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : Converter_Init
* Description    : Initialzes the peripherals used to measure the temperature.  
* Input          : None
* Output         : None   
* Return         : None
*******************************************************************************/
void Converter_Init(void)
{
  /* P3.0 & P3.1 configuration */
  GPIO3_InitStructure.GPIO_Mode = GPIO_Mode_HI_AIN_TRI;
  GPIO3_InitStructure.GPIO_Pins = GPIO_PIN_0 | GPIO_PIN_1 ;
  GPIO_Init(GPIO3, &GPIO3_InitStructure);
   
  /* ADC Initialization */
  ADC_InitStructure.ADC_Calibration = ADC_Calibration_ON;
  ADC_InitStructure.ADC_CalibAverage = ADC_CalibAverage_Enable;
  ADC_InitStructure.ADC_AutoClockOff = ADC_AutoClockOff_Disable ;
  ADC_InitStructure.ADC_ConversionMode = ADC_ConversionMode_Scan ;
  ADC_InitStructure.ADC_SamplingPrescaler = 0x2 ;
  ADC_InitStructure.ADC_ConversionPrescaler = 0x4 ;
  ADC_InitStructure.ADC_FirstChannel = ADC_CHANNEL0;
  ADC_InitStructure.ADC_ChannelNumber = 2;
  ADC_DeInit();
  ADC_Init(&ADC_InitStructure);  
}

/*******************************************************************************
* Function Name  : Converter_DeInit
* Description    : Stops temperature display on the LCD and disables the ADC. 
* Input          : None
* Output         : None  
* Return         : None
*******************************************************************************/
void Converter_DeInit(void)
{ 
  /* disable ADC */
  ADC_Cmd(DISABLE);  
  
  LCD_LineClear(2);
}

/*******************************************************************************
* Function Name  : Send_digits
* Description    : Displays temperature digits on LCD.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Send_digits(u16 temp, u8 Row, u8 Col)
{
  u16 temp1 = 0, temp2 = 0, temp3 = 0;

  digit_temp.deci = temp & 1;
  digit_temp.deci *= 10;
  digit_temp.deci >>= 1;
  temp = temp >> 1; 
  digit_temp.tens = temp / 10;
  digit_temp.ones = temp % 10;

  if(TemperatureUnit == Fahrenheit)
  {
    digit_temp.deci = (digit_temp.deci*9/5);   
    temp1 = digit_temp.ones + (digit_temp.tens*10);
    temp1 = (((temp1*90)/5)+320);   
    digit_temp.hundreds = temp1 / 1000;    
    temp2 = temp1 % 1000;
    digit_temp.tens = temp2 / 100;
    temp3 = temp2 % 100;
    digit_temp.ones = temp3 / 10; 
    temp1 = temp3 % 10;
    
    digit_temp.deci = digit_temp.deci + temp1; 
    digit_temp.ones = digit_temp.ones + (digit_temp.deci / 10);
    digit_temp.tens = digit_temp.tens + (digit_temp.ones / 10); 
    digit_temp.hundreds = digit_temp.hundreds + (digit_temp.tens / 10);
    digit_temp.deci = digit_temp.deci % 10;
    digit_temp.ones = digit_temp.ones % 10;
    digit_temp.tens = digit_temp.tens % 10;
    digit_temp.hundreds = digit_temp.hundreds % 10;
  }

  GPIO_BitWrite(GPIO0, LCD_PIN_RS, Bit_SET);

  if(digit_temp.hundreds != 0)
  {
    LCD_SendData(digit_temp.hundreds+48); /* show hundreds */
  }

  LCD_SetPosCur(Row, Col);
  LCD_SendData(digit_temp.tens+48); /* show tens */
  LCD_SendData(digit_temp.ones+48); /* show ones */
  LCD_SendData(0x2e);/* show point "." */
  LCD_SendData(digit_temp.deci+48); /* show deci */
  LCD_SendData(32);   /* show point " " */
  LCD_SendData(0xdF); /* show point "°" */
}

/*******************************************************************************
* Function Name  : CelsiusDisplay
* Description    : Displays temperature in celsius.
* Input          : None
* Output         : TemperatureUnit: specifies the temperature display unit (used
*                  by the Send_digits function).
* Return         : None
*******************************************************************************/
void CelsiusDisplay(void)
{
  /* set temperature unit variable */  
  TemperatureUnit = Celsius;
  
  /* enable ADC */
  ADC_Cmd(ENABLE);

  /* start ADC conversion */
  ADC_ConversionCmd(ADC_ConversionStart);

  /* display temperature on LCD */
  Temperature_Display();
}

/*******************************************************************************
* Function Name  : FahrenheitDisplay
* Description    : Displays temperature in Fahrenheit.
* Input          : None
* Output         : TemperatureUnit: specifies the temperature display unit (used
*                  by the Send_digits function).   
* Return         : None
*******************************************************************************/
void FahrenheitDisplay(void)
{
  /* set temperature unit variable */
  TemperatureUnit = Fahrenheit;
  
  /* enable ADC */
  ADC_Cmd(ENABLE);

  /* start ADC conversion */
  ADC_ConversionCmd(ADC_ConversionStart); 

  /* display temperature on LCD */
  Temperature_Display();

}

/*******************************************************************************
* Function Name  : Temperature_Display
* Description    : Displays temperature value on LCD. The temperature value is 
*                  refreshed every 200 ms.
* Input          : None
* Output         : PeriodmeasureDelay: incremented each 125 µs in TB1 interrupt
*                  handler routine.   
* Return         : None
*******************************************************************************/
void Temperature_Display(void)
{
  /* disable screen saver mode */	
  WDG_Cmd(DISABLE);

  while(ReadKey() != BACK)
  {
    /* display temperature value on LCD */
    LCD_SetPosCur(2, 7); 
    Send_digits(ADC_GetConversionValue( ADC_CHANNEL1), 2 ,8);
    
    /* wait 200 ms */ 
    TemperatureDelay = 0;
    while(TemperatureDelay < TemperatureRefresh); 
  }

  /* enable screen saver mode */
  WDG_Cmd(ENABLE);
}

/******************* (C) COPYRIGHT 2005 STMicroelectronics *****END OF FILE****/
