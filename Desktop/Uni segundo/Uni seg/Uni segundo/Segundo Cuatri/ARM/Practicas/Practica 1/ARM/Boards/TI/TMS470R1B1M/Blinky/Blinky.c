/******************************************************************************/
/* Blinky.c: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2008 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <TMS470R1.h>                   /* TMS470R1 definitions               */
#include <absacc.h>                     /* To enable absolute adressing       */
#ifdef RT_AGENT
#include <stdio.h>                      /* Standard I/O definitions           */
#include <RT_Agent.h>                   /* RT Agent definitions               */
#endif
#include <std_het.h>                    /* TMS470R1 HET definitions           */
#include "HET.h"                        /* Include generated by HET assembly  */
#include "LCD.h"                        /* LCD function prototypes            */

/* HET RAM start (dependant on MFBAHR4 value in TMS470R1B1M.s)                */
#define HET_RAM       0x40000000

/* Frequency of the main clock (needed for calculation of delays for LCD)     */
#define FREQ_MAIN     30000             /* in kHz                             */

#define LCD_TIME      (FREQ_MAIN<<6)    /* Time for initlial LCD display      */
#define BAR_TIME      (FREQ_MAIN>>1)    /* Time for bargraph display (in cyc) */
#define LED_TIME      (FREQ_MAIN<<2)    /* Number of cyc for LED refresh      */
#define DEBOUNCE_TIME (FREQ_MAIN>>5)    /* Number of cyc for button debounce  */

volatile HETPROGRAM0_UN e_HETPROGRAM0_UN __at(HET_RAM);
static unsigned int TimeFlag   = 0;
static unsigned int DebounceOn = 0;

 /* time ---------------------------------------------------------------------\
|                                                                              |
|  This function simulates interrupt timer functionality, counts up each time  |
|  it is called. When it reaches value >= parameter 'cnt' then it sets         |
|  TimeFlag to 1.                                                              |
|                                                                              |
 \---------------------------------------------------------------------------*/

void time (int cnt)  {
  static int counter = 0;
  static int button_state = 0;
  static int debounce_cnt;

  counter ++;

  /* Do the debouncing if button is pressed for a while toggle LED L7         */
  if (DebounceOn) {
    debounce_cnt++;
    if ((debounce_cnt > DEBOUNCE_TIME) && !(pGIO->GIOA.GIODIN & (1 << 4))) {
      DebounceOn  = 0;
      debounce_cnt = 0;
      if (!(pGIO->GIOA.GIODIN & (1 << 4))) {
        button_state ^= 0x01;
        if (button_state)
          pGIO->GIOE.GIODSET = 0x40;
        else
          pGIO->GIOE.GIODCLR = 0x40;
      }
    }
  }

  if (counter >= cnt)  {
    TimeFlag = 1;
    counter  = 0;
  }
}


 /* memcopy32 ----------------------------------------------------------------\
|                                                                              |
|  This function copies 32-bit values to destination from source addresses of  |
|  size also given as parameter                                                |
|  Parameters:                                                                 |
|    dest           - destination address                                      |
|    src            - source      address                                      |
|    size           - size to copy in bytes                                    |
|  Return value:                                                               |
|                                                                              |
 \---------------------------------------------------------------------------*/

void memcopy32 (unsigned int *dest, unsigned int *src, unsigned int size)  {
  size >>= 2;
  while (size--) *dest++ =  *src++;
}


 /* Setup --------------------------------------------------------------------\
|                                                                              |
|  This function setups the HET and starts it setups and GPIOA4 interrupt, it  |
|  is implemented through SWI call because it is required that setting be done |
|  in Supervisory Mode of ARM                                                  |
|  Parameters:                                                                 |
|  Return value:                                                               |
|                                                                              |
 \---------------------------------------------------------------------------*/

void __swi(8) Setup (void);
void __SWI_8      (void) {
  pHET->HETGCR       = 0x00010002;      /* Clock master + ignore suspend      */
  /* Copy HET program from code memory to HET RAM                             */
  memcopy32 ((unsigned int *) &e_HETPROGRAM0_UN, (unsigned int *) HET_INIT0_PST, sizeof(HET_INIT0_PST));
  pHET->HETDCLR      = 0x000000FF;      /* Clear HET0..7                      */
  pHET->HETDIR       = 0x000000FF;      /* Setup HET0..7 as outputs           */
  pHET->HETPRY      |= (1 << 1);        /* Interrupt priority for HET1        */
  pHET->HETPFR       = 0x0000052B;      /* Set prescale factor                */
  pSM->REQMASK      |= (1 << 7);        /* Enable HET interrupt 1             */
  pHET->HETGCR      |= 0x00000001;      /* Start HET                          */

  pGIO->GIOPOL1      = 0x00000000;      /* Falling edge trigger on GIOA4 pin  */
  pGIO->GIOPRY1      = (1 << 4);        /* High-priority interrupt            */
  pGIO->GIOA.GIODIR  = 0x00000000;      /* All GIOA pins are inputs           */
  pGIO->GIOFLG1      = 0x000000FF;      /* Clear interrupt flags for GIOA     */
  pGIO->GIOENA1     |= (1 << 4);        /* Enable GIOA4 pin interrupt         */
  pSM->REQMASK      |= (1 << 5);        /* Enable GIOA interrupt              */

#ifdef RT_AGENT
  /* Setup RTI timer interrupt, every 250us                                   */
  pSM->RTIPCTL       = (7 << 11) | (1000 - 1);
  pSM->RTICMP2       = 15;
  pSM->RTICINT       = 0;
  pSM->RTICINT       = (1 << 4);
  pSM->RTICNTR       = 0;
  pSM->REQMASK      |= (1 << 1);
#endif
}


 /* IRQ_Handler --------------------------------------------------------------\
|                                                                              |
|  This function is a common interrupt handler which handles all interrupt     |
|  requests                                                                    |
|  Parameters:                                                                 |
|  Return value:                                                               |
|                                                                              |
 \---------------------------------------------------------------------------*/

__irq void IRQ_Handler (void) {

  static unsigned int pwm;

  
  switch ((pSM->IRQIVEC & 0xFF) - 1)    /* Identify IRQ request               */
  {
    case 1:                             /* RTI timer interrupt request        */
#ifdef RT_AGENT
      RTA_rx_word_ext();
      RTA_tx_word_ext();
#endif
      pSM->RTICNTR  = 0;
      pSM->RTICINT &= ~(1 << 6);
      pSM->INTREQ  &= ~(1 << 1);
      break;

    case 5:                             /* GIOA interrupt request             */
      DebounceOn = 1;

      pGIO->GIOFLG1 = (1 << 4);         /* Clear interrupt flag for GIOA4     */
      break;

    case 7:                             /* HET1 interrupt request             */
      pwm += 100;
      if (pwm > 10000) pwm = 100;

      /* Edit HET_pwm_a_clear data_word value                                 */
      HET_pwm_a_clear_0.memory.data_word = pwm;
      /* Edit HET_pwm_a_det   data_word value                                 */
      HET_pwm_a_set_0.memory.data_word   = 10100-pwm;

      pHET->HETFLG = (1 << 1);          /* Clear interrupt flag register      */
      break;
  }
}


/* Function for Real-Time Agent                                               */
void RTA_irq_enable_ext (void) {
  pSM->REQMASK |=  (1 << 1);            /* Enable RTI Interrupt               */  
}


/* Function for Real-Time Agent                                               */
void RTA_irq_disable_ext (void) {
  pSM->REQMASK &= ~(1 << 1);            /* Disable RTI Interrupt              */  
}


/******************************************************************************
* Main Program                                                                *
*******************************************************************************/

int main (void) {

  unsigned int  n = 0x0001;
  int LED_time = LED_TIME;
  unsigned long AD_value = 0, AD_old = 0;

#ifdef RT_AGENT
  RTA_Init();
#endif

  /* Setup LED pins                                                           */
  pGIO->GIOE.GIODIR  = 0xFF;            /* Setup GIOE0..7 as outputs          */

  /* Setup and start HET, and setups GIOA4 interrupt generation               */
  Setup ();

  /* Setup AD converter                                                       */
  pMIBADC->ADCR1     = 0x0027;          /* Enable ADC, prescale 8             */
  pMIBADC->ADSAMPEV  = 0x8000;          /* Enable Sample Register             */
  pMIBADC->ADSAMP1   = 0x0020;          /* Sample Window for Group 1          */
  pMIBADC->ADISR1    = 0x0001;          /* Channel 0 in group 1               */
  pMIBADC->ADCR2     = 0x0020;          /* Enable Group 1 Continuous Conv     */

  LCD_init();                           /* Initialize the display             */
                                        /* comment this line if display is    */
                                        /* already initialized                */

  LCD_backlight (1);                    /* Turn the backlight ON              */
  LCD_clear();                          /* Clear the display                  */
  LCD_print (0, 0, "TMS470R1B1M DEMO"); /* Display text                       */
  LCD_print (0, 1, "  www.keil.com  ");

  while (!TimeFlag)                     /* Wait for initial screen display    */
    time(LCD_TIME);

  LCD_backlight (0);                    /* Turn the backlight OFF             */

  LCD_clear();
  LCD_print (0, 0, "  Analog value  ");

  while (1) {                           /* Loop forever                       */

    /* AD convertor functionality                                             */
    if (pMIBADC->ADSR & 0x0004)  {      /* If conversion ended                */
      AD_value = pMIBADC->ADDR0;        /* Read AD_last value                 */
      AD_value /= 10;                   /* Scale to AD_Value to 0 - 100       */
      if (AD_old != AD_value)  {        /* If AD value has changed            */
        LED_time -= BAR_TIME;           /* Shorten LED switch time            */
        AD_old = AD_value;
        LCD_bargraph (0,1,16,AD_value); /* Display bargraph according to AD   */
      }
      pMIBADC->ADSR |= 0x0004;          /* Clear End of Conversion Flag       */
    }

    /* Shift LEDs and turn them ON according to state                         */
    if (TimeFlag)  {
      LED_time = LED_TIME;              /* Reload timig constant              */
      TimeFlag = 0;
      if (n <= 0x00000010) 
        n <<= 1;
      else
        n = 0x0001;

      /* Turn on LEDs according to state                                      */
      pGIO->GIOE.GIODOUT = (pGIO->GIOE.GIODOUT & 0xC0) | (n & 0x3F);

#ifdef RT_AGENT
      printf("AD value = 0x%04X\n", AD_value);
#endif
    }
    time(LED_time);                     /* Signalize after n function calls   */
  }
}
