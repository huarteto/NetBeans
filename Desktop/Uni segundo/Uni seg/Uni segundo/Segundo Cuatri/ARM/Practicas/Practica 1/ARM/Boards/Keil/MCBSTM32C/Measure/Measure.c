/*----------------------------------------------------------------------------
 *      Name:    MEASURE.C
 *      Purpose: Remote Measurement Recorder
 *----------------------------------------------------------------------------
 *      This file is part of the uVision/ARM development tools.
 *      This software may only be used under the terms of a valid, current,
 *      end user licence from KEIL for a compatible version of KEIL software
 *      development tools. Nothing else gives you the right to use it.
 *
 *      Copyright (c) 2005-2009 Keil Software.
 *---------------------------------------------------------------------------*/

#include <stdio.h>                          /* standard I/O .h-file           */
#include <ctype.h>                          /* character functions            */
#include <stm32f10x_cl.h>

#include "measure.h"                        /* global project definition file */
#include "GLCD.h"                                 // GLCD function prototypes


unsigned long TimeTick;                     /* counts 10ms timeTicks          */


const char menu[] =
   "\n"
   "+***************** REMOTE MEASUREMENT RECORDER *****************+\n"
   "| This program is a simple Measurement Recorder based on the    |\n"
   "| STM32F107 controller. It records the state of GPIOD (Joystick)|\n"
   "| and the voltage on the analog input ADC1 channel14.           |\n"
   "+ command -+ syntax -----+ function ----------------------------+\n"
   "| Read     | R [n]       | read <n> recorded measurements       |\n"
   "| Display  | D           | display current measurement values   |\n"
   "| Time     | T hh:mm:ss  | set time                             |\n"
   "| Interval | I mm:ss.ttt | set interval time                    |\n"
   "| Clear    | C           | clear measurement records            |\n"
   "| Quit     | Q           | quit measurement recording           |\n"
   "| Start    | S           | start measurement recording          |\n"
   "+----------+-------------+--------------------------------------+\n";

struct interval setinterval;                /* interval setting values        */
struct interval interval;                   /* interval counter               */

volatile int measurement_interval = 0;      /* measurement interval over      */
volatile int mdisplay = 0;                  /* measurement display requested  */
volatile int startflag = 0;                 /* start measurement recording    */

struct mrec current;                        /* current measurements           */

#define SCNT 20                             /* number of records              */

struct mrec save_record[SCNT];              /* buffer for measurements        */
int sindex;                                 /* save index                     */
int savefirst;                              /* save first index               */

char ERROR_STR [] = "\n*** ERROR: %s\n";    /* ERROR message string in code   */

#define WRONGINDEX 0xffff                   /* error signal for wrong index   */


/******************************************************************************/
/*               Save current measurements in save_record                     */
/******************************************************************************/
static void save_current_measurements (void) {
  save_record[sindex++] = current;             /* copy current measurements   */
  if (sindex == SCNT) sindex = 0;              /* check bounds of sindex      */
  if (sindex == savefirst)  {                  /* check circular buffer limits*/
    if (++savefirst == SCNT)  savefirst = 0;   /* check bounds of savefirst   */
  }
}


/******************************************************************************/
/*               Initialises the LEDs and the Joystick                        */
/******************************************************************************/
void io_Init (void) {

  RCC->APB2ENR |= 0x00000070;

  RCC->APB2ENR |= (1<<4);                      /* enable clock for GPIOD      */
  GPIOD->CRH &= 0x00000FFF;                    /* Configure GPIO for JOYSTICK */
  GPIOD->CRH |= 0x44444000;

  RCC->APB2ENR |= (1<<6);                      /* enable clock for GPIOE      */
  GPIOE->CRH  = 0x33333333;                    /* Configure GPIO for LEDs     */

}

/******************************************************************************/
/*               Initialises the TIM3 for 1ms @ 72MHz                         */
/******************************************************************************/
void tim3_Init (void) {

  RCC->APB1ENR |= (1<<1);                      /* enable clock for TIM3       */

  TIM3->PSC = 3;                               /* set prescaler               */
  TIM3->ARR = (18000000UL / 1000UL) - 1;       /* set auto-reload             */
  TIM3->CR1 = 0;                               /* reset command register 1    */
  TIM3->CR2 = 0;                               /* reset command register 2    */
  TIM3->DIER = (1<<0);                         /* Update interrupt enabled    */
  TIM3->CR1 |= (1<<0);                         /* Enable Timer                */

  NVIC_EnableIRQ(TIM3_IRQn);                   /* enable TIM3 interrupt       */

}

/******************************************************************************/
/*               Initialises the Analog/Digital converter                     */
/*               PC4 (ADC Channel14) is used as analog input                  */
/******************************************************************************/
void adc_Init (void) {

  RCC->APB2ENR |= (1<<4);                      /* enable clock for GPIOC      */
  GPIOC->CRL &= 0xFFF0FFFF;                    /* Configure ADC.14 input.     */

  RCC->APB2ENR |= (1<<9);                      /* enable clock for ADC1       */
  ADC1->SQR1  = 0x00000000;                    /* one conversion              */
  ADC1->SQR3  = (14<< 0);                      /* set order to chn14          */
  ADC1->SMPR1 = ( 5<<12);                      /* sample time (55,5 cycles)   */ 

  ADC1->CR1   =  0x00000100;                   /* independant mode, SCAN mode */
  ADC1->CR2   =  0x000E0003;                   /* data align right            */
                                               /* continuous conversion       */
                                               /* EXTSEL = SWSTART            */
                                               /* enable ADC                  */ 
  ADC1->CR2  |= 0x00000008;                    /* Reset calibration           */
  while (ADC1->CR2 & 0x00000008);

  ADC1->CR2  |= 0x00000004;                    /* Start calibration           */
  while (ADC1->CR2 & 0x00000004);

  ADC1->CR2  |=  0x00500000;                   /* start SW conversion         */

}



/******************************************************************************/
/*                Timer Counter 3 interrupt service function                  */
/*                executes each 1ms @ 25 MHz Crystal Clock                     */
/******************************************************************************/
void TIM3_IRQHandler (void) {

  if (TIM3->SR & (1<<0)) {                     /* UIF interrupt ?             */
    if (measurement_interval)  {               /* measurement done ?          */
      save_current_measurements ();            /* yes -> save measurements    */
      measurement_interval = 0;                /* Save measurements done      */
    }
                                               /* check if interval is over   */
    if (interval.min  == 0 &&
        interval.sec  == 0 &&
        interval.msec == 0     )  {
      interval = setinterval;                  /* set interval time again     */
      measurement_interval = startflag;        /* determine measurement flag  */
    }
    else  {                                    /* update interval time        */
      if (interval.msec-- == 0)  {             /* calculate millisecond       */
        interval.msec = 999;
        if (interval.sec-- == 0)  {            /* calculate second            */
          interval.sec = 59;
          interval.min--;                      /* calculate minute            */
        }
      }
    }
                                               /* update current time         */
    if (++current.time.msec == 1000)  {        /* update millisecond cnt      */
      current.time.msec = 0;

      if (++current.time.sec == 60)  {         /* update second counter       */
        current.time.sec = 0;

        if (++current.time.min == 60)  {       /* update minute counter       */
          current.time.min = 0;

          if (++current.time.hour == 24)  {    /* update hour counter         */
            current.time.hour = 0;
          }
        }
      }
    }	/* end of if( ++current.time.msec... */

    if (current.time.msec == 0) {
      GPIOE->BSRR = (1<<8);                    /* Turn LED#1 On               */
    }
    if (current.time.msec == 100) {
      GPIOE->BRR  = (1<<8);                    /* Turn LED#1 Off              */
    }
  
    if (measurement_interval || mdisplay)  {   /* process measurement         */
      current.gpio = GPIOD->IDR;               /* Joystick (right,left,up,sel)*/

      current.analog[0] = (ADC1->DR & 0x0FFF); /* AD value (12 bit)           */

      mdisplay = 0;                            /* mdisplay = 0 for ready sig. */
    }

    TIM3->SR &= ~(1<<0);                       /* clear UIF flag              */
  }

}


/******************************************************************************/
/*                       Calculate first Read Index                           */
/******************************************************************************/
static int read_index (char *buffer) {
  int index = 0;
  int args;

  if (setinterval.min == 0     &&              /* check if setinterval is     */
      setinterval.sec == 0     &&              /* below 1 second and          */
      setinterval.msec < 999   &&              /* measurements are collected  */
      startflag                   )  {
    printf (ERROR_STR, "QUIT MEASUREMENTS BEFORE READ");
    return (WRONGINDEX);                       /* no display on the fly if    */
  }                                            /* interval time < 1 second    */
  args = sscanf (buffer, "%d", &index);        /* scan input for read count   */
  if (args == 0  ||  index == 0  || args == EOF)  index = SCNT-1;
  index = sindex - index;                      /* calculate first read index  */
  if (index < 0) index += SCNT;                /* from read count             */
  return (index);
}


/******************************************************************************/
/*                         Clear Measurement Records                          */
/******************************************************************************/
static void clear_records (void) {
  int idx;                                     /* index for loop              */

  startflag = 0;                               /* stop measurement collecting */
  sindex = savefirst = 0;                      /* reset circular buffer index */
  for (idx = 0; idx != SCNT; idx++)  {         /* mark all records unused     */
    save_record[idx].time.hour = 0xff;         /* unused flag: hour = 0xff    */
  }
}

/******************************************************************************/
/*                         Update LCD Display                                 */
/******************************************************************************/
void updLCD (unsigned char state) {
  char sDisplay[20];                           /* buffer used for sprintf()   */

  switch (state) {
    case 0:
      GLCD_SetTextColor(Blue);
      GLCD_DisplayString(6, 0, 1, "GPIOD: ");
      GLCD_DisplayString(7, 0, 1, "ADC1.14: ");
      GLCD_DisplayString(8, 0, 1, "Time: ");
      break;

    case 1:
      GLCD_SetTextColor(Red);
      sprintf(sDisplay, "%02X", (int)current.gpio);
      GLCD_DisplayString(6, 8, 1, (unsigned char *)sDisplay);
      sprintf(sDisplay, "%4.2fV", (float)current.analog[0] * 3.3 / 0xFFF);
      GLCD_DisplayString(7, 9, 1, (unsigned char *)sDisplay);
      sprintf(sDisplay, "%02d:%02d:%02d.%03d", current.time.hour, current.time.min, current.time.sec, current.time.msec);
      GLCD_DisplayString(8, 6, 1, (unsigned char *)sDisplay);

      GLCD_SetTextColor(Green);
    
      if (current.gpio & (1<<15))                                    /* evaluate  "Left" Key */
        GLCD_DisplayChar(6, 17, 1, 0x80+8);                          /* key released         */
      else
        GLCD_DisplayChar(6, 17, 1, 0x80+9);                          /* key pressed          */

      if (current.gpio & (1<<13))                                    /* evaluate "Right" Key */
        GLCD_DisplayChar(6, 19, 1, 0x80+10);
      else
        GLCD_DisplayChar(6, 19, 1, 0x80+11);

      if (current.gpio & (1<<12))                                    /* evaluate    "Up" Key */
        GLCD_DisplayChar(5, 18, 1, 0x80+4);
      else
        GLCD_DisplayChar(5, 18, 1, 0x80+5);

      if (current.gpio & (1<<11))                                    /* evaluate   "Sel" Key */
        GLCD_DisplayChar(6, 18, 1, 0x80+0);
      else
        GLCD_DisplayChar(6, 18, 1, 0x80+1);
      
      break;

    case 2:
      GLCD_SetTextColor(White);
      GLCD_ClearLn(Line5, 1);
      GLCD_ClearLn(Line6, 1);
      GLCD_ClearLn(Line7, 1);
      GLCD_ClearLn(Line8, 1);
      GLCD_ClearLn(Line9, 1);
      break;
  }

}


/******************************************************************************/
/***************************      MAIN PROGRAM      ***************************/
/******************************************************************************/
int main (void)  {                             /* main entry for program      */
  char cmdbuf [15];                            /* command input buffer        */
  int i;                                       /* index for command buffer    */
  int idx;                                     /* index for circular buffer   */

  /*--------------------------------------------------------------------------*/
  SystemInit();                                /* Initialize clocks           */
  io_Init();                                   /* Initialize LEDs & Joystick  */
  tim3_Init();                                 /* Initialize Timer #3         */
  adc_Init();                                  /* Initialise A/D converter    */
  init_serial();                               /* Initialize USART #2         */

  GLCD_Init();                                 /* Initialize the GLCD         */
  GLCD_Clear(White);                           /* Clear the GLCD              */

  GLCD_SetBackColor(Blue);                     /* Set the Back Color          */
  GLCD_SetTextColor(White);                    /* Set the Text Color          */
  GLCD_DisplayString(0, 0, 1, "   MCBSTM32C Demo   ");
  GLCD_DisplayString(1, 0, 1, "      Measure       ");
  GLCD_DisplayString(2, 0, 1, "    www.keil.com    ");
  GLCD_SetBackColor(White);                    /* Set the Back Color          */
  GLCD_SetTextColor(Blue);                     /* Set the Text Color          */

  clear_records ();                            /* initialize circular buffer  */
  printf ( menu );                             /* display command menu        */

  while (1)  {                                 /* loop forever                */
    printf ("\nCommand: ");
    getline (&cmdbuf[0], sizeof (cmdbuf));     /* input command line          */

    for (i = 0; cmdbuf[i] != 0; i++)  {        /* convert to upper characters */
      cmdbuf[i] = toupper(cmdbuf[i]);
    }

    for (i = 0; cmdbuf[i] == ' '; i++);        /* skip blanks                 */

    switch (cmdbuf[i])  {                      /* proceed to command function */

      case 'R':                                /* Read circular Buffer        */
        if ((idx = read_index (&cmdbuf[i+1])) == WRONGINDEX)  break;
        while (idx != sindex)  {               /* check end of table          */
          if (USART2->SR & 0x0020)  {          /* check serial interface      */
            if (getkey() == 0x1B) break;       /* for escape character        */
          }
          if (save_record[idx].time.hour != 0xff)  {
            measure_display (save_record[idx]);      /* display record        */
            printf ("\n");
          }
          if (++idx == SCNT) idx = 0;          /* next circular buffer entry  */
        }
        break;

      case 'T':                                /* Enter Current Time          */
        set_time (&cmdbuf[i+1]);
        break;

      case 'I':                                /* Enter Interval Time         */
        set_interval (&cmdbuf[i+1]);
        break;

      case 'D':                                /* Display Command             */
        printf ("\nDisplay current Measurements: (ESC to abort)\n");
        updLCD (0);                            /* display initial Text        */
        do  {
          while (!(USART2->SR & 0x0020))  {    /* check serial interface      */
            mdisplay = 1;                      /* request measurement         */
            while (mdisplay);                  /* wait for measurement        */
            measure_display (current);         /* display values              */
            updLCD (1);                        /* update display              */
          }
        } while (getkey () != 0x1B);           /* escape terminates command   */
        printf ("\n\n");
        updLCD (2);                            /* cleser used display lines   */
        break;

      case 'S':                                /* Start Command               */
        printf ("\nStart Measurement Recording\n");
        startflag = 1;
        break;

      case 'Q':                                /* Quit Command                */
        printf ("\nQuit Measurement Recording\n");
        startflag = 0;
        break;

      case 'C':                                /* Clear Command               */
        printf ("\nClear Measurement Records\n");
        clear_records ();
        break;

      default:                                 /* Error Handling              */
        printf (ERROR_STR, "UNKNOWN COMMAND");
        printf (menu);                         /* display command menu        */
        break;
    }
  }
}

