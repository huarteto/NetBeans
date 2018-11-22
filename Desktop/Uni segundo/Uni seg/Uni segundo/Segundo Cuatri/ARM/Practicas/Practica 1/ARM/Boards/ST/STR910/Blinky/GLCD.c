/******************************************************************************/
/* GLCD.c: Graphic LCD functions for LCD using two SED1520 LCD controllers    */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <91x_lib.h>


extern const unsigned char font5x7[256][6];

enum {LEFT=0,RIGHT};

/************************ Graphic LCD size definitions ************************/

/* Width (in pixels) controlled by one controller                             */ 
#define GCTRL_W     61                      

/* Hight (in pixels) controlled by one controller                             */ 
#define GCTRL_H     32                      

/************************ Graphic LCD pin definitions  ************************/

#define DATA GPIO8->DR[0x3FC]           /* Data bits D0 = P8.0 .. DB7 = P8.7  */
#define E1   GPIO9->DR[0x040]           /* Left  portion of screen enable     */
#define E2   GPIO9->DR[0x080]           /* Right portion of screen enable     */
#define EN   GPIO9->DR[0x0C0]           /* Both enable bits (left and right)  */
#define RW   GPIO9->DR[0x100]           /* Read/Write control                 */
#define DI   GPIO9->DR[0x200]           /* Data/Instruction control           */


/************************ Global function definitions *************************/


/******************************************************************************
* Delay in while loop cycles                                                  * 
*   Parameter:     cnt:    number of while cycles to delay                    * 
*   Return:                                                                   * 
******************************************************************************/

void delay (int cnt)
{
  while (cnt--);
}


/******************************************************************************
* Write command to graphic controller                                         * 
*   Parameter:     c:      command to be written                              * 
*                  enable: controller enable (left (0) or right (1))          * 
*   Return:                                                                   * 
******************************************************************************/

void glcd_write_cmd (unsigned char c, char enable)
{
  RW   = 0;
  DI   = 0;                             /* Instruction                        */
  delay(10);
  EN   = 0x10 << enable;
  delay(10);
  DATA = c;
  delay(10);
  EN   = 0;
  delay(10);
}


/******************************************************************************
* Write data to graphic controller                                            * 
*   Parameter:     c:      data to be written                                 * 
*                  enable: controller enable (left (0) or right (1))          * 
*   Return:                                                                   * 
******************************************************************************/

void glcd_write_data (unsigned char c, char enable)
{
  RW   = 0;
  DI   = 0xFF;                          /* Data                               */
  delay(10);
  EN   = 0x10 << enable;
  delay(10);
  DATA = c;
  delay(10);
  EN   = 0;
  delay(10);
}


/******************************************************************************
* Clear the LCD display                                                       * 
*   Parameter:                                                                * 
*   Return:                                                                   * 
******************************************************************************/

void glcd_clear (void)
{
  int i, j;

  for (i = 0; i < (GCTRL_H>>3); i++)  { /* Pages                              */
    glcd_write_cmd(0xB8+i,  LEFT);      /* Set page (Y address) at i (left)   */
    glcd_write_cmd(0x00,    LEFT);      /* Set X address at 0        (left)   */
    glcd_write_cmd(0xB8+i,  RIGHT);     /* Set page (Y address) at i (right)  */
    glcd_write_cmd(0x00,    RIGHT);     /* Set X address at 0        (right)  */

    glcd_write_cmd(0xE0, LEFT);         /* Enter read modify write mode       */
    glcd_write_cmd(0xE0, RIGHT);        /* Enter read modify write mode       */
    for (j = 0; j < GCTRL_W; j++)  {    /* Bytes per page (X autoincrement)   */
      glcd_write_data(0x00, LEFT);      /* Write 0 to left  half of display   */
      glcd_write_data(0x00, RIGHT);     /* Write 0 to right half of display   */
    }
    glcd_write_cmd(0xEE, LEFT);         /* Exit read modify write mode        */
    glcd_write_cmd(0xEE, RIGHT);        /* Exit read modify write mode        */
  }

  glcd_write_cmd(0xB8, LEFT);           /* Set page (Y address) at 0 (left)   */
  glcd_write_cmd(0x00, LEFT);           /* Set X address at 0        (left)   */
  glcd_write_cmd(0xB8, RIGHT);          /* Set page (Y address) at 0 (right)  */
  glcd_write_cmd(0x00, RIGHT);          /* Set X address at 0        (right)  */
}


/******************************************************************************
* Initialize the LCD display (initialize 2 graphic controllers)               * 
*   Parameter:     c:      data to be written                                 * 
*                  enable: controller enable (left (0) or right (1))          * 
*   Return:                                                                   * 
******************************************************************************/

void glcd_init (void)
{ 

  /* Reset controller of the LEFT part of the screen                          */

  glcd_write_cmd(0xAE, LEFT);           /* Display OFF                        */
  glcd_write_cmd(0xC0, LEFT);           /* Set start line to 0                */
  glcd_write_cmd(0xA5, LEFT);           /* Static drive OFF                   */
  glcd_write_cmd(0x00, LEFT);           /* Set column address to 0            */
  glcd_write_cmd(0xB8, LEFT);           /* Set page address to 0              */
  glcd_write_cmd(0xA9, LEFT);           /* Set duty 1/32                      */
  glcd_write_cmd(0xA0, LEFT);           /* Set ADC forward                    */

  glcd_write_cmd(0xA4, LEFT);           /* Static drive ON                    */
  glcd_write_cmd(0xAF, LEFT);           /* Display ON                         */
  glcd_write_cmd(0xEE, LEFT);           /* Exit read modify write mode        */

  /* Reset controller of the RIGHT part of the screen                         */

  glcd_write_cmd(0xAE, RIGHT);          /* Display OFF                        */
  glcd_write_cmd(0xC0, RIGHT);          /* Set start line to 0                */
  glcd_write_cmd(0xA5, RIGHT);          /* Static drive OFF                   */
  glcd_write_cmd(0x00, RIGHT);          /* Set column address to 0            */
  glcd_write_cmd(0xB8, RIGHT);          /* Set page address to 0              */
  glcd_write_cmd(0xA9, RIGHT);          /* Set duty 1/32                      */
  glcd_write_cmd(0xA0, RIGHT);          /* Set ADC forward                    */

  glcd_write_cmd(0xA4, RIGHT);          /* Static drive ON                    */
  glcd_write_cmd(0xAF, RIGHT);          /* Display ON                         */
  glcd_write_cmd(0xEE, RIGHT);          /* Exit read modify write mode        */
}


/******************************************************************************
* Display graphical bitmap image at position x horizontally and y vertically  * 
* (Restriction: starting x and y position must be divisible by 8)             * 
*   Parameter:     x:      horizontal position                                * 
*                  y:      vertical position                                  * 
*                  w:      width of bitmap                                    * 
*                  h:      height of bitmap                                   * 
*                  bitmap: address at which the bitmap data resides           * 
*   Return:                                                                   * 
******************************************************************************/

void glcd_bitmap (int x, int y, int w, int h, const unsigned char *bitmap)
{
  int i, j, i_temp;

  /* Drawing left screen portion of bitmap                                    */
  if (x < GCTRL_W)
  {
    if ((x+w) > (GCTRL_W-1)) i_temp = GCTRL_W;
    else i_temp = x+w;

    for (j = y>>3; j < ((y>>3)+(h>>3)); j++) {  /* Pages                      */
      glcd_write_cmd(0xB8+j, LEFT);     /* Set page (Y address) at j          */
      glcd_write_cmd(x,      LEFT);     /* Set X address at x                 */
      glcd_write_cmd(0xE0,   LEFT);     /* Enter read modify write mode       */
      for (i = x; i < i_temp; i++)  {   /* Bytes at current page              */
        glcd_write_data(*bitmap++,LEFT);/* Write data                         */
      }
      glcd_write_cmd(0xEE, LEFT);       /* Exit read modify write mode        */
      if ((x+w) >= GCTRL_W)  {
        glcd_write_cmd(0xB8+j, RIGHT);  /* Set page (Y address) at j          */
        glcd_write_cmd(0,      RIGHT);  /* Set X address at 0                 */
        glcd_write_cmd(0xE0,   RIGHT);  /* Enter read modify write mode       */
        for (i = 0; i < (x+w-GCTRL_W); i++) {   /* Bytes at current page      */
          glcd_write_data(*bitmap++,RIGHT);     /* Write data                 */
        }
        glcd_write_cmd(0xEE, RIGHT);    /* Exit read modify write mode        */
      }
    }
  }

  /* Drawing right screen portion of bitmap                                   */
  else  {
    for (j = y>>3; j < ((y>>3)+(h>>3)); j++) {/* Pages                        */
      glcd_write_cmd(0xB8+j,RIGHT);     /* Set page (Y address) at j          */
      glcd_write_cmd((x-61),RIGHT);     /* Set X address at (x-61)            */
      glcd_write_cmd(0xE0,RIGHT);       /* Enter read modify write mode       */
      for (i = x; i < (x+w); i++)  {    /* Bytes at current page              */
        glcd_write_data(*bitmap++,RIGHT);       /* Write data                 */
      }
      glcd_write_cmd(0xEE,RIGHT);       /* Exit read modify write mode        */
    }
  }
}


/******************************************************************************
* Print string on LCD display                                                 * 
*   Parameter:     pos_x:  horizontal position (in characters)                * 
*                  pos_y:  vertical position   (in characters)                * 
*                  font:   font index (only font 0 implemented)               * 
*                  string: address of string                                  * 
*   Return:                                                                   * 
******************************************************************************/

void glcd_print (int pos_x, int  pos_y, int font, const unsigned char *string)
{
  int i = 0;

  switch (font)  {
    case 0: 
      while (*string) glcd_bitmap(6*(pos_x+i++),8*pos_y,6,8,&font5x7[*string++][0]);
      break;
    case 1: break;
    case 2: break;
    case 3: break;
  }
}


/******************************************************************************/
