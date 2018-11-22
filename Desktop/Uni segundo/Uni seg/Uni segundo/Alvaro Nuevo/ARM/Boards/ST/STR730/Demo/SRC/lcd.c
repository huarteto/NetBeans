/******************** (C) COPYRIGHT 2005 STMicroelectronics ********************
* File Name          : lcd.c
* Author             : MCD Application Team
* Date First Issued  : 09/27/2005 :  V1.0
* Description        : This file provides a set of functions needed to manage the
*                      LCD connected to P0.0..10 pins.
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

/* Standard includes -------------------------------------------------------- */
#include "main.h"

/* Include of other module interface headers ---------------------------------*/
/* Local includes ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
GPIO_InitTypeDef GPIO_InitStructure0;
vu16 LCDDelay = 0;

/* Private function prototypes -----------------------------------------------*/
/* Interface functions -------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : LCD_Init
* Description    : Initializes the LCD driver.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_Init(void)
{
  /* LCD pins configuration */
  GPIO_InitStructure0.GPIO_Mode = GPIO_Mode_OUT_PP;
  GPIO_InitStructure0.GPIO_Pins = GPIO_PIN_0 | GPIO_PIN_1 |GPIO_PIN_2 | GPIO_PIN_3 |
                                  GPIO_PIN_4 | GPIO_PIN_5 |GPIO_PIN_6 | GPIO_PIN_7 |
                                  GPIO_PIN_8 | GPIO_PIN_9 |GPIO_PIN_10; 
  GPIO_Init(GPIO0, &GPIO_InitStructure0);
   
  /* send "FUNCTION SET 8BIT LONG COMMAND" command */
  LCD_SendCommand(LCD_CMD_8BIT);
    
  /* send "FUNCTION SET 8BIT LONG COMMAND" command */
  LCD_SendCommand(LCD_CMD_8BIT);
 
  /* send "FUNCTION SET 8BIT LONG COMMAND" command */
  LCD_SendCommand(LCD_CMD_8BIT);
 
  /* send "FUNCTION SET COMMAND" command: 8BIT MODE, 2 LINES, 5*8DOT FONT */
  LCD_SendCommand(LCD_CMD_8BIT_2LINE_8DOT);
 
  /* send "DISPLAY CLEAR COMMAND" command */
  LCD_SendCommand(LCD_CMD_CLR_DISP);

  /* wait 5 ms */
  LCD_Wait(40); 
  
  /* send "ENTRY MODE COMMAND" command: INCREMENT CURSOR, NO SHIFT */
  LCD_SendCommand(LCD_CMD_INCREM_NOSHIFT);
  
  /* send "DISPLAY ON/OFF CONTROL" command: DISPLAY ON, CURSOR OFF, BLINK OFF */
  LCD_SendCommand(LCD_CMD_DISPON_CURSOFF_BLKOFF); 
}

/*******************************************************************************
* Function Name  : LCD_E_High
* Description    : Puts the LCD Enable line (E) high.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_E_High(void)
{
  GPIO_BitWrite(GPIO0, LCD_PIN_E, Bit_SET);
}

/*******************************************************************************
* Function Name  : LCD_E_Low
* Description    : Puts the LCD Enable line (E) low.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_E_Low(void)
{ 
  GPIO_BitWrite(GPIO0, LCD_PIN_E, Bit_RESET);
}

/*******************************************************************************
* Function Name  : LCD_SendCommand
* Description    : Sends a command to the LCD.
* Input          : Cmd: command to be sent.
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_SendCommand(u8 Cmd)
{ 
  /* reset RW bit */
  GPIO_BitWrite(GPIO0, LCD_PIN_RW, Bit_RESET); 

  /* reset RS bit */
  GPIO_BitWrite(GPIO0, LCD_PIN_RS, Bit_RESET);

  /* send the command */
  GPIO_ByteWrite(GPIO0, GPIO_LSB, Cmd);

  /* enable read/write to the LCD */
   LCD_E_High();
    
   /* wait 125 탎 */
   LCD_Wait(8);
   
  /* disable read/write to the LCD */
   LCD_E_Low(); 
}

/*******************************************************************************
* Function Name  : LCD_SendData
* Description    : Sends data to the LCD.
* Input          : Data: data to be displayed.
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_SendData(u8 Data)
{ 
  /* reset RW bit */
  GPIO_BitWrite(GPIO0, LCD_PIN_RW, Bit_RESET);
  
  /* set RS bit */
  GPIO_BitWrite(GPIO0, LCD_PIN_RS, Bit_SET);

  /* send data */
  GPIO_ByteWrite(GPIO0, GPIO_LSB, Data);

  /* enable read/write to the LCD */
   LCD_E_High();

   /* wait 125 탎 */
   LCD_Wait(1);
 
  /* disable read/write to the LCD */
   LCD_E_Low(); 
}

/*******************************************************************************
* Function Name  : LCD_SendString
* Description    : Displays a string on the LCD.
* Input          : - pBuffer: pointer to the buffer containing the data to be 
*                    displayed on the LCD.
*                  - Line: specifies the line where the string will be displayed.
*                  - Column: specifies the start column from where the string will 
*                    be displayed.
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_SendString(u8 *pBuffer, u8 Line, u8 Column)
{
  u8 len = 0, count = 0;
  
  len = strlen(( const char *)pBuffer);     
 
  if((len < (LCD_HALF_LENGTH + 2 - Column)))
  { 
    LCD_SetPosCur(Line, Column);
           
    for(count =0; count < len; count++)            
    {
      LCD_SendData(pBuffer[count]);            
    }   
  }
  else 
  {
    LCD_SetPosCur(Line, Column);
                      
    for(count = 0; count <= (LCD_HALF_LENGTH-Column); count++)
    {
      LCD_SendData(pBuffer[count]); 
    }

    LCD_SetPosCur(2, 1);
            
    for(count = (LCD_HALF_LENGTH-Column+1); count < len; count++)
    {
      LCD_SendData(pBuffer[count]); 
    }
  }
}

/*******************************************************************************
* Function Name  : LCD_SendStringByStep
* Description    : Displays a string on the LCD by step.
* Input          : - pBuffer: pointer to the buffer containing the data to be 
*                    displayed on the LCD.
*                  - Line: specifies the line where the string will be displayed.
*                  - Column: specifies the start column from where the string will 
*                    be displayed.
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_SendStringByStep(u8 *pBuffer, u8 Line, u8 Column)
{
  u8 len = 0, count = 0;
    
  len = strlen(( const char *)pBuffer);     
 
  if(len < (LCD_HALF_LENGTH + 2 - Column))
  { 
    LCD_SetPosCur(Line, Column);
            
    for(count =0; count < len; count++)            
    {
      /* wait 120 ms */
      LCD_Wait(960);
      LCD_SendData(pBuffer[count]);            
    }  
  }
  else 
  {
    LCD_SetPosCur(Line, Column);
                      
    for(count = 0; count <= (LCD_HALF_LENGTH-Column); count++)
    {
      /* wait 120 ms */
      LCD_Wait(960); 
      LCD_SendData(pBuffer[count]); 
    }

    LCD_SetPosCur(2, 1);
            
    for(count = (LCD_HALF_LENGTH-Column+1); count < len; count++)
    {
      /* wait 120 ms */
      LCD_Wait(960); 
      LCD_SendData(pBuffer[count]); 
    }
  }
}

/*******************************************************************************
* Function Name  : LCD_SendStringPinPong
* Description    : Displays a string on the LCD as ping pong.
* Input          : - pBuffer: pointer to the buffer containing the data to be 
*                    displayed on the LCD.
*                  - Line: specifies the line where the string will be displayed.
*                  - nTime: number of ping pong.
* Output         :
* Return         : None
*******************************************************************************/
void LCD_SendStringPinPong(u8 *pBuffer, u8 Line, u32 nTime)
{
  u8 len = 0, index = 1;
  bool Direction = TRUE;
     
  len = strlen(( const char *)pBuffer); 
   
  while(nTime--)
  {
    if(Direction)
    {      
      LCD_SendString(pBuffer, Line, index);
      /* wait 120 ms */
      LCD_Wait(960);  
      index++;  
      LCD_LineClear(Line);                  
      if(index == (LCD_HALF_LENGTH - len) + 1)
      {
        Direction = FALSE;
      }
    }
    else
    {      
      LCD_SendString(pBuffer, Line, index);
      /* wait 120 ms */
      LCD_Wait(960);  
      index--;    
      LCD_LineClear(Line);                 
      if(index == 1)
      {
        Direction = TRUE;
      }
    }
  }
}

/*******************************************************************************
* Function Name  : LCD_SetPosCur
* Description    : Sets the line and column position of the LCD cursor.
* Input          : - Line: specifies the cursor's line position.
*                  - Column: specifies cursor's column position.
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_SetPosCur(u8 Line, u8 Column)
{
  if(Line == 1)
  {
    Column = Column - 1;
  }
  else
  {
    Column = Column -1  + 64; 
  }
  
  LCD_SendCommand(Column | 0x80);
}

/*******************************************************************************
* Function Name  : LCD_Clear
* Description    : Clears the LCD display.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_Clear(void)
{
  /* reset RS bit */
  GPIO_BitWrite(GPIO0, LCD_PIN_RS, Bit_RESET);    
  
  /* send "DISPLAY CLEAR COMMAND" command */
  LCD_SendCommand(LCD_CMD_CLR_DISP);
         
  /* wait 4 ms */
  LCD_Wait(32);  
}

/*******************************************************************************
* Function Name  : LCD_LineClear
* Description    : Clears the LCD specified line.
* Input          : Line: line to be cleared.
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_LineClear(u8 Line)
{       
  u8 j = 0;
    
  /* reset RS bit */
  GPIO_BitWrite(GPIO0, LCD_PIN_RS, Bit_RESET);
  
  LCD_SetPosCur(Line, 1);     
  
  /* set RS bit */
  GPIO_BitWrite(GPIO0, LCD_PIN_RS, Bit_SET);      
  
  /* write LCD_HALF_LENGTH spaces to clear the line */
  for(j = LCD_HALF_LENGTH; j != 0; j--)
  {
    LCD_SendData(' ');
  }
}

/*******************************************************************************
* Function Name  : LCD_DigitDisplay
* Description    : Displays a digit on the LCD.
* Input          : - Digit: digit to be displayed.
*                  - Line: specifies the line where the digit will be displayed.
*                  - Column: specifies the start column from where the digit will 
*                    be displayed.
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_DigitDisplay(u8 Digit, u8 Line, u8 Column)
{
  LCD_SetPosCur(Line, Column);

  LCD_SendData(Digit + 48);
}

/*******************************************************************************
* Function Name  : LCD_Wait
* Description    : Inserts a delay.
* Input          : nTime: specifies the time-out value, in step of 125 
*                  microsecondes (탎).
* Output         : LCDDelay: incremented each 125 탎 in TB1 interrupt handler
*                  routine and used to define the time-out value.
* Return         : None
*******************************************************************************/
void LCD_Wait(u16 nTime) 
{
  LCDDelay = 0;
  while(LCDDelay < nTime);
}

/******************* (C) COPYRIGHT 2005 STMicroelectronics *****END OF FILE****/
