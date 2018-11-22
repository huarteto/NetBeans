/******************************************************************************/
/*  This file is part of the uVision/ARM development tools                    */
/*  Copyright KEIL ELEKTRONIK GmbH 2002-2004                                  */
/******************************************************************************/
/*                                                                            */
/*  MCOMMAND.C:  Time Set Commands for the Remote Measurement Recorder        */
/*                                                                            */
/******************************************************************************/


#include <stdio.h>                       /* standard ANSI I/O .h-file         */
#include <LPC21xx.H>                     /* LPC21xx definitions               */

#include "measure.h"                     /* global project definition file    */


/******************************************************************************/
/*                           Display measurements                             */
/******************************************************************************/
void measure_display (struct mrec display) {    
  int i;                                 /* index count for AIN0 - AIN3       */

  printf ("\rTime: %2d:%02d:%02d.%03d  P0:%08X",
           display.time.hour,            /* display hour                      */
           display.time.min,             /* display minute                    */
           display.time.sec,             /* display second                    */
           display.time.msec,            /* display millisecond               */
           (int)display.port0);          /* display port 0 value              */
                                         
  for (i = 0; i < 4; i++) {              /* display AN0 through AN3           */
    printf (" AIN%d:%4.2fV", i, (float)display.analog[i] * 3.3 / 1024);
  }
}


/******************************************************************************/
/*                           Set Current Time                                 */
/******************************************************************************/
void set_time (char * buffer)  {
  int hour, min, sec;                    /* temporary time values             */
  int args;                              /* number of arguments               */

  args = sscanf (buffer, "%d:%d:%d",     /* scan input line for               */
                 &hour,                  /* hour, minute and second           */
                 &min,
                 &sec);
  if (hour > 23  ||  min > 59  ||        /* check for valid inputs            */
      sec > 59   ||  args < 2  ||  args == EOF)  {
    printf (ERROR, "INVALID TIME FORMAT");
  }
  else  {                                /* if inputs valid then              */
    VICIntEnClr = 0x00000010;            /* disable Timer0 IRQ while          */
    current.time.hour = hour;            /* setting the new time: hours       */
    current.time.min  = min;             /* setting the new time: minutes     */
    current.time.sec  = sec;             /* setting the new time: seconds     */
    current.time.msec = 0;               /* setting the new time: miliseconds */
    interval.min = 0;                    /* force new interval                */
    interval.sec = 0;
    interval.msec = 0;
    VICIntEnable = 0x00000010;           /* enable interrupts again           */
  }
}


/******************************************************************************/
/*                            Set Interval Time                               */
/******************************************************************************/
void set_interval (char * buffer) {
  struct interval itime;                 /* temporary interval record         */
  int min, sec, msec;
  int args;                              /* number of arguments               */

  args = sscanf (buffer, "%d:%d.%d",     /* scan input line for               */
                 &min,                   /* minute, second and milliseconds   */
                 &sec,
                 &msec);
                                         /* check valid inputs                */
  if (sec > 59  ||  msec > 999  ||  args < 2  || args == EOF)  {
    printf (ERROR, "INVALID INTERVAL FORMAT");
  }
  else  {                                /* if inputs are valid then          */
    itime.min  = min;
    itime.sec  = sec;
    itime.msec = msec;
    if (itime.min != 0 || itime.sec != 0 || itime.msec != 0)  {
      if (itime.msec-- == 0)  {          /* correct interval time for         */
        itime.msec = 999;                /* interrupt routine                 */
        if (itime.sec-- == 0)  {
          itime.sec = 59;
          itime.min--;
        }
      }
    }
    VICIntEnClr = 0x00000010;            /* disable Timer0 IRQ while          */
    setinterval = itime;                 /* of the new setinterval time       */
    interval.min = 0;                    /* force new interval                */
    interval.sec = 0;
    interval.msec = 0;
    VICIntEnable = 0x00000010;           /* enable interrupts again           */
  }
}
