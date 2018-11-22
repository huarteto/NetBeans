/*----------------------------------------------------------------------------
 * Name:    Measure.c
 * Purpose: Remote Measurement Recorder
 * Version: V1.00
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
 *----------------------------------------------------------------------------
 * History:
 *          V1.00 Initial Version
 *----------------------------------------------------------------------------*/

#include <stdio.h>                          /* standard I/O .h-file           */
#include <ctype.h>                          /* character functions            */
#include <SAM3U.H>                          /* ATSAM3U definitions            */

#include "Serial.h"
#include "Measure.h"                        /* global project definition file */

const char menu[] =
   "\n"
   "+***************** REMOTE MEASUREMENT RECORDER *****************+\n"
   "| This program is a simple Measurement Recorder. It is based on |\n"
   "| the SAM3U and records the state of Buttons USR-LEFT, USR-RIGHT|\n"
   "| and the voltage on the four analog inputs ADC0 trough ADC1.   |\n"
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


/*----------------------------------------------------------------------------
  Save current measurements in save_record
 *----------------------------------------------------------------------------*/
static void save_current_measurements (void) {
  save_record[sindex++] = current;             /* copy current measurements   */
  if (sindex == SCNT) sindex = 0;              /* check bounds of sindex      */
  if (sindex == savefirst)  {                  /* check circular buffer limits*/
    if (++savefirst == SCNT)  savefirst = 0;   /* check bounds of savefirst   */
  }
}


#define ADC_CLK           5000000UL               /* ADC clock                */
#define ADC_STARTUP            10UL               /* ADC startup time         */
#define ADC_SAMPLE_HOLD      1200UL               /* ADC sample and hold time */
/*----------------------------------------------------------------------------
  Initialises the Analog/Digital converter
  PB4  ADC0 channel 3
  PB5  ADC1 channel 0
 *----------------------------------------------------------------------------*/
void adc_Init (void) {
  volatile unsigned int data;
  unsigned int prescal = (SystemFrequency / (2*ADC_CLK)) - 1;
  unsigned int startup = ((ADC_CLK/1000000) * ADC_STARTUP / 8) - 1;
  unsigned int shtim   = (((ADC_CLK/1000000) * ADC_SAMPLE_HOLD)/1000) - 1;

  PMC->PMC_PCER  |= ((1UL << ID_PIOB)   |          /* enable clock for PIOB */
                     (1UL << ID_ADC12B) |          /* enable clock for ADC0 */
                     (1UL << ID_ADC)     );        /* enable clock for ADC1 */ 
                       
  PIOB->PIO_IDR = 
  PIOB->PIO_ODR = 
  PIOB->PIO_PER = (PIO_PB5 | PIO_PB4);             /* configure PB4, PB4 as input       */

  ADC12B->ADC_CR   = ADC_CR_SWRST;                           /* SW reset */

  ADC12B->ADC_MR   = ((0x0     <<  0) | /* (ADC) Hardware triggers are disabled. Starting a conversion is only possible by software */
                      (0x1     <<  4) | /* (ADC) 10-bit resolution */
                      (0x0     <<  5) | /* (ADC) Normal Mode */
                      (prescal <<  8) | /* prescaler */
                      (startup << 16) | /* startup time */
                      (shtim   << 24)   /* SHTIM */
                                       );
  ADC12B->ADC_ACR  = 0;                                         /*                  */ 
  ADC12B->ADC_CHER = ADC_CHER_CH3;                             /* enable channel 3 */ 
  data = ADC12B->ADC_CDR[3] & 0x3FF;                              /* AD value (10 bit) */

  ADC12B->ADC_CR   = ADC_CR_START;                           /* Start conversion */ 

  ADC->ADC_CR      = ADC_CR_SWRST;                           /* SW reset */

  ADC->ADC_MR      = ((0x0     <<  0) | /* (ADC) Hardware triggers are disabled. Starting a conversion is only possible by software */
                      (0x0     <<  5) | /* (ADC) Normal Mode */
                      (prescal <<  8) | /* prescaler */
                      (startup << 16) | /* startup time */
                      (shtim   << 24)   /* SHTIM */
                                       );
                                                   
  ADC->ADC_CHER    = ADC_CHER_CH0;                             /* enable channel 0 */ 
  data = ADC->ADC_CDR[0] & 0x3FF;                              /* AD value (10 bit) */
  ADC->ADC_CR      = ADC_CR_START;                           /* Start conversion */ 
}

/*----------------------------------------------------------------------------
  Initialises the LEDs and the Push Buttons
 *----------------------------------------------------------------------------*/
void io_Init (void) {

  PMC->PMC_PCER  |= ((1UL << ID_PIOA) |           /* enable clock for PIOA             */
                     (1UL << ID_PIOB)  );         /* enable clock for PIOB             */  

  PIOA->PIO_IDR  = 
  PIOA->PIO_ODR  = 
  PIOA->PIO_PER  =  (PIO_PA19 | PIO_PA18); /* Setup Pins PA18..PA19 for Buttons */

  PIOB->PIO_IDR  = 
  PIOB->PIO_OER  =
  PIOB->PIO_PER  =  (PIO_PB1  | PIO_PB0);  /* Setup Pins PB0..PB1 for LEDs      */
  PIOB->PIO_SODR =  (PIO_PB1  | PIO_PB0);  /* Turn LEDs Off                     */

}

/*----------------------------------------------------------------------------
  Systick interrupt handler (executes every 1ms)
 *----------------------------------------------------------------------------*/
void SysTick_Handler (void) {

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
      PIOB->PIO_CODR = PIO_PB0;                /* Turn LED#1 On               */
    }
    if (current.time.msec == 100) {
      PIOB->PIO_SODR = PIO_PB0;                /* Turn LED#1 Off              */
    }
  
    if (measurement_interval || mdisplay)  {   /* process measurement         */
      current.gpioa = PIOA->PIO_PDSR &         /* PB-LEFT, PB-RIGHT status    */
                      (PIO_PA19 | PIO_PA18);

      if (ADC12B->ADC_SR & ADC_CHSR_CH3) {
        current.analog[0] = ADC12B->ADC_CDR[3] & 0x3FF;  /* AD value (10 bit) */
        ADC12B->ADC_CR    = ADC_CR_START;       /* Start conversion           */ 
      }
      if (ADC->ADC_SR    & ADC_CHSR_CH0) {
        current.analog[1] = ADC->ADC_CDR[0]    & 0x3FF;  /* AD value (10 bit) */
        ADC->ADC_CR       = ADC_CR_START;       /* Start conversion           */ 
      }

      mdisplay = 0;                            /* mdisplay = 0 for ready sig. */
    }

}


/*----------------------------------------------------------------------------
  Calculate first Read Index
 *----------------------------------------------------------------------------*/
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


/*----------------------------------------------------------------------------
  Clear Measurement Records
 *----------------------------------------------------------------------------*/
static void clear_records (void) {
  int idx;                                     /* index for loop              */

  startflag = 0;                               /* stop measurement collecting */
  sindex = savefirst = 0;                      /* reset circular buffer index */
  for (idx = 0; idx != SCNT; idx++)  {         /* mark all records unused     */
    save_record[idx].time.hour = 0xff;         /* unused flag: hour = 0xff    */
  }
}


/*----------------------------------------------------------------------------
  MAIN PROGRAM
 *----------------------------------------------------------------------------*/
int main (void)  {                             /* main entry for program      */
  char cmdbuf [15];                            /* command input buffer        */
  int i;                                       /* index for command buffer    */
  int idx;                                     /* index for circular buffer   */

  /*--------------------------------------------------------------------------*/
  SystemInit();                                /* System setup                */
  SysTick_Config(SystemFrequency/1000);        /* Generate interrupt every 1ms*/

  SER_init();                                  /* initialise serial interface */
  io_Init ();                                  /* initialize LEDs, Buttons    */
  adc_Init();                                  /* initialise A/D converter    */

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
          if (SER_charReady()) {               /* check serial interface      */
            if (SER_getChar() == 0x1B) break;  /* for escape character        */
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
        do  {
          while (!(SER_charReady())) {         /* check serial interface      */
            mdisplay = 1;                      /* request measurement         */
            while (mdisplay);                  /* wait for measurement        */
            measure_display (current);         /* display values              */
          }
        } while (SER_getChar() != 0x1B);       /* escape terminates command   */
        printf ("\n\n");
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

