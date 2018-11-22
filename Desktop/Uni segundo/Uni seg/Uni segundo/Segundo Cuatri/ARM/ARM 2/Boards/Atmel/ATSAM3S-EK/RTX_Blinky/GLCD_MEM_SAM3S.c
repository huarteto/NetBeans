/******************************************************************************/
/* GLCD_MEM_SAM3S.c: SAM3S low level Graphic LCD (320x240 pixels) driven with */
/*                   Memory Mapped functions                                  */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2010 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/


#include <SAM3S.H>                      /* ATSAM3S definitions                */
#include "GLCD.h"
#include "Font_24x16.h"

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

/* Note: LCD /CS is CCS1 */
#define LCD_BASE        (0x61000000UL)
#define LCD_REG8   (*((volatile unsigned char  *)(LCD_BASE  ))) 
#define LCD_DAT8   (*((volatile unsigned char  *)(LCD_BASE+2)))
 
/*---------------------------- Global variables ------------------------------*/

/******************************************************************************/
static volatile unsigned short TextColor = Black, BackColor = White;


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

  LCD_REG8 = 0;
  LCD_REG8 = c;
}


/*******************************************************************************
* Write data to LCD controller                                                 *
*   Parameter:    c:      data to be written                                   *
*   Return:                                                                    *
*******************************************************************************/

static __inline void wr_dat (unsigned short c) {

  LCD_DAT8 = (c >> 8) & 0xFF;
  LCD_DAT8 = c & 0xFF;
}


/*******************************************************************************
* Start of data writing to LCD controller                                      *
*   Parameter:                                                                 *
*   Return:                                                                    *
*******************************************************************************/

static __inline void wr_dat_start (void) {

  /* not needed */
}


/*******************************************************************************
* Stop of data writing to LCD controller                                       *
*   Parameter:                                                                 *
*   Return:                                                                    *
*******************************************************************************/

static __inline void wr_dat_stop (void) {

  /* not needed */
}


/*******************************************************************************
* Data writing to LCD controller                                               *
*   Parameter:    c:      data to be written                                   *
*   Return:                                                                    *
*******************************************************************************/

static __inline void wr_dat_only (unsigned short c) {

  LCD_DAT8 = (c >> 8) & 0xFF;
  LCD_DAT8 = c & 0xFF;
}


/*******************************************************************************
* Read data from LCD controller                                                *
*   Parameter:                                                                 *
*   Return:               read data                                            *
*******************************************************************************/

static __inline unsigned short rd_dat (void) {
  unsigned short val = 0;

  val   = LCD_DAT8;                       /* Read D8..D15                 */
  val <<= 8;
  val  |= LCD_DAT8;                       /* Read D0..D7                  */
  return (val);
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
#define LCD_D0_D7       (0xFF)
#define LCD_RS          (PIO_PC19)
#define LCD_RD          (PIO_PC11)
#define LCD_WR          (PIO_PC8)
#define LCD_NCS1        (PIO_PC15)
#define LCD_BACKLIGHT   (PIO_PC13)

void GLCD_Init (void) { 
  static unsigned short driverCode;
  unsigned int mode, i;

/* Configure the LCD Control pins -------------------------------------------*/
  PMC->PMC_WPMR = 0x504D4300;                  /* Disable write protect */
  PMC->PMC_PCER1 = (1 << ID_PIOC);             /* enable clock for PIOC */

/* Configure the Data Bus pins ----------------------------------------------*/
  PIOC->PIO_PDR    =  (LCD_D0_D7);            /* enable Peripheral function */  
  PIOC->PIO_IDR    =  (LCD_D0_D7);            /* disable interrupt */ 
  PIOC->PIO_ABCDSR[0]  &= ~(LCD_D0_D7);       /* select Peripheral A */ 
  PIOC->PIO_ABCDSR[1]  &= ~(LCD_D0_D7);       /* select Peripheral A */ 

/* Configure the Control pins -----------------------------------------------*/
  PIOC->PIO_PDR    =  (LCD_RS);               /* enable Peripheral function */ 
  PIOC->PIO_IDR    =  (LCD_RS);               /* disable interrupt */ 
  PIOC->PIO_ABCDSR[0] &= ~(LCD_RS);           /* select Peripheral A */ 
  PIOC->PIO_ABCDSR[1] &= ~(LCD_RS);           /* select Peripheral A */ 

  PIOC->PIO_PDR    =  (LCD_RD | LCD_WR);      /* enable Peripheral function */ 
  PIOC->PIO_IDR    =  (LCD_RD | LCD_WR);      /* disable interrupt */ 
  PIOC->PIO_ABCDSR[0] &= ~(LCD_RD | LCD_WR);  /* select Peripheral A */ 
  PIOC->PIO_ABCDSR[1] &= ~(LCD_RD | LCD_WR);  /* select Peripheral A */ 

  PIOC->PIO_PDR    =  (LCD_NCS1);             /* enable Peripheral function */  
  PIOC->PIO_IDR    =  (LCD_NCS1);             /* disable interrupt */ 
  PIOC->PIO_ABCDSR[0] &= ~(LCD_NCS1);         /* select Peripheral A */ 
  PIOC->PIO_ABCDSR[1] &= ~(LCD_NCS1);         /* select Peripheral A */ 

/* Configure the Backlight pins ---------------------------------------------*/
  PIOC->PIO_IDR  =                             /* disable interrupt */
  PIOC->PIO_OER  =                             /* enable output */
  PIOC->PIO_PER  = (LCD_BACKLIGHT);            /* disable Peripheral function */ 
  PIOC->PIO_CODR = (LCD_BACKLIGHT);            /* Backlight off */

/*-- SMC Configuration ------------------------------------------------------*/
/*--------------------------- CS2 -------------------------------------------*/
  PMC->PMC_PCER0 = (1 << ID_SMC);              /* enable clock for SMC   */

  SMC->SMC_CS_NUMBER[1].SMC_SETUP = 0
                                 | ((  2 <<  0) & SMC_SETUP1_NWE_SETUP)
                                 | ((  2 <<  8) & SMC_SETUP1_NCS_WR_SETUP)
                                 | ((  2 << 16) & SMC_SETUP1_NRD_SETUP)
                                 | ((  2 << 24) & SMC_SETUP1_NCS_RD_SETUP)
                                 ;

  SMC->SMC_CS_NUMBER[1].SMC_PULSE = 0
                                 | (( 4 <<  0) & SMC_PULSE1_NWE_PULSE)
                                 | (( 4 <<  8) & SMC_PULSE1_NCS_WR_PULSE)
                                 | ((10 << 16) & SMC_PULSE1_NRD_PULSE)
                                 | ((10 << 24) & SMC_PULSE1_NCS_RD_PULSE)
                                 ;

  SMC->SMC_CS_NUMBER[1].SMC_CYCLE = 0
                                 | ((10 <<  0) & SMC_CYCLE1_NWE_CYCLE)
                                 | ((22 << 16) & SMC_CYCLE1_NRD_CYCLE)
                                 ;

  mode = SMC->SMC_CS_NUMBER[1].SMC_MODE;
  SMC->SMC_CS_NUMBER[1].SMC_MODE = (mode & ~(SMC_MODE1_DBW | SMC_MODE1_READ_MODE | SMC_MODE1_WRITE_MODE))
                                | (SMC_MODE1_READ_MODE)
                                | (SMC_MODE1_WRITE_MODE)
                                | (0)//(SMC_MODE1_DBW)
                                ;

  delay(5);                             /* Delay 50 ms                        */
  driverCode = rd_reg(0x00);
  if (driverCode);                      /* avoid warning */

  /* Start Initial Sequence --------------------------------------------------*/
  wr_reg(0x10, 0x0000);                 /* DSTB = LP = STB = 0                */
  wr_reg(0x00, 0x0001);                 /* Start internal OSC                 */
  wr_reg(0x01, 0x0100);                 /* Set SS and SM bit                  */
  wr_reg(0x02, 0x0700);                 /* Set 1 line inversion               */
  wr_reg(0x03, 0x1038);                 /* Set GRAM write direction and BGR=1 */   // ??
  wr_reg(0x04, 0x0000);                 /* Resize register                    */
  wr_reg(0x08, 0x0202);                 /* 2 lines each, back and front porch */
  wr_reg(0x09, 0x0000);                 /* Set non-disp area refresh cyc ISC  */
  wr_reg(0x0A, 0x0000);                 /* FMARK function                     */
  wr_reg(0x0C, 0x0000);                 /* RGB interface setting              */
  wr_reg(0x0D, 0x0000);                 /* Frame marker Position              */
  wr_reg(0x0F, 0x0000);                 /* RGB interface polarity             */

  /* Power On sequence -------------------------------------------------------*/
  wr_reg(0x10, 0x0000);                 /* Reset Power Control 1              */
  wr_reg(0x11, 0x0000);                 /* Reset Power Control 2              */
  wr_reg(0x12, 0x0000);                 /* Reset Power Control 3              */
  wr_reg(0x13, 0x0000);                 /* Reset Power Control 4              */
  delay(20);                            /* Discharge cap power voltage (200ms)*/
  wr_reg(0x10, 0x1290);                 /* SAP, BT[3:0], AP, DSTB, SLP, STB   */
  wr_reg(0x11, 0x0227);                 /* DC1[2:0], DC0[2:0], VC[2:0]        */
  delay(5);                             /* Delay 50 ms                        */
  wr_reg(0x12, 0x001B);                 /* Internal reference voltage= Vci    */
  delay(5);                             /* Delay 50 ms                        */
  wr_reg(0x13, 0x1100);                 /* Set VDV[4:0] for VCOM amplitude    */
  wr_reg(0x29, 0x0019);                 /* Set VCM[5:0] for VCOMH             */
  wr_reg(0x2B, 0x000D);                 /* Set Frame Rate                     */
  delay(5);                             /* Delay 50 ms                        */
  wr_reg(0x20, 0x0000);                 /* GRAM horizontal Address            */
  wr_reg(0x21, 0x0000);                 /* GRAM Vertical Address              */

  /* Adjust the Gamma Curve --------------------------------------------------*/
  wr_reg(0x30, 0x0000);
  wr_reg(0x31, 0x0204);
  wr_reg(0x32, 0x0200);
  wr_reg(0x35, 0x0007);
  wr_reg(0x36, 0x1404);
  wr_reg(0x37, 0x0705);
  wr_reg(0x38, 0x0305);
  wr_reg(0x39, 0x0707);
  wr_reg(0x3C, 0x0701);
  wr_reg(0x3D, 0x000E);
  
  /* Set GRAM area -----------------------------------------------------------*/
  wr_reg(0x50, 0x0000);                 /* Horizontal GRAM Start Address      */
  wr_reg(0x51, (HEIGHT-1));             /* Horizontal GRAM End   Address      */
  wr_reg(0x52, 0x0000);                 /* Vertical   GRAM Start Address      */
  wr_reg(0x53, (WIDTH-1));              /* Vertical   GRAM End   Address      */
  wr_reg(0x60, 0x2700);                 /* Gate Scan Line                     */
  wr_reg(0x61, 0x0001);                 /* NDL,VLE, REV                       */
  wr_reg(0x6A, 0x0000);                 /* Set scrolling line                 */

  /* Partial Display Control -------------------------------------------------*/
  wr_reg(0x80, 0x0000);
  wr_reg(0x81, 0x0000);
  wr_reg(0x82, 0x0000);
  wr_reg(0x83, 0x0000);
  wr_reg(0x84, 0x0000);
  wr_reg(0x85, 0x0000);

  /* Panel Control -----------------------------------------------------------*/
  wr_reg(0x90, 0x0010);
  wr_reg(0x92, 0x0600);
  wr_reg(0x95, 0x0110);

  /* Set GRAM write direction and BGR = 1
     I/D=01 (Horizontal : increment, Vertical : decrement)
     AM=1 (address is updated in vertical writing direction)                  */
  wr_reg(0x03, 0x1018);

  wr_reg(0x07, 0x0133);                 /* 262K color and display ON          */

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


/*******************************************************************************
* Set draw window region to whole screen                                       *
*   Parameter:                                                                 *
*   Return:                                                                    *
*******************************************************************************/

void GLCD_WindowMax (void) {

  wr_reg(0x50, 0);                      /* Horizontal GRAM Start Address      */
  wr_reg(0x51, HEIGHT-1);               /* Horizontal GRAM End   Address (-1) */
  wr_reg(0x52, 0);                      /* Vertical   GRAM Start Address      */
  wr_reg(0x53, WIDTH-1);                /* Vertical   GRAM End   Address (-1) */
}


/*******************************************************************************
* Draw a pixel in foreground color                                             *
*   Parameter:      x:        horizontal position                              *
*                   y:        vertical position                                *
*   Return:                                                                    *
*******************************************************************************/

void GLCD_PutPixel (unsigned int x, unsigned int y) {

  wr_reg(0x20, y);
  wr_reg(0x21, x);
  wr_cmd(0x22);
  wr_dat(TextColor);
}


/*******************************************************************************
* Set foreground color                                                         *
*   Parameter:      color:    foreground color                                 *
*   Return:                                                                    *
*******************************************************************************/

void GLCD_SetTextColor (unsigned short color) {

  TextColor = color;
}


/*******************************************************************************
* Set background color                                                         *
*   Parameter:      color:    background color                                 *
*   Return:                                                                    *
*******************************************************************************/

void GLCD_SetBackColor (unsigned short color) {

  BackColor = color;
}


/*******************************************************************************
* Clear display                                                                *
*   Parameter:      color:    display clearing color                           *
*   Return:                                                                    *
*******************************************************************************/

void GLCD_Clear (unsigned short color) {
  unsigned int   i;

  GLCD_WindowMax();

  wr_reg(0x20, 0);
  wr_reg(0x21, 0);
  wr_cmd(0x22);
  wr_dat_start();
  for(i = 0; i < (WIDTH*HEIGHT); i++)
    wr_dat_only(color);
  wr_dat_stop();
}


/*******************************************************************************
* Draw character on given position                                             *
*   Parameter:      x:        horizontal position                              *
*                   y:        vertical position                                *
*                   c:        pointer to character bitmap                      *
*   Return:                                                                    *
*******************************************************************************/

void GLCD_DrawChar (unsigned int x, unsigned int y, unsigned short *c) {
  int idx = 0, i, j;

  wr_reg(0x50, y);                      /* Horizontal GRAM Start Address      */
  wr_reg(0x51, y+CHAR_H-1);             /* Horizontal GRAM End   Address (-1) */
  wr_reg(0x52, x);                      /* Vertical   GRAM Start Address      */
  wr_reg(0x53, x+CHAR_W-1);             /* Vertical   GRAM End   Address (-1) */

  wr_reg(0x20, y);
  wr_reg(0x21, x+CHAR_W-1);
  wr_cmd(0x22);
  wr_dat_start();
  for (j = 0; j < CHAR_H; j++) {
    for (i = CHAR_W-1; i >= 0; i--) {
      if((c[idx] & (1 << i)) == 0x00) {
        wr_dat_only(BackColor);
      } else {
        wr_dat_only(TextColor);
      }
    }
    c++;
  }
  wr_dat_stop();
}


/*******************************************************************************
* Disply character on given line                                               *
*   Parameter:      ln:       line number                                      *
*                   col:      column number                                    *
*                   c:        ascii character                                  *
*   Return:                                                                    *
*******************************************************************************/

void GLCD_DisplayChar (unsigned int ln, unsigned int col, unsigned char c) {

  c -= 32;
  GLCD_DrawChar(col * CHAR_W, ln * CHAR_H, (unsigned short *)&Font_24x16[c * CHAR_H]);
}


/*******************************************************************************
* Disply string on given line                                                  *
*   Parameter:      ln:       line number                                      *
*                   col:      column number                                    *
*                   s:        pointer to string                                *
*   Return:                                                                    *
*******************************************************************************/

void GLCD_DisplayString (unsigned int ln, unsigned int col, unsigned char *s) {

  GLCD_WindowMax();
  while (*s) {
    GLCD_DisplayChar(ln, col++, *s++);
  }
}


/*******************************************************************************
* Clear given line                                                             *
*   Parameter:      ln:       line number                                      *
*   Return:                                                                    *
*******************************************************************************/

void GLCD_ClearLn (unsigned int ln) {

  GLCD_WindowMax();
  GLCD_DisplayString(ln, 0, "                    ");
}

/*******************************************************************************
* Draw bargraph                                                                *
*   Parameter:      x:        horizontal position                              *
*                   y:        vertical position                                *
*                   w:        maximum width of bargraph (in pixels)            *
*                   val:      value of active bargraph (in 1/1024)             *
*   Return:                                                                    *
*******************************************************************************/

void GLCD_Bargraph (unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int val) {
  int i,j;

  wr_reg(0x50, y);                      /* Horizontal GRAM Start Address      */
  wr_reg(0x51, y+CHAR_H-1);             /* Horizontal GRAM End   Address (-1) */
  wr_reg(0x52, x);                      /* Vertical   GRAM Start Address      */
  wr_reg(0x53, x+w-1);                  /* Vertical   GRAM End   Address (-1) */

  val = (val * w) >> 10;                /* Scale value for 24x12 characters   */
  wr_reg(0x20, y);
  wr_reg(0x21, x);
  wr_cmd(0x22);
  wr_dat_start();
  for (i = 0; i < h; i++) {
    for (j = w-1; j >= 0; j--) {
      if(j >= val) {
        wr_dat_only(BackColor);
      } else {
        wr_dat_only(TextColor);
      }
    }
  }
  wr_dat_stop();
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

void GLCD_Bitmap (unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned char *bitmap) {
  unsigned int    i, j;
  unsigned short *bitmap_ptr = (unsigned short *)bitmap;

  wr_reg(0x50, y);                      /* Horizontal GRAM Start Address      */
  wr_reg(0x51, y+h-1);                  /* Horizontal GRAM End   Address (-1) */
  wr_reg(0x52, x);                      /* Vertical   GRAM Start Address      */
  wr_reg(0x53, x+w-1);                  /* Vertical   GRAM End   Address (-1) */

  wr_reg(0x20, y);
  wr_reg(0x53, x+w-1);                  /* Vertical   GRAM End   Address (-1) */
  wr_cmd(0x22);
  wr_dat_start();
  for (j = 0; j < h; j++) {
    bitmap_ptr += w-1;
    for (i = 0; i < w; i++) {
      wr_dat_only(*bitmap_ptr--);
    }
    bitmap_ptr += w+1;
  }
  wr_dat_stop();
}


/*******************************************************************************
* Display graphical bmp file image at position x horizontally and y vertically *
* (This function is optimized for 16 bits per pixel format, it has to be       *
*  adapted for any other bits per pixel format)                                *
*   Parameter:      x:        horizontal position                              *
*                   y:        vertical position                                *
*                   w:        width of bitmap                                  *
*                   h:        height of bitmap                                 *
*                   bmp:      address at which the bmp data resides            *
*   Return:                                                                    *
*******************************************************************************/

void GLCD_Bmp (unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned char *bmp) {
  unsigned int    i, j;
  unsigned short *bitmap_ptr = (unsigned short *)bmp;

  wr_reg(0x50, y);                      /* Horizontal GRAM Start Address      */
  wr_reg(0x51, y+h-1);                  /* Horizontal GRAM End   Address (-1) */
  wr_reg(0x52, x);                      /* Vertical   GRAM Start Address      */
  wr_reg(0x53, x+w-1);                  /* Vertical   GRAM End   Address (-1) */

  wr_reg(0x20, y    );
  wr_reg(0x21, x+w-1);
  wr_cmd(0x22);
  wr_dat_start();
  bitmap_ptr += (h*w)-1;
  for (j = 0; j < h; j++) {
    for (i = 0; i < w; i++) {
      wr_dat_only(*bitmap_ptr--);
    }
  }
  wr_dat_stop();
}

/******************************************************************************/
