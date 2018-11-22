/******************************************************************************/
/* GLCD.c: Functions for Graphic LCD (HITACHI TX09D71VM1CCA (240x320 pixels)) */
/*         driving with Atmel AT91SAM9261 device                              */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2009 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <AT91SAM9261.H>                /* AT91SAM9261 definitions            */

#ifndef _GLCD_H
#define _GLCD_H


/*********************** Hardware specific configuration **********************/

/*---------------------- Graphic LCD size definitions ------------------------*/

#define LCD_W       240                 /* Screen Width (in pixels)           */
#define LCD_H       320                 /* Screen Hight (in pixels)           */
#define LCD_BPP     18                  /* Bits per pixel (each color 6 bits) */
#define LCD_BYPP    ((LCD_BPP+7)/8)     /* Bytes per pixel                    */

#define LCD_BUF0    0x21000000          /* Frame buffer 0 address             */


/******************************************************************************/


/************************ Exported functions **********************************/

/*******************************************************************************
* Initialize the Graphic LCD controller                                        *
*   Parameter:                                                                 *
*   Return:                                                                    *
*******************************************************************************/

void GLCD_init (void) { 

  /* Configure the PIO Lines for interfacing LCD                              */
  AT91C_BASE_PIOB->PIO_IDR       = 0x001F9F9E;
  AT91C_BASE_PIOB->PIO_PPUDR     = 0x001F9F9E;
  AT91C_BASE_PIOB->PIO_ASR       = 0x001F9F9E;
  AT91C_BASE_PIOB->PIO_PDR       = 0x001F9F9E;
  AT91C_BASE_PIOB->PIO_IDR       = 0x1FC00000;
  AT91C_BASE_PIOB->PIO_PPUDR     = 0x1FC00000;
  AT91C_BASE_PIOB->PIO_BSR       = 0x1FC00000;
  AT91C_BASE_PIOB->PIO_PDR       = 0x1FC00000;

  /* Enable pin PA12 and pull it high to enable LCD                           */
  AT91C_BASE_PIOA->PIO_PER       = AT91C_PIO_PA12;
  AT91C_BASE_PIOA->PIO_OER       = AT91C_PIO_PA12;
  AT91C_BASE_PIOA->PIO_CODR      = AT91C_PIO_PA12;

  AT91C_BASE_PMC->PMC_SCER       = AT91C_PMC_HCK1;          /* Enable HCLOCK  */

  AT91C_BASE_PMC->PMC_PCER       = 1<<AT91C_ID_LCDC;        /* Enable LCD clk */

  AT91C_BASE_LCDC->LCDC_DMACON   = 0;                       /* Disable DMA    */
  AT91C_BASE_LCDC->LCDC_PWRCON   = 0;                       /* LCD off        */

  AT91C_BASE_LCDC->LCDC_LCDCON1  = (0x09 << 12) ;           /* CLKVAL         */

  AT91C_BASE_LCDC->LCDC_LCDCON2  = (2UL  << 30) | /* MEMOR:     Little Endian */ 
                                   (0x01 << 15) | /* CLKMOD:    Always active */ 
                                   (0x00 << 12) | /* INVDVAL:   Normal        */ 
                                   (0x00 << 11) | /* INVCLK:    Normal        */ 
                                   (0x00 << 10) | /* INVLINE:   Normal        */ 
                                   (0x00 <<  9) | /* INVFRAME:  Normal        */ 
                                   (0x00 <<  8) | /* INVVD:     Normal        */ 
                                   (0x05 <<  5) | /* PIXELSIZE: 24 BPP, packed*/ 
                                   (0x00 <<  3) | /* IFWIDTH:   4-bit         */ 
                                   (0x00 <<  2) | /* SCANMOD:   Single Scan   */ 
                                   (0x02 <<  0) ; /* DISTYPE:   TFT           */ 

  AT91C_BASE_LCDC->LCDC_LCDFRCFG = ((LCD_W-1) << 21) |      /* LINESIZE       */
                                   ((LCD_H-1) <<  0) ;      /* LINEVAL        */

  AT91C_BASE_LCDC->LCDC_TIM1     = (0x00 << 24) |           /* VHDLY          */ 
                                   (0x00 << 16) |           /* VPW            */ 
                                   (0x04 <<  8) |           /* VBP            */ 
                                   (0x02 <<  0) ;           /* VFP            */ 

  AT91C_BASE_LCDC->LCDC_TIM2     = (0x50 << 21) |           /* HFP            */ 
                                   (0x04 <<  8) |           /* HPW            */ 
                                   (0x10 <<  0) ;           /* HBP            */ 

  AT91C_BASE_LCDC->LCDC_CTRSTVAL = (0x80 <<  0) ;           /* CVAL           */ 

  AT91C_BASE_LCDC->LCDC_CTRSTCON = (0x01 <<  3) |           /* ENA            */ 
                                   (0x01 <<  2) |           /* POL            */ 
                                   (0x00 <<  0) ;           /* PS             */ 

  AT91C_BASE_LCDC->LCDC_FRMCFG   = (0x03 << 24) |           /* BRSTLN         */
                                   (LCD_W*LCD_H*LCD_BYPP/4);/* FRMSIZE        */

  AT91C_BASE_LCDC->LCDC_FIFO     = 2048 - (2 * 4 + 3);      /* FIFOTH         */

  AT91C_BASE_LCDC->LCDC_BA1      = (unsigned int) LCD_BUF0; /* DMA Base Addr  */

  AT91C_BASE_LCDC->LCDC_DMACON   = AT91C_LCDC_DMAEN;        /* Enable DMA     */

  AT91C_BASE_LCDC->LCDC_PWRCON   = (0x0C <<  1) |           /* GUARDTIME      */
                                   (0x01 <<  0) ;           /* LCD_PWR: On    */
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

  unsigned int  i, j;
  unsigned char *disp_ptr;
  unsigned char pixel_b, pixel_g;
  unsigned int  row_add;

  disp_ptr = (unsigned char *) LCD_BUF0 + (x + y * LCD_W) * LCD_BYPP;
  row_add  = (LCD_W - w) * LCD_BYPP;

  for (j = 0; j < h; j++) {
    for (i = 0; i < w; i++) {
      bitmap++;
      pixel_b = *bitmap++;
      pixel_g = *bitmap++;
      *disp_ptr++ = *bitmap++;          /* Pixel's Red   component            */
      *disp_ptr++ = pixel_g;            /* Pixel's Green component            */
      *disp_ptr++ = pixel_b;            /* Pixel's Blue  component            */
    }
    disp_ptr += row_add;
  }
}


#endif /* _GLCD_H */

/******************************************************************************/
