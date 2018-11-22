/*----------------------------------------------------------------------------
 * Name:    GLCD.c
 * Purpose: ATSAM3U-EK low level Graphic LCD (320x240 pixels) functions
 * Version: V1.00
 * Note(s):
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2009 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------
 * History:
 *          V1.00 Initial Version
 *----------------------------------------------------------------------------*/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <SAM3U.H>                      /* ATSAM3U definitions                */

#include "GLCD.h"
#include "font.h"


/*********************** Hardware specific configuration **********************/

/*------------------------- Speed dependant settings -------------------------*/

/* If processor works on high frequency delay has to be increased, it can be 
   increased by factor 2^N by this constant                                   */
#define DELAY_2N    18

/*---------------------- Graphic LCD size definitions ------------------------*/

#define WIDTH       320                 /* Screen Width (in pixels)           */
#define HEIGHT      240                 /* Screen Hight (in pixels)           */
#define BPP         16                  /* Bits per pixel                     */
#define BYPP        ((BPP+7)/8)         /* Bytes per pixel                    */

/*--------------- Graphic LCD interface hardware definitions -----------------*/

/* Note: LCD /CS is CCS2 */
#define LCD_BASE        (0x62000000UL)
#define LCD_REG16  (*((volatile unsigned short *)(LCD_BASE  ))) 
#define LCD_DAT16  (*((volatile unsigned short *)(LCD_BASE+2)))
 
/*---------------------------- Global variables ------------------------------*/

/******************************************************************************/
static volatile unsigned short TextColor = Black, BackColor = White;
static volatile unsigned short DriverCode;


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
* Write command to LCD controller                                              *
*   Parameter:    c:      command to be written                                *
*   Return:                                                                    *
*******************************************************************************/

static __inline void wr_cmd (unsigned char c) {

  LCD_REG16 = c;
}


/*******************************************************************************
* Write data to LCD controller                                                 *
*   Parameter:    c:      data to be written                                   *
*   Return:                                                                    *
*******************************************************************************/

static __inline void wr_dat (unsigned short c) {

  LCD_DAT16 = c;
}


/*******************************************************************************
* Read data from LCD controller                                                *
*   Parameter:                                                                 *
*   Return:               read data                                            *
*******************************************************************************/

static __inline unsigned short rd_dat (void) {

  return (LCD_DAT16);                   /* return value                       */
}

/*******************************************************************************
* Write to LCD register                                                        *
*   Parameter:    reg:    register to be read                                  *
*                 val:    value to write to register                           *
*******************************************************************************/

static __inline void wr_reg (unsigned char reg, unsigned short val) {

  wr_cmd(reg);
  wr_dat(val);
}


/*******************************************************************************
* Read from LCD register                                                       *
*   Parameter:    reg:    register to be read                                  *
*   Return:               value read from register                             *
*******************************************************************************/

static unsigned short rd_reg (unsigned char reg) {

  wr_cmd(reg);
  return (rd_dat());
}


/************************ Exported functions **********************************/

/*******************************************************************************
* Initialize the Graphic LCD controller                                        *
*   Parameter:                                                                 *
*   Return:                                                                    *
*******************************************************************************/
#define LCD_D0_D14      (0xFE01FE00)
#define LCD_D15         (PIO_PB6)
#define LCD_RS          (PIO_PB8)
#define LCD_RD          (PIO_PB19)
#define LCD_WR          (PIO_PB23)
#define LCD_NCS2        (PIO_PC16)
#define LCD_BACKLIGHT   (PIO_PC19)

void GLCD_init (void) { 
  unsigned int mode, i;

/* Configure the LCD Control pins -------------------------------------------*/
  PMC->PMC_PCER = ((1 << ID_PIOB) |            /* enable clock for PIOB */
                   (1 << ID_PIOC) );           /* enable clock for PIOC */

/* Configure the Data Bus pins ----------------------------------------------*/
   PIOB->PIO_PDR    =  (LCD_D0_D14 | LCD_D15); /* enable Peripheral function */  
   PIOB->PIO_IDR    =  (LCD_D0_D14 | LCD_D15); /* disable interrupt */ 
   PIOB->PIO_ABSR  &= ~(LCD_D0_D14);           /* select Peripheral A */ 
   PIOB->PIO_ABSR  |=  (LCD_D15);              /* select Peripheral B */ 

/* Configure the Control pins -----------------------------------------------*/
   PIOB->PIO_PDR    =  (LCD_RS);               /* enable Peripheral function */ 
   PIOB->PIO_IDR    =  (LCD_RS);               /* disable interrupt */ 
   PIOB->PIO_ABSR  |=  (LCD_RS);               /* select Peripheral B */ 

   PIOB->PIO_PDR    =  (LCD_RD | LCD_WR);      /* enable Peripheral function */ 
   PIOB->PIO_IDR    =  (LCD_RD | LCD_WR);      /* disable interrupt */ 
   PIOB->PIO_ABSR  &= ~(LCD_RD | LCD_WR);      /* select Peripheral A */ 

   PIOC->PIO_PDR    =  (LCD_NCS2);             /* enable Peripheral function */  
   PIOC->PIO_IDR    =  (LCD_NCS2);             /* disable interrupt */ 
   PIOC->PIO_ABSR  &= ~(LCD_NCS2);             /* select Peripheral A */ 

/* Configure the Backlight pins ---------------------------------------------*/
  PIOC->PIO_IDR  =                             /* disable interrupt */
  PIOC->PIO_OER  =                             /* enable output */
  PIOC->PIO_PER  = (LCD_BACKLIGHT);            /* disable Peripheral function */ 
  PIOC->PIO_CODR = (LCD_BACKLIGHT);            /* Backlight off */

/*-- SMC Configuration ------------------------------------------------------*/
/*--------------------------- CS2 -------------------------------------------*/
  PMC->PMC_PCER = (1 << ID_SMC);               /* enable clock for SMC   */

  SMC->SMC_CS_NUMBER[2].SMC_SETUP = 0
                                 | (( 4 <<  0) & SMC_SETUP2_NWE_SETUP)
                                 | (( 2 <<  8) & SMC_SETUP2_NCS_WR_SETUP)
                                 | (( 4 << 16) & SMC_SETUP2_NRD_SETUP)
                                 | (( 2 << 24) & SMC_SETUP2_NCS_RD_SETUP)
                                 ;

  SMC->SMC_CS_NUMBER[2].SMC_PULSE = 0
                                 | ((  5 <<  0) & SMC_PULSE2_NWE_PULSE)
                                 | (( 18 <<  8) & SMC_PULSE2_NCS_WR_PULSE)
                                 | ((  5 << 16) & SMC_PULSE2_NRD_PULSE)
                                 | (( 18 << 24) & SMC_PULSE2_NCS_RD_PULSE)
                                 ;

  SMC->SMC_CS_NUMBER[2].SMC_CYCLE = 0
                                 | ((22 <<  0) & SMC_CYCLE2_NWE_CYCLE)
                                 | ((22 << 16) & SMC_CYCLE2_NRD_CYCLE)
                                 ;

  mode = SMC->SMC_CS_NUMBER[2].SMC_MODE;
  SMC->SMC_CS_NUMBER[2].SMC_MODE = (mode & ~(SMC_MODE2_DBW | SMC_MODE2_READ_MODE | SMC_MODE2_WRITE_MODE))
                                | (SMC_MODE2_READ_MODE)
                                | (SMC_MODE2_WRITE_MODE)
                                | (SMC_MODE2_DBW)
                                ;

  delay(5);                        /* Delay 50 ms    */
  DriverCode = rd_reg(0x00);

  /* Start internal OSC */
  wr_reg(0x19, 0x49);              /* OSCADJ=10 0000, OSD_EN=1 // 60Hz */
  wr_reg(0x93, 0x0C);              /* RADJ=1100    */

    /* Power on flow */
  wr_reg(0x44, 0x4D);              /* VCM=100 1101 */
  wr_reg(0x45, 0x11);              /* VDV=1 0001   */
  wr_reg(0x20, 0x40);              /* BT=0100      */
  wr_reg(0x1D, 0x07);              /* VC1=111      */
  wr_reg(0x1E, 0x00);              /* VC3=000      */
  wr_reg(0x1F, 0x04);              /* VRH=0100     */

  wr_reg(0x1C, 0x04);              /* AP=100       */
  wr_reg(0x1B, 0x10);              /* GASENB=0, PON=1, DK=0, XDK=0, DDVDH_TRI=0, STB=0  */
  delay(5);

  wr_reg(0x43, 0x80);              /* Set VCOMG=1  */
  delay(5);

  /* Gamma for CMO 2.8  */
  wr_reg(0x46, 0x95);
  wr_reg(0x47, 0x51);
  wr_reg(0x48, 0x00);
  wr_reg(0x49, 0x36);
  wr_reg(0x4A, 0x11);
  wr_reg(0x4B, 0x66);
  wr_reg(0x4C, 0x14);
  wr_reg(0x4D, 0x77);
  wr_reg(0x4E, 0x13);
  wr_reg(0x4F, 0x4C);
  wr_reg(0x50, 0x46);
  wr_reg(0x51, 0x46);

  /* 240x320 window setting */
  wr_reg(0x02, 0x00);
  wr_reg(0x03, 0x00);
  wr_reg(0x04, (((WIDTH - 1) >> 8) & 0xFF));
  wr_reg(0x05, (((WIDTH - 1)     ) & 0xFF));
  wr_reg(0x06, 0x00);
  wr_reg(0x07, 0x00);
  wr_reg(0x08, (((HEIGHT - 1) >> 8) & 0xFF));
  wr_reg(0x09, (((HEIGHT - 1)     ) & 0xFF));

    /* Display Setting  */
  wr_reg(0x01, 0x06);              /* IDMON=0, INVON=1, NORON=1, PTLON=0 */
  wr_reg(0x16, 0xE8);              /* MY=1, MX=1, MV=1, BGR=1 */
  wr_reg(0x23, 0x95);              /* N_DC=1001 0101 */
  wr_reg(0x24, 0x95);              /* P_DC=1001 0101 */
  wr_reg(0x25, 0xFF);              /* I_DC=1111 1111 */
  wr_reg(0x27, 0x06);              /* N_BP=0000 0110 */
  wr_reg(0x28, 0x06);              /* N_FP=0000 0110 */
  wr_reg(0x29, 0x06);              /* P_BP=0000 0110 */
  wr_reg(0x2A, 0x06);              /* P_FP=0000 0110 */
  wr_reg(0x2C, 0x06);              /* I_BP=0000 0110 */
  wr_reg(0x2D, 0x06);              /* I_FP=0000 0110 */
  wr_reg(0x3A, 0x01);              /* N_RTN=0000, N_NW=001 */
  wr_reg(0x3B, 0x01);              /* P_RTN=0000, P_NW=001 */
  wr_reg(0x3C, 0xF0);              /* I_RTN=1111, I_NW=000 */
  wr_reg(0x3D, 0x00);              /* DIV=00   */
  wr_reg(0x3E, 0x38);              /* SON=38h  */
  wr_reg(0x40, 0x0F);              /* GDON=0Fh */
  wr_reg(0x41, 0xF0);              /* GDOF=F0h */

  /* Display ON Setting  */
  wr_reg(0x90, 0x7F);              /* SAP=0111 1111      */
  wr_reg(0x26, 0x04);              /* GON=0, DTE=0, D=01 */
  delay(10);
  wr_reg(0x26, 0x24);              /* GON=1, DTE=0, D=01 */
  wr_reg(0x26, 0x2C);              /* GON=1, DTE=0, D=11 */
  delay(10);
  wr_reg(0x26, 0x3C);              /* GON=1, DTE=1, D=11 */



  /* Turn backlight on with a certain backlight level */
  for (i = 0; i < 25 /* level */; i++) {
    PIOC->PIO_SODR = (LCD_BACKLIGHT);
    PIOC->PIO_SODR = (LCD_BACKLIGHT);
    PIOC->PIO_SODR = (LCD_BACKLIGHT);

    PIOC->PIO_CODR = (LCD_BACKLIGHT);
    PIOC->PIO_CODR = (LCD_BACKLIGHT);
    PIOC->PIO_CODR = (LCD_BACKLIGHT);
  }
  PIOC->PIO_SODR = (LCD_BACKLIGHT);
}


void GLCD_putPixel(unsigned int x, unsigned int y) {

  wr_reg(0x02, ((y >> 8) & 0xFF));
  wr_reg(0x03, ((y     ) & 0xFF));
  wr_reg(0x06, ((x >> 8) & 0xFF));
  wr_reg(0x07, ((x     ) & 0xFF));

  wr_cmd(0x22);
  wr_dat(TextColor);
}


/*******************************************************************************
* Set foreground color                                                         *
*   Parameter:    color:  color for clearing display                           *
*   Return:                                                                    *
*******************************************************************************/
void GLCD_setTextColor(unsigned short color) {
  TextColor = color;
}


/*******************************************************************************
* Set background color                                                         *
*   Parameter:    color:  color for clearing display                           *
*   Return:                                                                    *
*******************************************************************************/
void GLCD_setBackColor(unsigned short color) {
  BackColor = color;
}


/*******************************************************************************
* Clear display                                                                *
*   Parameter:    color:  color for clearing display                           *
*   Return:                                                                    *
*******************************************************************************/
void GLCD_clear (unsigned short color) {
  unsigned int   i;

  wr_reg(0x02, 0);
  wr_reg(0x03, 0);
  wr_reg(0x06, 0);
  wr_reg(0x07, 0);

  wr_cmd(0x22);
  for(i = 0; i < (WIDTH*HEIGHT); i++)
    wr_dat(color);
}


/*******************************************************************************
* Draw character on given position (line, coloum                               *
*   Parameter:     x :        horizontal position                              *
*                  y :        vertical position                                *
*                  c*:        pointer to color value                           *
*   Return:                                                                    *
*******************************************************************************/
void GLCD_drawChar(unsigned int x, unsigned int y, unsigned short *c) {
  unsigned int index = 0;
  int  i = 0;
  unsigned int Xaddress = 0;
   
  Xaddress = x;
  
  wr_reg(0x02, ((y        >> 8) & 0xFF));
  wr_reg(0x03, ((y            ) & 0xFF));
  wr_reg(0x06, ((Xaddress >> 8) & 0xFF));
  wr_reg(0x07, ((Xaddress     ) & 0xFF));

  for(index = 0; index < 24; index++)
  {
    wr_cmd(0x22);
    for(i = 15; i >= 0; i--)
//    for(i = 0; i < 16; i++)
    {
      if((c[index] & (1 << i)) == 0x00) {
        wr_dat(BackColor);
      } else {
        wr_dat(TextColor);
      }
    }
    Xaddress++;

    wr_reg(0x02, ((y        >> 8) & 0xFF));
    wr_reg(0x03, ((y            ) & 0xFF));
    wr_reg(0x06, ((Xaddress >> 8) & 0xFF));
    wr_reg(0x07, ((Xaddress     ) & 0xFF));
  }
}


/*******************************************************************************
* Disply character on given line                                               *
*   Parameter:     c :        ascii character                                  *
*                  ln:        line number                                      *
*   Return:                                                                    *
*******************************************************************************/
void GLCD_displayChar(unsigned int ln, unsigned int col, unsigned char  c) {
  c -= 32;
  GLCD_drawChar(ln, col, &ASCII_Table[c * 24]);
}


/*******************************************************************************
* Disply string on given line                                                  *
*   Parameter:     s*:        pointer to string                                *
*                  ln:        line number                                      *
*   Return:                                                                    *
*******************************************************************************/
void GLCD_displayStringLn(unsigned int ln, unsigned char *s) {
  unsigned int i = 0;
  unsigned int refcolumn = (WIDTH/*-1*/)-16;

  while ((*s != 0) & (i < 20))                   /* write the string character by character on lCD */
  {
    GLCD_displayChar(ln, refcolumn, *s);         /* Display one character on LCD */
    refcolumn -= 16;                             /* Decrement the column position by 16 */
    s++;                                         /* Point on the next character */
    i++;                                         /* Increment the character counter */
  }
}


/*******************************************************************************
* Clear given line                                                             *
*   Parameter:     ln:        line number                                      *
*   Return:                                                                    *
*******************************************************************************/
void GLCD_clearLn(unsigned int ln) {
  GLCD_displayStringLn(ln, "                    ");
}


/*******************************************************************************
* Display graphical bitmap image at position x horizontally and y vertically   *
* (This function is optimized for 16 bits per pixel format, it has to be       *
*  adapted for any other bits per pixel format)                                *
*   Parameter:      x:        horizontal position                              *
*                   y:        vertical position                                *
*                   w:        width of bitmap                                  *
*                   h:        height of bitmap                                 *
*                   bitmap:   address at which the bitmap data resides         *
*   Return:                                                                    *
*******************************************************************************/
void GLCD_bitmap (unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned char *bitmap) {
  unsigned int   i;
  unsigned int   len = w*h;
  unsigned short *bitmap_ptr = (unsigned short *)bitmap;

  /* set X/Y range to bitmap values */
  /* X - range */
  wr_reg(0x02, (( x      >> 8) & 0xFF));
  wr_reg(0x03, (( x          ) & 0xFF));
  wr_reg(0x04, (((x+w-1) >> 8) & 0xFF));
  wr_reg(0x05, (((x+w-1)     ) & 0xFF));

  /* Y - range */
  wr_reg(0x06, (( y      >> 8) & 0xFF));
  wr_reg(0x07, (( y          ) & 0xFF));
  wr_reg(0x08, (((y+h-1) >> 8) & 0xFF));
  wr_reg(0x09, (((y+h-1)     ) & 0xFF));

  wr_cmd(0x22);
  for (i = 0; i < len; i++) {
    wr_dat(*bitmap_ptr++);
  }

  /* restore X/Y range to initial value */
  wr_reg(0x02, 0x00);
  wr_reg(0x03, 0x00);
  wr_reg(0x04, (((WIDTH - 1) >> 8) & 0xFF));
  wr_reg(0x05, (((WIDTH - 1)     ) & 0xFF));

  wr_reg(0x06, 0x00);
  wr_reg(0x07, 0x00);
  wr_reg(0x08, (((HEIGHT - 1) >> 8) & 0xFF));
  wr_reg(0x09, (((HEIGHT - 1)     ) & 0xFF));
}

/******************************************************************************/
