/******************************************************************************/
/* MEASURE.C: Remote Measurement Recorder                                     */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <stdio.h>                       /* standard I/O .h-file              */
#include <ctype.h>                       /* character functions               */
#include <91M55800.H>                    /* AT91M55800A definitions           */

#include "measure.h"                     /* global project definition file    */

const char menu[] =
   "\n"
   "+***************** REMOTE MEASUREMENT RECORDER *****************+\n"
   "| This program is a simple Measurement Recorder. It is based on |\n"
   "| the AT91M55800A and records the state of Port A and Port B    |\n"
   "| and the voltage on the four analog inputs AD0 trough AD3.     |\n"
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

#define SCNT  20                            /* number of records              */

struct mrec save_record[SCNT];              /* buffer for measurements        */
int sindex;                                 /* save index                     */
int savefirst;                              /* save first index               */

char ERROR [] = "\n*** ERROR: %s\n";        /* ERROR message string in code   */

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
/*                Timer Counter 0 interrupt service function                  */
/*                executes each 1ms @ 32 MHz Crystal Clock                    */
/******************************************************************************/
__irq void tc0 (void) {
  int i;

  if (measurement_interval)  {                 /* measurement done ?          */
    save_current_measurements ();              /* yes -> save measurements    */
    measurement_interval = 0;                  /* Save measurements done      */
  }
                                               /* check if interval is over   */
  if (interval.min  == 0 &&
      interval.sec  == 0 &&
      interval.msec == 0     )  {
    interval = setinterval;                    /* set interval time again     */
    measurement_interval = startflag;          /* determine measurement flag  */
  }
  else  {                                      /* update interval time        */
    if (interval.msec-- == 0)  {               /* calculate millisecond       */
      interval.msec = 999;
      if (interval.sec-- == 0)  {              /* calculate second            */
        interval.sec = 59;
        interval.min--;                        /* calculate minute            */
      }
    }
  }
                                               /* update current time         */
  if (++current.time.msec == 1000)  {          /* update millisecond cnt      */
    current.time.msec = 0;

    if (++current.time.sec == 60)  {           /* update second counter       */
      current.time.sec = 0;

      if (++current.time.min == 60)  {         /* update minute counter       */
        current.time.min = 0;

        if (++current.time.hour == 24)  {      /* update hour counter         */
          current.time.hour = 0;
        }
      }
    }
  }	/* end of if( ++current.time.msec... */

  if (current.time.msec == 0) {
    PIOA_CODR = 1;                             /* Turn LED On  (PA0 = 0)      */
  }
  if (current.time.msec == 100) {
    PIOA_SODR = 1;                             /* Turn LED Off (PA0 = 1)      */
  }

  if (measurement_interval || mdisplay)  {     /* process measurement         */
    current.portA = PIOA_PDSR;                 /* read port A                 */
    current.portB = PIOB_PDSR;                 /* read port B                 */
    ADC0_CR = ADC_START;                       /* start ADC0 conversion       */
    while ((ADC0_SR & ADC_EOC0) == 0);         /* wait for ADC0 conversion    */
    for (i = 0; i < 4; i++) {
      current.analog[i] = *(&ADC0_CDR0 + i);   /* result of ADC0 process      */
    }
    mdisplay = 0;                              /* mdisplay = 0 for ready sig. */
  }

  AIC_EOICR = TC0_SR;                          /* end interrupt               */
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
    printf (ERROR, "QUIT MEASUREMENTS BEFORE READ");
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
/***************************      MAIN PROGRAM      ***************************/
/******************************************************************************/
int main (void)  {                             /* main entry for program      */
  char cmdbuf [15];                            /* command input buffer        */
  int i;                                       /* index for command buffer    */
  int idx;                                     /* index for circular buffer   */

  /* setup APMC */
  APMC_PCER = (1<<PIOA_ID) | (1<<PIOB_ID) |    /* enable PIOA, PIOB,          */
              (1<<ADC0_ID) | (1<<US0_ID)  |    /*        ADC0, US0,           */
              (1<<TC0_ID);                     /*        TC0                  */

  PIOA_OER = 1;                                /* Set PA0 as Output           */

  init_serial ();                              /* initialite serial interface */

  /* setup A/D converter */
  ADC0_CR = ADC_SWRST;                         /* reset ADC0                  */
  ADC0_CHER = ADC_CH0 | ADC_CH1 | ADC_CH2 | ADC_CH3;  /* enable AD0..3        */
  ADC0_MR = ADC_8_BIT_RES | (9<<ADC_B_PRESCAL);       /* setup ADC0           */

  /* setup the timer counter 0 interrupt */
  TC0_CMR  = TC_CAPT | TC_CPCTRG;              /* set timer mode              */
  TC0_RC   = 16000 - 1;                        /* set timer period            */
  TC0_IER  = TC_CPCS;                          /* enable RC compare interrupt */
  AIC_SVR6 = (unsigned long)tc0;               /* set interrupt vector        */
  AIC_SMR6 = AIC_SRCTYPE_INT_EDGE_TRIGGERED;   /* edge triggered interrupt    */
  AIC_IECR = (1<<TC0_ID);                      /* enable interrupt            */

  TC0_CCR  = TC_CLKEN | TC_SWTRG;              /* enable and start timer      */

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
          if (US0_CSR & US_RXRDY)  {           /* check serial interface      */
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
        do  {
          while (!(US0_CSR & US_RXRDY))  {     /* check serial interface      */
            mdisplay = 1;                      /* request measurement         */
            while (mdisplay);                  /* wait for measurement        */
            measure_display (current);         /* display values              */
          }
        } while (getkey () != 0x1B);           /* escape terminates command   */
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
        printf (ERROR, "UNKNOWN COMMAND");
        printf (menu);                         /* display command menu        */
        break;
    }
  }
}
