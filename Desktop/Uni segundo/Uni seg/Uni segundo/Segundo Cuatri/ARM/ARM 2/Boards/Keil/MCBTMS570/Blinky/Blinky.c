/******************************************************************************/
/* BLINKY.C: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2010 Keil Software. All rights reserved.                     */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <stdio.h>
#include <TMS570.H>
#include "GLCD.h"

#define LED_NUM    6                    /* Number of user LEDs                */
const unsigned long led_gioMask[]  = { 1UL<< 0, 1UL<< 3, 1UL<< 4,  1UL<< 5};
const unsigned long led_nhetMask[] = { 1UL<< 1, 1UL<< 31};

char  text[40];

extern void EMIF_MPU_init(void);
extern void SER_init (void);
extern void TIM_init (void);
extern void NHET_Init(void);
extern void adcInit(void);

extern unsigned int clock_1s;
extern unsigned int AD_Light_value;
extern unsigned int AD_Temp_value;


/*----------------------------------------------------------------------------
  Function that initializes ADC
 *----------------------------------------------------------------------------*/
void ADC_init (void) {
  adcInit();
}


/*----------------------------------------------------------------------------
  Function that initializes LEDs
 *----------------------------------------------------------------------------*/
void LED_init(void) {
  gioInit();
  NHET_Init();
}

/*----------------------------------------------------------------------------
  Function that turns on requested LED
 *----------------------------------------------------------------------------*/
void LED_On (unsigned int num) {
  if (num < 4) {
    gioPORTA->DSET = led_gioMask[num];
  } else {
    nhetREG->CCDSET = led_nhetMask[num-4];
  }
}

/*----------------------------------------------------------------------------
  Function that turns off requested LED
 *----------------------------------------------------------------------------*/
void LED_Off (unsigned int num) {
  if (num < 4) {
    gioPORTA->DCLR = led_gioMask[num];
  } else {
    nhetREG->CCDCLR = led_nhetMask[num-4];
  }
}

/*----------------------------------------------------------------------------
  Function that outputs value to LEDs
 *----------------------------------------------------------------------------*/
void LED_Out(unsigned int value) {
  int i;

  for (i = 0; i < LED_NUM; i++) {
    if (value & (1<<i)) {
      LED_On (i);
    } else {
      LED_Off(i);
    }
  }
}


/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {                       /* Main Program                       */
  unsigned int AD_Light_last, AD_Light_print = 0;
  unsigned int AD_Temp_last,  AD_Temp_print  = 0;
  unsigned int TempLimitLow, TempLimitHigh;

  EMIF_MPU_init();                      /* EMIF MPU Initialization            */
  LED_init();                           /* LED Initialization                 */
  ADC_init();                           /* ADC Initialization                 */
  SER_init();                           /* Serial IF initialization           */
  GLCD_Init();                          /* GLCD Initialization                */
  TIM_init();

  GLCD_Clear(White);                    /* Clear graphical LCD display        */
  GLCD_SetBackColor(Blue);
  GLCD_SetTextColor(White);
  GLCD_DisplayString(0, 0, "   MCBTMS570 Demo   ");
  GLCD_DisplayString(1, 0, "       Blinky       ");
  GLCD_DisplayString(2, 0, "    www.keil.com    ");
  GLCD_SetBackColor(White);
  GLCD_SetTextColor(Blue);

  GLCD_DisplayString(4, 0, " Light:");
  GLCD_DisplayString(5, 0, " Temp: ");

  TempLimitLow  = 0x0840;
  TempLimitHigh = 0x0840 + 80;
  clock_1s = 0;
  while (clock_1s == 0);                /* wait until light/temp buffer full  */
  
  while (1) {                           /* Loop forever                       */

    if ((AD_Light_value != AD_Light_last) || 
	    (AD_Temp_value  != AD_Temp_last)    ) {
      AD_Light_last  = AD_Light_value;
      AD_Temp_last   = AD_Temp_value;
      AD_Light_print = AD_Light_last;   /* Get unscaled value for printout    */
      AD_Temp_print  = AD_Temp_last;    /* Get unscaled value for printout    */
      GLCD_SetTextColor(Red);
      GLCD_Bargraph (144, Line4, 160, 20, (AD_Light_last * 160 ) >> 12);
      if (AD_Temp_last < (TempLimitLow  - 10)) TempLimitLow  = AD_Temp_last - 10;
      if (AD_Temp_last > (TempLimitHigh + 10)) TempLimitHigh = AD_Temp_last + 10;
      GLCD_Bargraph (144, Line5, 160, 20, ((AD_Temp_last - TempLimitLow) * 160) / (TempLimitHigh - TempLimitLow));
      GLCD_SetTextColor(White);
    }

    /* print out unscaled values */
    if (clock_1s) {
      clock_1s = 0;

      GLCD_SetTextColor(Blue);
      sprintf(text, " AD Light: 0x%04X", AD_Light_print);
      GLCD_DisplayString(7, 0,  (char *)text);
      printf ("%s\n\r", text);

      sprintf(text, " AD Temp : 0x%04X", AD_Temp_print);
      GLCD_DisplayString(8, 0,  (char *)text);
      printf ("%s\n\r", text);
    }

  }
}
