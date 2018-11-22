/******************************************************************************/
/* GLCD.c: Functions for I2C Graphic LCD Module (96 x 40 pixels)              */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/


#include "LPC318x.h"                    /* LPC3000 definitions                */
#include "Charset_8x6.h"                /* Character definitions              */


/*************************** System dependant settings ************************/

/* Frequency of the main clock (needed for calculation of delays for LCD)     */
#define FREQ_PERIPH  13000              /* in kHz                             */


/****************************** Time constants ********************************/

#define DELAY        (FREQ_PERIPH/20)   /* Delay depending on peripheral freq */


/*******************************************************************************
* Delay in while loop cycles (each cycle is 6 instruction cycles long)         *
*   Parameter:    cnt:    number of while cycles to delay                      *
*   Return:                                                                    *
*******************************************************************************/

static void delay(int delay)
{
  while (delay--);
}


/*******************************************************************************
* Write data to I2C graphic display                                            *
*   Parameter:   data:    data to be written                                   *
*   Return:                                                                    *
*******************************************************************************/

static void i2c_write (unsigned int data)
{
  I2C0_TX = data;
  delay(DELAY);
}


/*******************************************************************************
* Set cursor to a line beginning                                               *
*   Parameter:   line:    line number where cursor will be positioned          *
*   Return:                                                                    *
*******************************************************************************/

void set_line (unsigned char line)
{
  i2c_write(0x017A);
  i2c_write(0x60+line);
  i2c_write(0x0000);
}


/*******************************************************************************
* Initialize the graphic display                                               *
*   Parameter:                                                                 *
*   Return:                                                                    *
*******************************************************************************/

void lcd_init (void)
{
  I2CCLK_CTRL      = 0x01;
  I2C0_CLKH        = 0x41;
  I2C0_CLKL        = 0x41;
}


/*******************************************************************************
* Print character to current cursor position                                   *
*   Parameter:    c:      character to be printed                              *
*   Return:                                                                    *
*******************************************************************************/

void lcd_putchar (char c)
{
  int i;
  for (i = 0; i < 6; i++)  {
    i2c_write(charset[(c-32)][i]);
  }
}


/*******************************************************************************
* Print sting to the graphic display                                           *
*   Parameter:    string: pointer to output string                             *
*                   line: line where string will be displayed                  *
*   Return:                                                                    *
*******************************************************************************/

void lcd_print (unsigned char const *string, unsigned char line)
{
  set_line (line);
  while (*string != 0)  {
    lcd_putchar(*string++);
  }
}


/*******************************************************************************
* Clear the LCD display                                                        *
*   Parameter:                                                                 *
*   Return:                                                                    *
*******************************************************************************/

void lcd_clear (void)
{
  lcd_print ("               ", 0);
  lcd_print ("               ", 1);
  lcd_print ("               ", 2);
  lcd_print ("               ", 3);
  lcd_print ("               ", 4);
  set_line(0);
}

