/*----------------------------------------------------------------------------
 * Name:    Getline.c
 * Purpose: Line Edited Character Input
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
 *----------------------------------------------------------------------------*/

#include <stdio.h>

#define _CNTLQ       0x11             /* Control Q       */
#define _CNTLS       0x13             /* Control S       */
#define _DEL         0x7F             /* Delete          */
#define _BS          0x08             /* Backspace       */
#define _CR          0x0D             /* Carriage Return */
#define _LF          0x0A             /* Line Feed       */

extern int getkey (void);             /* external function:  input character */

/*----------------------------------------------------------------------------
 *      Line Editor
 *---------------------------------------------------------------------------*/
void getline (char *line, int n) {
  int  cnt = 0;
  char c;

  do {
    if ((c = getkey ()) == _CR)  c = _LF;  /* read character                 */
    if (c == _BS  ||  c == _DEL) {         /* process backspace              */
      if (cnt != 0)  {            
        cnt--;                             /* decrement count                */
        line--;                            /* and line pointer               */
        putchar (_BS);                     /* echo backspace                 */
        putchar (' ');
        putchar (_BS);
      }
    }
    else if (c != _CNTLQ && c != _CNTLS) { /* ignore Control S/Q             */
      putchar (*line = c);                 /* echo and store character       */
      line++;                              /* increment line pointer         */
      cnt++;                               /* and count                      */
    }
  } while (cnt < n - 1  &&  c != _LF);     /* check limit and line feed      */
  *(line - 1) = 0;                         /* mark end of string             */
}

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/

