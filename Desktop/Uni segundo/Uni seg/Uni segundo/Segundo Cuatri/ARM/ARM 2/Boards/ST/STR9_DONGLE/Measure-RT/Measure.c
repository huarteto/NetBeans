/******************************************************************************/
/* MEASURE.C: Remote Measurement Recorder                                     */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

//#include <stdio.h>                       /* standard I/O .h-file              */
#include <ctype.h>                       /* character functions               */
#include <91x_lib.h> 					 /* STR91x definitions                */

#include "measure.h"                     /* global project definition file    */
#include "RT_Agent.h"

const char menu[] =
   "\n"
   "+***************** REMOTE MEASUREMENT RECORDER *****************+\n"
   "| This program is a simple Measurement Recorder. It is based on |\n"
   "| the STR912FW44 and records the state of the Buttons B85..B88  |\n"
   "+ command -+ syntax -----+ function ----------------------------+\n"
   "| Read     | R [n]       | read <n> recorded measurements       |\n"
   "| Display  | D           | display current measurement values   |\n"
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
int ledNr = 0;

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
/*                Timer Counter 3 interrupt service function                  */
/*                executes each 1ms @ 25 MHz Crystal Clock                    */
/******************************************************************************/
__irq void tc3 (void) {

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
    GPIO6->DR[(0x01<<ledNr)*4] = 0x00;         /* Turn LED On                 */
  }
  if (current.time.msec == 100) {
    GPIO6->DR[(0x01<<ledNr)*4] = 0x0F;         /* Turn LED Off                */
	ledNr = (ledNr+1) & 0x03;
  }
  
  if (measurement_interval || mdisplay)  {     /* process measurement         */
    current.gpio = GPIO7->DR[0x0F*4];          /* B85..B87 status             */

    mdisplay = 0;                              /* mdisplay = 0 for ready sig. */
  }

  TIM3->SR   &= ~TIM_FLAG_OC2;         /* Clear Timer Overflow interrupt flag */
  TIM3->CNTR  = 0x0000;                        /* Setup TIM3 counter register */

  VIC0->VAR = 0;                               /* Acknowledge Interrupt       */  
  VIC1->VAR = 0;
}


/******************************************************************************/
/*                       Calculate first Read Index                           */
/******************************************************************************/
static int read_index (void) {
  int index;

  index = SCNT-1;
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

  /* General Purpose I/O Port 6 Setup (LED)*/
  SCU->GPIOOUT[6]   = 0x0055;               /* P6.0..3 output - mode 1        */
  GPIO6->DDR        = 0x0F;                 /* P6.0..3 Outputs (LED Data)     */
  GPIO6->DR[0x0F*4] = 0x0F;                 /* clear LEDs                     */

  /* General Purpose I/O Port 7 Setup (Push Button)*/
  SCU->GPIOOUT[7]   = 0x0000;               /* P7.0..7 no output              */
  GPIO7->DDR        = 0x00;                 /* P7.0..7 Input (Push Buttons)   */

  /* setup the timer counter 3 interrupt */
  VIC0->VAiR[7]   = (unsigned int)tc3;    /* Setup TIM3 IRQ Hndl addr         */
  VIC0->VCiR[7]  |= 0x20;                 /* Enable the vector interrupt      */
  VIC0->VCiR[7]  |= 7;                    /* Specify the interrupt number     */
  VIC0->INTER    |= (1<<7);               /* Enable TIM3 interrupt            */

  /* Timer 3 Configuration (TIM3) */
  #define _CLOCK       48000000
  #define _TICK            1000           /* 1000  (1ms)					  */

  TIM3->OC2R = (_CLOCK / _TICK / 100) - 5; /* -5 because cnt starts at 0xFFFC */
  TIM3->CR2  =  TIM_FLAG_OC2 | (100 - 1);      /* prescale 100                */
  TIM3->CR1  =  0x8000;                        /* Enable timer                */
                       
  /*--------------------------------------------------------------------------*/
  RTA_Init ();                                 /* Initialize Real-Time Agent  */
//  init_serial ();                              /* initialite serial interface */

  clear_records ();                            /* initialize circular buffer  */
  prints ( menu );                               /* display command menu        */

  while (1)  {                                 /* loop forever                */
    prints ("\nCommand: ");
    getline (&cmdbuf[0], sizeof (cmdbuf));     /* input command line          */

    for (i = 0; cmdbuf[i] != 0; i++)  {        /* convert to upper characters */
      cmdbuf[i] = toupper(cmdbuf[i]);
    }

    for (i = 0; cmdbuf[i] == ' '; i++);        /* skip blanks                 */

    switch (cmdbuf[i])  {                      /* proceed to command function */

      case 'R':                                /* Read circular Buffer        */
        idx = read_index ();
        while (idx != sindex)  {               /* check end of table          */
          if (RTA_TermIsKey())  {              /* check serial interface      */
            if (RTA_TermGetChar() == 0x1B) break;      /* for escape character        */
          }
          if (save_record[idx].time.hour != 0xff)  {
            measure_display (save_record[idx]);      /* display record        */
            prints ("\n");
          }
          if (++idx == SCNT) idx = 0;          /* next circular buffer entry  */
        }
        break;

       case 'D':                                /* Display Command             */
        prints ("\nDisplay current Measurements: (ESC to abort)\n");
        do  {
          while (!RTA_TermIsKey())  {          /* check serial interface      */
            mdisplay = 1;                      /* request measurement         */
            while (mdisplay);                  /* wait for measurement        */
            measure_display (current);         /* display values              */
          }
        } while (RTA_TermGetChar () != 0x1B);          /* escape terminates command   */
        prints ("\n\n");
        break;

      case 'S':                                /* Start Command               */
        prints ("\nStart Measurement Recording\n");
        startflag = 1;
        break;

      case 'Q':                                /* Quit Command                */
        prints ("\nQuit Measurement Recording\n");
        startflag = 0;
        break;

      case 'C':                                /* Clear Command               */
        prints ("\nClear Measurement Records\n");
        clear_records ();
        break;

      default:                                 /* Error Handling              */
        prints ("\n*** ERROR: UNKNOWN COMMAND");
        prints (menu);                         /* display command menu        */
        break;
    }
  }
}
