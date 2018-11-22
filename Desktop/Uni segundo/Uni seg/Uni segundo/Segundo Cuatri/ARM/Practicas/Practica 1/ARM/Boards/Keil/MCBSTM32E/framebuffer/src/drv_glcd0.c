/******************************************************************************
 * @file:    drv_glcd0.c
 * @purpose: IO driver layer for GLCD
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

#include "term_ctrl.h"
#include "drv_glcd0.h"
#include "glcd.h"
#include "framebuf.h"




// ----------------------   IO   -------------------------------
int  Open_GLCD0   (void) 
{
  // GLCD Init
  GLCD_init();
  GLCD_setBackColor(Black);
  GLCD_setTextColor(White);
  
  // FrameBuffer init
  ExtSRAM_Init();
  FrameBuf_Init();
  FrameBuf_ClearScreen();
 
  return(0);
}

int  Close_GLCD0  (void) 
{
  return(0);
}

unsigned int  Input_GLCD0  (unsigned char *buf, unsigned int len) { return(len); }

unsigned int  Output_GLCD0 (const unsigned char *buf, unsigned int len)
{
  glcd0_putchar_retarget(buf, len);
  return(0);
}



// ----------------------   Driver   -------------------------------
__inline int glcd0_putchar_retarget (const unsigned char *buf, unsigned int len)
{   
  static signed int state=0, x=0, y=0, num=0, x_pos=-FONT_WIDTH, y_pos=0, color=LCD_WHITE;
  unsigned char c;
  
  while(len--)
  {
    c = *buf++;
    
    if((state==0) && (c==27)) state=1; 
    else if((state==1) && (c=='[')) 
    {
      num = 0;
      state=2;
    }
    
    else if(state==2)
    {
      if(c=='J')  // Clear Screen
      {
        state=0;
        FrameBuf_ClearScreen();
      }
      else if(c=='C')
      {
        x_pos+=FONT_WIDTH;
        state=0;
      }
      else if(c=='D')
      {
        x_pos-=FONT_WIDTH;
        state=0;
      }
      else if(c=='K')
      {
        FrameBuf_PutStr(x_pos + FONT_WIDTH, y_pos, "                                        ", color);
        state=0;
      }
      else
      {
        if((c>=48) && (c<=57))
        {     
          num*=10;
          num+=(c-48);
        }
        else if(c==';') 
        {
          y=num;
          num=0;
        }
        else if((c=='f') || (c=='H')) // Sets Cursor
        {
          x = num;
          num=0;    
          x_pos = (x*FONT_WIDTH)-FONT_WIDTH;
          y_pos = y * FONT_HEIGHT;
          state=0;
        }
        else if(c=='m')
        {
          switch(num%10)
          {
            case BLACK:   color = LCD_BLACK; break;
            case RED:     color = LCD_RED; break;
            case GREEN:   color = LCD_GREEN; break;
            case YELLOW:  color = LCD_YELLOW; break;
            case BLUE:    color = LCD_BLUE; break;
            case MAGENTA: color = LCD_MAGENTA; break;
            case CYAN:    color = LCD_CYAN; break;
            case WHITE:   color = LCD_WHITE; break;
          }
          //lcd_BackgroundColor = charBackFrontColor;     (if foreground / back color!)
          num=0;
          state=0;
        }
        else state=0;
      }
    }      
    else 
    {
      if(c=='\n')   // handles newline
      {
        x_pos=-FONT_WIDTH;
        y_pos+=FONT_HEIGHT;
        if(y_pos > (LCD_WIDTH - FONT_HEIGHT))
        {
          FrameBuf_Scroll(FONT_HEIGHT);
          y_pos-=FONT_HEIGHT;
        }
        
        if(x_pos > (LCD_HEIGHT - FONT_WIDTH)) x_pos=0;
        
        
        FrameBuf_PutStr(x_pos, y_pos, "                                        ", color);     
      }
      else if(c=='\r') x_pos=-FONT_WIDTH;
      else if(c=='\t')
      {
        if(x_pos < LCD_HEIGHT) x_pos+=(FONT_WIDTH*3);
      }
      
      else 
      {      
        x_pos+=FONT_WIDTH;
        
        if(x_pos >= LCD_HEIGHT)
        {
          x_pos=0;
          y_pos+=FONT_HEIGHT;
          if(y_pos > (LCD_WIDTH - FONT_HEIGHT)) y_pos=0;      
          FrameBuf_PutStr(x_pos, y_pos, "                                        ", color);      
        }  
        FrameBuf_PutChar(x_pos, y_pos, c, color);      
      }
      state=0;
    }
  }
  return(c);
}


