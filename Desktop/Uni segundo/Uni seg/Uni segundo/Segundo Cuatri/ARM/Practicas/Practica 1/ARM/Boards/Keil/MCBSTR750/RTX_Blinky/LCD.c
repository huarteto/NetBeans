/*----------------------------------------------------------------------------
 *      RL-ARM - Library
 *----------------------------------------------------------------------------
 *      Name:    LCD.C
 *      Purpose: LCD module 2x16 driver for ST7066 controller
 *      Rev.:    V4.05
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2009 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include <RTL.h>
#include <75x_lib.h>                    /* STR750 definitions                */
#include "LCD.h"

/* LCD IO definitions */
/* PINS: 
   - DB4 = P2.08
   - DB5 = P2.09
   - DB6 = P2.10
   - DB7 = P2.11
   - E   = P2.12
   - RW  = P2.13
   - RS  = P2.14                                                              */

#define PIN_E                 0x1000
#define PIN_RW                0x2000
#define PIN_RS                0x4000
#define PINS_CTRL             0x7000
#define PINS_DATA             0x0F00
#define PINS_ALL              0x7F00

/* pin E  setting to 0 or 1                                                   */
#define LCD_E(x)              GPIO2->PD = (GPIO2->PD & ~PIN_E)  | ((x) << 12)

/* pin RW setting to 0 or 1                                                   */
#define LCD_RW(x)             GPIO2->PD = (GPIO2->PD & ~PIN_RW) | ((x) << 13)

/* pin RS setting to 0 or 1                                                   */
#define LCD_RS(x)             GPIO2->PD = (GPIO2->PD & ~PIN_RS) | ((x) << 14)

/* Reading DATA pins                                                          */
#define LCD_DATA_IN           (((GPIO2->PD & PINS_DATA) >> 8) & 0x0F)

/* Writing value to DATA pins                                                 */
#define LCD_DATA_OUT(x)       GPIO2->PD = (GPIO2->PD & ~PINS_DATA) | ((x) << 8)

/* Setting all pins to output mode                                            */
#define LCD_ALL_DIR_OUT       GPIO2->PC0 |=  PINS_ALL; \
                              GPIO2->PC1 &= ~PINS_ALL; \
                              GPIO2->PC2 |=  PINS_ALL; \
                              GPIO2->PM  &= ~PINS_ALL; \
                              GPIO2->PD  &= ~PINS_ALL;
 
/* Setting DATA pins to input mode                                            */
#define LCD_DATA_DIR_IN       GPIO2->PC0 |=  PINS_DATA; \
                              GPIO2->PC1 &= ~PINS_DATA; \
                              GPIO2->PC2 &= ~PINS_DATA;

/* Setting DATA pins to output mode                                           */
#define LCD_DATA_DIR_OUT      GPIO2->PC0 |=  PINS_DATA; \
                              GPIO2->PC1 &= ~PINS_DATA; \
                              GPIO2->PC2 |=  PINS_DATA;

/* Local variables */
static U32 lcd_ptr;

/* 8 user defined characters to be loaded into CGRAM (used for bargraph) */
static const U8 UserFont[8][8] = {
  { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
  { 0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10 },
  { 0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18 },
  { 0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C },
  { 0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E },
  { 0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F },
  { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 },
  { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 }
};

/* Local Function Prototypes */
static void delay (U32 cnt);
static void lcd_write (U32 c);
static void lcd_write_4bit (U32 c);
static U32  lcd_rd_stat (void);
static void lcd_wr_cmd (U32 c);
static void lcd_wr_data (U32 d);
static void lcd_wait_busy (void);

/*----------------------------------------------------------------------------
 * LCD Driver Interface Functions
 *---------------------------------------------------------------------------*/


/*--------------------------- delay -----------------------------------------*/

static void delay (U32 cnt) {
  /* Delay in while loop cycles. */

  while (cnt--);
}


/*--------------------------- lcd_write_4bit --------------------------------*/

static void lcd_write_4bit (U32 c) {
  /* Write a 4-bit command to LCD controller. */

  LCD_RW(0);
  LCD_E(1);
  LCD_DATA_OUT(c&0x0F);
  delay(10);
  LCD_E(0);
  delay(10);
}


/*--------------------------- lcd_write -------------------------------------*/

static void lcd_write (U32 c) {
  /* Write data/command to LCD controller. */

  lcd_write_4bit (c >> 4);
  lcd_write_4bit (c);
}


/*--------------------------- lcd_rd_stat -----------------------------------*/

static U32 lcd_rd_stat (void) {
  /* Read status of LCD controller (ST7066) */
  U32 stat;

  LCD_DATA_DIR_IN;
  LCD_RS(0);
  LCD_RW(1);
  delay(10);
  LCD_E(1);
  delay(10);
  stat  = LCD_DATA_IN << 4;
  LCD_E(0);
  delay(10);
  LCD_E(1);
  delay(10);
  stat |= LCD_DATA_IN;
  LCD_E(0);
  LCD_DATA_DIR_OUT;
  return (stat);
}


/*--------------------------- lcd_wait_busy ---------------------------------*/

static void lcd_wait_busy (void) {
  /* Wait until LCD controller (ST7066) is busy. */
  U32 stat;

  do {
    stat = lcd_rd_stat ();
  } while (stat & 0x80);                /* Wait for busy flag                */
}


/*--------------------------- lcd_wr_cmd ------------------------------------*/

static void lcd_wr_cmd (U32 c) {
  /* Write command to LCD controller. */

  lcd_wait_busy ();
  LCD_RS(0);
  lcd_write (c);
}


/*--------------------------- lcd_wr_data -----------------------------------*/

static void lcd_wr_data (U32 d) {
  /* Write data to LCD controller. */

  lcd_wait_busy ();
  LCD_RS(1);
  lcd_write (d);
}


/*--------------------------- LCD_init --------------------------------------*/

void LCD_init (void) {
  /* Initialize the ST7066 LCD controller to 4-bit mode. */ 

  LCD_ALL_DIR_OUT;
  LCD_RS(0);

  lcd_write_4bit (0x3);                 /* Select 4-bit interface            */
  delay (100000);
  lcd_write_4bit (0x3);
  delay (10000);
  lcd_write_4bit (0x3);
  lcd_write_4bit (0x2);

  lcd_wr_cmd (0x28);                    /* 2 lines, 5x8 character matrix     */
  lcd_wr_cmd (0x0e);                    /* Display ctrl:Disp/Curs/Blnk=ON    */
  lcd_wr_cmd (0x06);                    /* Entry mode: Move right, no shift  */

  LCD_load ((U8 *)&UserFont, sizeof (UserFont));
  LCD_cls ();
}


/*--------------------------- LCD_load --------------------------------------*/

void LCD_load (U8 *fp, U32 cnt) {
  /* Load user-specific characters into CGRAM */
  U32 i;

  lcd_wr_cmd (0x40);                    /* Set CGRAM address counter to 0    */
  for (i = 0; i < cnt; i++, fp++)  {
    lcd_wr_data (*fp);
  }
}

/*--------------------------- LCD_gotoxy ------------------------------------*/

void LCD_gotoxy (U32 x, U32 y) {
  /* Set cursor position on LCD display. Left corner: 1,1, right: 16,2 */
  U32 c;

  c = --x;
  if (--y) {
    c |= 0x40;
  }
  lcd_wr_cmd (c | 0x80);
  lcd_ptr = y*16 + x;
}


/*--------------------------- LCD_cls ---------------------------------------*/

void LCD_cls (void) {
  /* Clear LCD display, move cursor to home position. */

  lcd_wr_cmd (0x01);
  LCD_gotoxy (1,1);
}


/*--------------------------- LCD_cur_off------------------------------------*/

void LCD_cur_off (void) {
  /* Switch off LCD cursor. */

  lcd_wr_cmd (0x0c);
}


/*--------------------------- LCD_on ------ ---------------------------------*/

void LCD_on (void) {
  /* Switch on LCD and enable cursor. */

  lcd_wr_cmd (0x0e);
}


/*--------------------------- LCD_putc --------------------------------------*/

void LCD_putc (U8 c) { 
  /* Print a character to LCD at current cursor position. */

  if (lcd_ptr == 16) {
    lcd_wr_cmd (0xc0);
  }
  lcd_wr_data (c);
  lcd_ptr++;
}


/*--------------------------- LCD_puts --------------------------------------*/

void LCD_puts (U8 *sp) {
  /* Print a string to LCD display. */

  while (*sp) {
    LCD_putc (*sp++);
  }
}


/*--------------------------- LCD_bargraph ----------------------------------*/

void LCD_bargraph (U32 val, U32 size) {
  /* Print a bargraph to LCD display.  */
  /* - val:  value 0..100 %            */
  /* - size: size of bargraph 1..16    */
  U32 i;

  val = val * size / 20;                /* Display matrix 5 x 8 pixels       */
  for (i = 0; i < size; i++) {
    if (val > 5) {
      LCD_putc (5);
      val -= 5;
    }
    else {
      LCD_putc (val);
      break;
    }
  }
}

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/
