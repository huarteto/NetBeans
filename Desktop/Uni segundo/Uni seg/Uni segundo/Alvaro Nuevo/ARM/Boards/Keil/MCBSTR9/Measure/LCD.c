/*----------------------------------------------------------------------------
 *      Name:    LCD.c
 *      Purpose: LCD Functions for 2 line 16 character Text LCD with ST7066 controller
 *      Version: V1.10
 *----------------------------------------------------------------------------
 *      This file is part of the uVision/ARM development tools.
 *      This software may only be used under the terms of a valid, current,
 *      end user licence from KEIL for a compatible version of KEIL software
 *      development tools. Nothing else gives you the right to use it.
 *
 *      Copyright (c) 2005-2007 Keil Software.
 *---------------------------------------------------------------------------*/

#include <91x_lib.h>

/* 8 characters                                                               */
extern const unsigned char UserFont[8][8];


/************************** Text LCD size definitions *************************/

/* Width (in character) controlled by ST7066 controller                       */ 
#define LineLen     16                      

/* Hight (in lines) controlled by ST7066 controller                           */ 
#define NumLines     2                      

/************************** Text LCD size definitions *************************/

#define LCD_DATA     GPIO8->DR[0x3FC]   /* Data bits D0 = P8.0 .. DB7 = P8.7  */
#define LCD_E        GPIO9->DR[0x004]   /* Enable control                     */
#define LCD_RW       GPIO9->DR[0x008]   /* Read/Write control                 */
#define LCD_RS       GPIO9->DR[0x010]   /* Data/Instruction control           */
#define LCD_CTRL     GPIO9->DR[0x01C]   /* All 3 control lines (E/RW/RS)      */

#define LCD_DISABLE  0x00
#define LCD_ENABLE   0x01
#define LCD_WRITE    0x00
#define LCD_READ     0x02
#define LCD_INST_REG 0x00
#define LCD_DATA_REG 0x04


/************************ Global function definitions *************************/


/*******************************************************************************
* Delay in while loop cycles                                                   *
*   Parameter:    cnt:    number of while cycles to delay                      *
*   Return:                                                                    *
*******************************************************************************/

static void delay (int cnt)
{
  while (cnt--);
}


/*******************************************************************************
* Read status of LCD controller (ST7066)                                       *
*   Parameter:    c:      command to be written                                *
*   Return:       Status byte contains busy flag and address pointer           *
*******************************************************************************/

static unsigned char lcd_read_status (void)
{
  unsigned char status;

  GPIO8->DDR = 0x00;                    /* P8.0..7 Inputs (LCD Data)          */
  LCD_RW     = LCD_READ;
  LCD_RS     = LCD_INST_REG;            /* Instruction                        */
  LCD_E      = LCD_ENABLE;
  delay(10);
#ifndef SIMULATOR
  status     = LCD_DATA;
#else
  status     = 0;
#endif
  LCD_E      = LCD_DISABLE;
  delay(10);
  return (status);
}


/*******************************************************************************
* Wait until LCD controller (ST7066) busy flag is 0                            *
*   Parameter:                                                                 *
*   Return:       Status bye of LCD controller (busy + address)                *
*******************************************************************************/

static unsigned char wait_while_busy (void)
{
  unsigned char status;

  do  {
    status = lcd_read_status();
  }  while (status & 0x80);             /* Wait for busy flag                 */

  return (status);
}


/*******************************************************************************
* Write command to LCD controller (ST7066)                                     *
*   Parameter:    c:      command to be written                                *
*   Return:                                                                    *
*******************************************************************************/

void lcd_write_cmd (unsigned char c)
{
  wait_while_busy();

  GPIO8->DDR = 0xFF;                    /* P8.0..7 Outputs (LCD Data)         */
  LCD_RW     = LCD_WRITE;
  LCD_RS     = LCD_INST_REG;            /* Instruction                        */
  LCD_E      = LCD_ENABLE;
  LCD_DATA   = c;
  delay(10);
  LCD_E      = LCD_DISABLE;
  delay(10);
}


/*******************************************************************************
* Write data to LCD controller (ST7066)                                        *
*   Parameter:    c:      data to be written                                   *
*   Return:                                                                    *
*******************************************************************************/

static void lcd_write_data (unsigned char c)
{
  GPIO8->DDR = 0xFF;                    /* P8.0..7 Outputs (LCD Data)         */
  LCD_RW     = LCD_WRITE;
  LCD_RS     = LCD_DATA_REG;            /* Data                               */
  LCD_E      = LCD_ENABLE;
  LCD_DATA   = c;
  delay(10);
  LCD_E      = LCD_DISABLE;
  delay(10);
}


/*******************************************************************************
* Print Character to current cursor position                                   *
*   Parameter:    c:      character to be printed                              *
*   Return:                                                                    *
*******************************************************************************/

void lcd_putchar (char c)
{ 
  wait_while_busy();
  lcd_write_data (c);
}


/*******************************************************************************
* Initialize the ST7066 LCD controller                                         *
*   Parameter:                                                                 *
*   Return:                                                                    *
*******************************************************************************/

void lcd_init (void)
{ 
  int i;
  unsigned char const *p;

  /* Reset controller                                                         */
  lcd_write_cmd(0x38);                  /* Function set: 8bit / 2 lines / 5x8 */
  lcd_write_cmd(0x0F);                  /* Display control: Disp/Curs/Blnk=ON */
  lcd_write_cmd(0x01);                  /* Display clear                      */
  lcd_write_cmd(0x06);                  /* Entry mode: Move right, no shift   */

  /* Load user-specific characters into CGRAM                                 */
  lcd_write_cmd(0x40);                  /* Set CGRAM address counter to 0     */
  p = &UserFont[0][0];
  for (i = 0; i < sizeof(UserFont); i++, p++)  {
    lcd_putchar (*p);
  }
  lcd_write_cmd(0x80);                  /* Set DDRAM address counter to 0     */
}


/*******************************************************************************
* Clear the LCD display                                                        *
*   Parameter:                                                                 *
*   Return:                                                                    *
*******************************************************************************/

void lcd_clear (void)
{
  lcd_write_cmd(0x01);                  /* Display clear                      */
}


/*******************************************************************************
* Set cursor position on LCD display                                           *
*   Parameter:    column: column position 0-15                                 *
*                 line:   line position 0-1                                    *
*   Return:                                                                    *
*******************************************************************************/

void set_cursor (unsigned char column, unsigned char line)
{
  unsigned char address;

  address = (line * 40) + column;
  address = 0x80 + (address & 0x7F);
  lcd_write_cmd(address);               /* Set DDRAM address counter to 0     */
}


/*******************************************************************************
* Print sting to LCD display                                                   *
*   Parameter:    string: pointer to output string                             *
*   Return:                                                                    *
*******************************************************************************/

void lcd_print (unsigned char const *string)
{
  while (*string)  {
    lcd_putchar (*string++);
  }
}
