/*----------------------------------------------------------------------------
 *      R T L   K e r n e l   E x a m p l e
 *----------------------------------------------------------------------------
 *      Name:    LCD_4bit.c
 *      Purpose: LCD module 2x16 driver, driven by 4-bit interface
 *      Rev.:    V3.20
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2008 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include <RTL.h>
#include "TMS470R1.h"                   /* TMS470R1 definitions               */
#include "LCD.h"

/*********************** Hardware specific configuration **********************/

/*------------------------- Speed dependant settings -------------------------*/

/* If processor works on high frequency delay has to be increased, it can be 
   increased by factor 2^N by this constant                                   */
#define DELAY_2N     6

/*------------------------- Text LCD size definitions ------------------------*/

#define LineLen     16                  /* Width (in characters)              */
#define NumLines     2                  /* Hight (in lines)                   */

/*-------------------- LCD interface hardware definitions --------------------*/

/* PINS: 
   - DB4 = PG.4
   - DB5 = PG.5
   - DB6 = PG.6
   - DB7 = PG.7
   - E   = PG.1
   - RW  = NC   (connected to GND enabling only write to display)
   - RS  = PG.3
   - BL  = PG.2 (backlight)                                                   */


#define PIN_E                 (1    << 1)
#define PIN_RW                (0    << 0)
#define PIN_RS                (1    << 3)
#define PIN_BL                (1    << 2)
#define PINS_CTRL             (0x0F << 0)
#define PINS_DATA             (0x0F << 4)

#define STATUS_DISABLED                 /* Status read is not enabled by pins */

/* pin E  setting to 0 or 1                                                   */
#define LCD_E(x)              ((x) ? (pGIO->GIOG.GIODOUT |= PIN_E)  : (pGIO->GIOG.GIODOUT &= ~PIN_E) ); delay(10);

/* pin RW setting to 0 or 1                                                   */
#define LCD_RW(x)             ((x) ? (pGIO->GIOG.GIODOUT |= PIN_RW) : (pGIO->GIOG.GIODOUT &= ~PIN_RW)); delay(10);

/* pin RS setting to 0 or 1                                                   */
#define LCD_RS(x)             ((x) ? (pGIO->GIOG.GIODOUT |= PIN_RS) : (pGIO->GIOG.GIODOUT &= ~PIN_RS)); delay(10);

/* pin BL (backlight) setting to 0 or 1                                       */
#define LCD_BL(x)             ((x) ? (pGIO->GIOG.GIODOUT |= PIN_BL) : (pGIO->GIOG.GIODOUT &= ~PIN_BL)); delay(10);

/* Reading DATA pins                                                          */
#define LCD_DATA_IN           ((pGIO->GIOG.GIODIN >> 4) & 0xF)

/* Writing value to DATA pins                                                 */
#define LCD_DATA_OUT(x)       pGIO->GIOG.GIODOUT  = (pGIO->GIOG.GIODOUT & ~PINS_DATA) | ((x & 0xF) << 4); delay(10);

/* Setting all pins to output mode                                            */
#define LCD_ALL_DIR_OUT       pGIO->GIOG.GIODIR |=  PINS_CTRL | PINS_DATA; delay(10);

/* Setting DATA pins to input mode                                            */
#define LCD_DATA_DIR_IN       pGIO->GIOG.GIODIR &= ~PINS_DATA; delay(10);

/* Setting DATA pins to output mode                                           */
#define LCD_DATA_DIR_OUT      pGIO->GIOG.GIODIR |=  PINS_DATA; delay(10);

/******************************************************************************/


/* 8 user defined characters to be loaded into CGRAM (used for bargraph)      */
const char UserFont[8][8] = {
  { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
  { 0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10 },
  { 0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18 },
  { 0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C },
  { 0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E },
  { 0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F },
  { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
  { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 }
};


/************************ Local auxiliary functions ***************************/

/*******************************************************************************
* Delay in while loop cycles                                                   *
*   Parameter:    cnt:    number of while cycles to delay                      *
*   Return:                                                                    *
*******************************************************************************/

static void delay (int cnt) {

  cnt <<= DELAY_2N;

  while (cnt--);
}


/*******************************************************************************
* Read status of LCD controller                                                *
*   Parameter:    none                                                         *
*   Return:       Status byte contains busy flag and address pointer           *
*******************************************************************************/

static unsigned char lcd_read_status (void) {
  unsigned char status;

#ifndef STATUS_DISABLED                 /* Read status if enabled             */
  LCD_DATA_DIR_IN
  LCD_RS(0)
  LCD_RW(1)
  delay(10);
  LCD_E(1)
  delay(10);
  status  = LCD_DATA_IN << 4;
  LCD_E(0)
  delay(10);
  LCD_E(1)
  delay(10);
  status |= LCD_DATA_IN;
  LCD_E(0)
  LCD_DATA_DIR_OUT
#else 
  status = 0;
#endif
  return (status);
}


/*******************************************************************************
* Wait until LCD controller busy flag is 0                                     *
*   Parameter:                                                                 *
*   Return:       Status byte of LCD controller (busy + address)               *
*******************************************************************************/

static unsigned char wait_while_busy (void) {
  unsigned char status;

  do  {
    status = lcd_read_status();
  }  while (status & 0x80);             /* Wait for busy flag                 */

  return (status);
}


/*******************************************************************************
* Write 4-bits to LCD controller                                               *
*   Parameter:    c:      command to be written                                *
*   Return:                                                                    *
*******************************************************************************/

void lcd_write_4bit (unsigned char c) {

  LCD_RW(0)
  LCD_E(1)
  LCD_DATA_OUT(c&0x0F)
  delay(10);
  LCD_E(0)
  delay(10);
}


/*******************************************************************************
* Write command to LCD controller                                              *
*   Parameter:    c:      command to be written                                *
*   Return:                                                                    *
*******************************************************************************/

void lcd_write_cmd (unsigned char c) {

  wait_while_busy();

  LCD_RS(0)
  lcd_write_4bit(c>>4);
  lcd_write_4bit(c);
}


/*******************************************************************************
* Write data to LCD controller                                                 *
*   Parameter:    c:      data to be written                                   *
*   Return:                                                                    *
*******************************************************************************/

static void lcd_write_data (unsigned char c) {

  wait_while_busy();

  LCD_RS(1)
  lcd_write_4bit(c>>4);
  lcd_write_4bit(c);
}


/************************ Exported functions **********************************/

/*******************************************************************************
* Initialize the LCD controller                                                *
*   Parameter:                                                                 *
*   Return:                                                                    *
*******************************************************************************/

void LCD_init (void) { 
  int i;
  char const *p;

  LCD_ALL_DIR_OUT                       /* Set all pins for LCD as outputs    */

  delay (15000);
  LCD_RS(0)
  lcd_write_4bit(0x3);                  /* Select 4-bit interface             */
  delay (4100);
  lcd_write_4bit(0x3);
  delay (100);
  lcd_write_4bit(0x3);
  lcd_write_4bit(0x2);

  lcd_write_cmd(0x28);                  /* 2 lines, 5x8 character matrix      */
  lcd_write_cmd(0x0C);                  /* Display ctrl:Disp=ON,Curs/Blnk=OFF */
  lcd_write_cmd(0x06);                  /* Entry mode: Move right, no shift   */

  /* Load user-specific characters into CGRAM                                 */
  lcd_write_cmd(0x40);                  /* Set CGRAM address counter to 0     */
  p = &UserFont[0][0];
  for (i = 0; i < sizeof(UserFont); i++, p++)
    LCD_putc (*p);

  lcd_write_cmd(0x80);                  /* Set DDRAM address counter to 0     */
}


/*******************************************************************************
* Clear the LCD display                                                        *
*   Parameter:                                                                 *
*   Return:                                                                    *
*******************************************************************************/

void LCD_clear (void) {

  lcd_write_cmd(0x01);                  /* Display clear                      */
  LCD_gotoxy(0, 0);
}


/*******************************************************************************
* Cursor control                                                               *
*   Parameter:     state: requested cursor visibility (0 - off, 1 - on)        *
*                  blink: requested cursor blinking (0 - off, 1 - on)          *
*   Return:                                                                    *
*******************************************************************************/

void LCD_cursor (unsigned char state, unsigned char blink) {
  unsigned char c = 0;

  if (state) c |= 1 << 1;
  if (blink) c |= 1 << 0;

  lcd_write_cmd(0x0C | c);
}


/*******************************************************************************
* Backlight control                                                            *
*   Parameter:     state: requested backlight state (0 - off, 1 - on)          *
*   Return:                                                                    *
*******************************************************************************/

void LCD_backlight (unsigned char state) {

  LCD_BL(state);
}


/*******************************************************************************
* Set cursor position on LCD display                                           *
*   Parameter:    column: column position                                      *
*                 line:   line position                                        *
*   Return:                                                                    *
*******************************************************************************/

void LCD_gotoxy (unsigned char x, unsigned char y) {
  unsigned char address;

  address = (y * 0x40) + x;
  address = 0x80 + (address & 0x7F);
  lcd_write_cmd(address);               /* Set DDRAM address counter to 0     */
}


/*******************************************************************************
* Print Character to current cursor position                                   *
*   Parameter:    c:      character to be printed                              *
*   Return:                                                                    *
*******************************************************************************/

void LCD_putc (char c) { 
  
  lcd_write_data(c);
}


/*******************************************************************************
* Print sting to LCD display                                                   *
*   Parameter:    string: pointer to output string                             *
*   Return:                                                                    *
*******************************************************************************/

void LCD_print (unsigned char x, unsigned char y, char *string) {

  LCD_gotoxy(x, y);
  while (*string)
    LCD_putc (*string++);
}


/*******************************************************************************
* Display bargraph on LCD display                                              *
*   Parameter:    string: pointer to output string                             *
*   Return:                                                                    *
*******************************************************************************/

void LCD_bargraph (unsigned char x, unsigned char y, unsigned char size, unsigned char val) {
  char i;

  val = val * size / 20;                /* Scale value for 5x8 characters     */
  LCD_gotoxy(x, y);
  for (i = 0; i < size; i++)  {
    if (val > 5) {
      LCD_putc(0x05);
      val -= 5;
    } else {
      LCD_putc(val);
      val  = 0;
    }
  }
}


/******************************************************************************/
