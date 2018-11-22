/******************************************************************************
 * @file:    framebuf.c
 * @purpose: Driver for framebuffer using glcd and external SRAM
 * @version: V1.0
 * @date:    14. Aug 2009
 *----------------------------------------------------------------------------
 *
 * Copyright (C) 2009 ARM Limited. All rights reserved.
 *
 * ARM Limited (ARM) is supplying this software for use with Cortex-Mx 
 * processor based microcontrollers.  This file can be freely distributed 
 * within development tools that are supporting such ARM based processors. 
 *
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 ******************************************************************************/
 

#include <stdio.h>
#include <string.h>
#include <RTL.h>                    /* RTX kernel functions & defines      */
#include "main.h"
#include "glcd.h"
#include "framebuf.h"
#include "keyboard.h"

// prototypes for internal use
void WriteFB                (void);



volatile unsigned short *frameBuf  = (volatile unsigned short *) EXT_SRAM_ADDR;
volatile unsigned short *backBuf   = (volatile unsigned short *) EXT_SRAM_ADDR + FB_SIZE;
volatile unsigned short *imageArea = (volatile unsigned short *) EXT_SRAM_ADDR + FB_SIZE + FB_SIZE;

OS_ID        fb_timer;
unsigned int frameBuf_timerRunning  = 0;
unsigned int soft_nVSYNC            = 0;       // prevent recursion!
signed   int fb_ScrollPos           = 0;
signed   int fb_TextStart           = 0;
unsigned int fb_LinesInBuf          = 0;




int FrameBuf_Init()
{
  memset((void *) frameBuf, 0, (FB_SIZE));
  memset((void *) backBuf,  0, (FB_SIZE));
  
  FrameBuf_Update();

  return(0);
}

void FrameBuf_ClearScreen(void)
{
  memset((void *) frameBuf, 0, FB_SIZE);
  FrameBuf_Update();
}


void FrameBuf_PutStr(unsigned int x, unsigned int y, char *pStr, unsigned int color)
{
  while(*pStr)
  {
    FrameBuf_PutChar(x, y, *pStr++, color);
    x += FONT_WIDTH;
  }
}




void FrameBuf_PutChar   (unsigned int x, unsigned int y, char c, unsigned int color)
{
  unsigned short data, pixelColor;
  unsigned int i, j;
  unsigned int fb_offs=0;
  

  for(i=0; i<FONT_HEIGHT; i++)
  {
    data = lcd_font[c-0x20][i];
    
    for(j=0; j<FONT_WIDTH; j++) 
    {
      pixelColor = (data & (1<<(FONT_WIDTH + FONT_FIRST_BIT - 1 - j))) ? color : BackColor;
      fb_offs    = ( ( (y+i) * 320) + ((x+j) ) );
      
      if(fb_offs < FB_SIZE)
      {
        frameBuf[(fb_ScrollPos + fb_offs) & (FB_SIZE-1)] = pixelColor;
      }
      else
      {
        __nop();
      }      
    }
  }
  FrameBuf_Update();
}


void WriteFB(void)
{
  unsigned int i=0;
  unsigned short val;
  
  if(!soft_nVSYNC)
  {
    soft_nVSYNC = 1;
    
    GLCD_SetCursor(320, 240);
 
    for(i=0; i<DISP_SIZE; i++)
    {
      val = frameBuf[(i + fb_ScrollPos - fb_TextStart) & (FB_SIZE-1)];

      if(val) GLCD_WritePixel(val);          // Transparency color = BLACK
      else GLCD_WritePixel(backBuf[i]);
    }    
    soft_nVSYNC = 0;
  }
}


void FrameBuf_Update(void)
{
  if(!frameBuf_timerRunning)
  {
    frameBuf_timerRunning=1;
    fb_timer = os_tmr_create(20, 2);
  }
}

void FrameBuf_ForceUpdate(void)
{
  WriteFB();
}


__task void FrameBuf_WriteDisp(void)
{
  os_evt_wait_or(FB_START, 0xffff);
  
  while(1)
  {    
    os_evt_wait_or(FB_UPD, 0xffff);    
    WriteFB();    
    frameBuf_timerRunning=0;
  }
}


void FrameBuf_Scroll(signed int nLines)
{
  int i;
  
  for(i=DISP_SIZE + fb_ScrollPos; i<DISP_SIZE + (nLines * 320) + fb_ScrollPos; i++)
  {
    frameBuf[i & (FB_SIZE-1)] = 0;
  }
  
  fb_ScrollPos += (nLines * 320);
  fb_ScrollPos &= (FB_SIZE-1);
  
  if(fb_LinesInBuf < fb_ScrollPos) fb_LinesInBuf = fb_ScrollPos;
}





void FrameBuf_LoadBackG(const unsigned short *backSrc, unsigned int pictWidth, int x_offs, int y_offs)
{  
  int i, j, k;
  
  j = y_offs * pictWidth;
  j+= x_offs;
  i=0;
  
  while(soft_nVSYNC) os_dly_wait(1);
  for(k=0; k<DISP_SIZE; k++)
  {
    backBuf[i++] = backSrc[j++];
    if(!(i % 320)) j+= (pictWidth-320);
  }
}



void FrameBuf_MoveTextArea(signed int nLines)
{
  signed int tmp;
  
  tmp = fb_TextStart + (nLines * FONT_HEIGHT * 320);
  
  if(tmp < FB_SIZE - DISP_SIZE)
  {
    if(tmp < fb_LinesInBuf+1)
    {
      fb_TextStart = tmp;
    }
  }
}






