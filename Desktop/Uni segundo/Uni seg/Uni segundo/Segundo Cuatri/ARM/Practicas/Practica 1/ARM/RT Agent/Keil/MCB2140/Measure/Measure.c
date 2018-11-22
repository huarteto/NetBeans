/******************************************************************************/
/*  This file is part of the uVision/ARM development tools                    */
/*  Copyright KEIL ELEKTRONIK GmbH 2002-2004                                  */
/******************************************************************************/
/*                                                                            */
/*  MEASURE.C:  Remote Measurement Recorder                                   */
/*                                                                            */
/******************************************************************************/


const char menu[] =
   "\n"
   "+***************** REMOTE MEASUREMENT RECORDER *****************+\n"
   "| This program is a simple Measurement Recorder. It is based on |\n"
   "| the LPC21XX and records the state of Port 0 and the voltage   |\n"
   "| on the four analog inputs AD0 trough AD3.                     |\n"
   "+ command -+ syntax -----+ function ----------------------------+\n"
   "| Read     | R [n]       | read <n> recorded measurements       |\n"
   "| Display  | D           | display current measurement values   |\n"
   "| Time     | T hh:mm:ss  | set time                             |\n"
   "| Interval | I mm:ss.ttt | set interval time                    |\n"
   "| Clear    | C           | clear measurement records            |\n"
   "| Quit     | Q           | quit measurement recording           |\n"
   "| Start    | S           | start measurement recording          |\n"
   "+----------+-------------+--------------------------------------+\n";

#include <stdio.h>                          /* standard I/O .h-file           */
#include <ctype.h>                          /* character functions            */

#ifndef MCB2130
  #include <LPC21xx.H>                      /* LPC21xx definitions            */
#else
  #include <LPC213x.H>                      /* LPC213x definitions            */
  #define  ADCR    AD0CR
  #define  ADDR    AD0DR
#endif

#include "timer.h"
#include "RT_agent.h"                       /* Real-Time Agent definitions    */

#include "measure.h"                        /* global project definition file */

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


extern void init_serial (void);

/******************************************************************************/
/*               Save current measurements in save_record                     */
/******************************************************************************/
void save_current_measurements (void) {
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
void tc0 (void) __irq {
  volatile unsigned int val;
  unsigned int crval;
  int i;

  T0IR = 1;                                    /* clear interrupt flag        */

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
    IOSET1 = 0x00010000;                              /* Turn LED On        */
  }
  if (current.time.msec == 100) {
    IOCLR1 = 0x00010000;                              /* Turn LED Off       */
  }

  if (measurement_interval || mdisplay)  {     /* process measurement         */
    current.port0 = IOPIN0;                    /* read port 0                 */
    for (i = 0; i != 4; i++) {                 /* loop for 4 A/D inputs       */
      crval = 0x01000000 | (1<<i);
      ADCR |= crval;                           /* Start A/D Conversion        */
      do {
        val = ADDR;                            /* Read A/D Data Register      */
      } while ((val & 0x80000000) == 0);       /* Wait for end of A/D Convers.*/
      ADCR &= ~crval;                          /* Stop A/D Conversion         */
      val = (val >> 6) & 0x03FF;               /* Extract AIN Value           */
      current.analog[i] = val;                 /* result of A/D process       */ 
    }
    mdisplay = 0;                              /* mdisplay = 0 for ready sig. */
  }

  VICVectAddr = 0;                                    /* acknowledge interrupt       */
}


/******************************************************************************/
/*                       Calculate first Read Index                           */
/******************************************************************************/
int read_index (char *buffer) {
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
void clear_records (void) {
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
  unsigned long volatile start;                /* volatile avoids loop opt.   */

  /* wait for debugger connection before starting code (about 0.3 sec)        */
#ifndef SIMULATOR
  for (start = 0; start < 2000000; start++) {;}
#endif

  /* Reset LEDs */
  IODIR1 = 0x00FF0000;                         /* P1.16..23 defined as Outputs*/
  IOCLR1 = 0x00FF0000;                         /* Clear all LEDs on P1.16..23 */

  /* setup A/D converter */
  PINSEL1 = 0x15400000;                        /* Select AIN0..AIN3           */
  ADCR    = 0x002E0401;                        /* Setup A/D: 10-bit @ 3MHz    */

  RTA_Init();                                  /* Initialize Real-Time Agent  */
  init_serial();                               /* Initialize com port 0       */

  /* setup the timer counter 0 interrupt */
  T0MR0 = 60000;                               /* 1mSec = 60000 counts @ 60MHz*/
  T0MCR = 3;                                   /* Interrupt and Reset on MR0  */
  T0TCR = 1;                                   /* Timer0 Enable               */
  VICVectAddr0 = (unsigned long)tc0;           /* set interrupt vector in 0   */
  VICVectCntl0 = 0x20 | 4;                     /* use it for Timer 0 IRQ      */
  VICIntEnable = 0x00000010;                   /* Enable Timer0 IRQ           */

  clear_records ();                            /* initialize circular buffer  */
  //printf ( menu );                             /* display command menu        */
  RTA_printf(menu);

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
          if (RTA_TermIsKey())  {              /* check serial interface      */
            if (getchar() == 0x1B) break;      /* for escape character        */
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
          while (!RTA_TermIsKey())  {          /* check serial interface      */
            mdisplay = 1;                      /* request measurement         */
            while (mdisplay);                  /* wait for measurement        */
            measure_display (current);         /* display values              */
          }
        } while (getchar () != 0x1B);          /* escape terminates command   */
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
