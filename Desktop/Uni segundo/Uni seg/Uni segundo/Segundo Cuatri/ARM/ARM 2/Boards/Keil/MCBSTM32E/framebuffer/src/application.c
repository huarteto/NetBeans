
#include <stdio.h>
#include <RTL.h>                    /* RTX kernel functions & defines      */
#include "application.h"
#include "term_ctrl.h"
#include "framebuf.h"
#include "keyboard.h"
#include "picture.h"
#include "glcd.h"


void PrintMessage(void);



__task void MoveBackImage(void)
{
  unsigned int val=0, oldVal=0, count=0, repeat=0;
  signed   int /* x_offs_text, */ y_offs_text;
  signed   int x_offs_back, y_offs_back;
  
  //x_offs_text = 0x00;
  y_offs_text = 0x00;
  
  x_offs_back = 0x6c;
  y_offs_back = 0x0c;
  
#if USE_BACKGROUND 
  FrameBuf_LoadBackG(mcbstm32e, PICT_WIDTH, x_offs_back, y_offs_back);      // load an area of Background Image and display on screen
  FrameBuf_Update();                                                        // FB update
#endif
  
  os_evt_wait_or(FB_START, 0xffff);
  
  while(1)
  { 
/*************  Keyboard debouncer  ****************
 * If a button is pressed and released quickly, the while-loop exits after a short
 * debouncing delay.
 * If a button is kept pressed, after a delay the repeat function is enabled,
 * and the button action is done in high speed.
 */
    oldVal = val;
    val = getKeyboardVal();
        
    if(val != oldVal) repeat = 0;
  
    if(!repeat)
    {
      while(getKeyboardVal() == val)
      {
        os_dly_wait(3);           // debouncing delay
        repeat++;
        if(repeat > 10) break;    // repeat detection counter
      }
    }
    else os_dly_wait(5);          // delay in repeat mode
/****************************************************/
   
    if(val)
    {           
      if(val & BUT_USER)
      {        
        if(val & JOY_UP)                          y_offs_text =  1;
        else if(val & JOY_DOWN)                   y_offs_text = -1;
          
        if(y_offs_text)
        {
          FrameBuf_MoveTextArea(y_offs_text);
          y_offs_text = 0;
        } 
      }
      else
      {
        if(val & JOY_UP)                            y_offs_back -= PICT_MOVE_SPEED;
        else if(val & JOY_DOWN)                     y_offs_back += PICT_MOVE_SPEED;
        if(val & JOY_RIGHT)                         x_offs_back += PICT_MOVE_SPEED;
        else if(val & JOY_LEFT)                     x_offs_back -= PICT_MOVE_SPEED;
        
        if(y_offs_back < 0)                         y_offs_back  = 0;
        else if(x_offs_back > (PICT_WIDTH -320))    x_offs_back  = (PICT_WIDTH  -320);
        if(x_offs_back < 0)                         x_offs_back  = 0;
        else if(y_offs_back > (PICT_HEIGHT-240))    y_offs_back  = (PICT_HEIGHT -240);
    
#if USE_BACKGROUND        
        FrameBuf_LoadBackG(mcbstm32e, PICT_WIDTH, x_offs_back , y_offs_back );
#endif 
      }
      
      if(val & JOY_BUT)
      {
      printf("\nHello, World: %i", count++);
      }
      
      FrameBuf_Update();                    // FB update
    }
    
  }
}



__task void TypeTextOnGLCD0(void)
{
  unsigned char  c, count=0;
  FILE *fp_itm;
  
    
  os_evt_wait_or(FB_START, 0xffff);
  
  PrintMessage();  
  
  fp_itm   = fopen("itm", "w+");
  fprintf(fp_itm, "\n\nSTDOUT is now redirected to GLCD0.\nInput Channel is still ITM.\nText: ");
  printf("\n\nType some text in ITM Window\n");
  
  while(1)
  {
    c = getchar();
    
    if(c == '\r') 
    { 
      printf("\n");
      fprintf(fp_itm, "\n");
      count=0;
    }
    else if(c == 8)
    {
      if(count)
      {
        printf("%c[D %c[D", 27, 27);
        fprintf(fp_itm, "%c[D %c[D", 27, 27);
        count--;
      }
    }
    else
    { 
      printf("%c", c);
      fprintf(fp_itm, "%c", c);
      count++;
    }
  }
}




void PrintMessage(void)
{
  int i=0;
  
  printf(" %c", 201);  
  for(i=0; i<35; i++) printf("%c", 205);
  printf("%c", 187);
  
  printf("\n %c", 186);
  printf("%c[%im       Welcome to %c[%imMCBSTM32%c[%im         "               , 27, (TEXT + CYAN), 27, (TEXT + MAGENTA), 27, (TEXTCOLOR));
  printf("%c", 186);
  
  printf("\n %c", 186);
  printf(" %c[%im This is a %c[%imFrameBuffer%c[%im example %c[%im   "   , 27, (TEXT + CYAN), 27, (TEXT + GREEN), 27, (TEXT + CYAN), 27, (TEXTCOLOR));
  printf("%c", 186);
  
  printf("\n %c", 200);  
  for(i=0; i<35; i++) printf("%c", 205);
  printf("%c", 188);
  
  printf("\nEnable %c[%imUSE_BACKGROUND%c[%im in %c[%impicture.h%c[%im\nfor Background Image", 
                        27, (TEXT + CYAN), 27, (TEXTCOLOR), 27, (TEXT + MAGENTA), 27, (TEXTCOLOR));
  
  gotoxy(22, 7);   printf("Move/Press/Hold");  
  gotoxy(22, 8);   printf("the Joystick");
  gotoxy(22, 9);   printf("Press / Hold");  
  gotoxy(22, 10);  printf("User Button and");
  gotoxy(22, 11);  printf("move Joystick");
}

