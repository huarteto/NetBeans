/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <ADuC7024.H>                      /* ADuC7024 definitions */

void wait (void)  {                        /* wait function */
  unsigned long i;                         /* Delay var */

  for (i = 0; i < 100000; )  {             /* Delay for 10000 Counts */
    i++;
  }
}

#ifdef P42LED_ONLY
int main (void)  {
  GP4DAT = 0x04000000;                     /* P4.2 defined as Output */
  while (1) {                              /* Loop forever */
    GP4DAT ^= 0x00040000;                  /* Toggle P4.2 LED */
    wait ();                               /* call wait function */
  }
}
#else
int main (void)  {
  unsigned int n;                          /* LED var */

  GP4DAT = 0xFF000000;                     /* P4.0..7 defined as Outputs */
  while (1) {                              /* Loop forever */
    for (n = 0x010000; n < 0x800000; n <<= 1) {
      /* Blink LED 0, 1, 2, 3, 4, 5, 6 */
      GP4SET = n;                          /* Turn on LED */
      wait ();                             /* call wait function */
      GP4CLR = n;                          /* Turn off LED */
    }
    for (n = 0x800000; n > 0x010000; n >>=1) {
      /* Blink LED 7, 6, 5, 4, 3, 2, 1 */
      GP4SET = n;                          /* Turn on LED */
      wait ();                             /* call wait function */
      GP4CLR = n;                          /* Turn off LED */
    }
  }
}
#endif
