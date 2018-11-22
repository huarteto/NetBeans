/*----------------------------------------------------------------------------
 *      Name:    MEASURE.C
 *      Purpose: Remote Measurement Recorder
 *      Version: V1.10
 *----------------------------------------------------------------------------
 *      This file is part of the uVision/ARM development tools.
 *      This software may only be used under the terms of a valid, current,
 *      end user licence from KEIL for a compatible version of KEIL software
 *      development tools. Nothing else gives you the right to use it.
 *
 *      Copyright (c) 2005-2007 Keil Software.
 *---------------------------------------------------------------------------*/

#include <stdio.h>                       /* standard I/O .h-file              */
#include <ctype.h>                       /* character functions               */
#include <91x_lib.h> 					 /* STR91x definitions                */

#include "measure.h"                     /* global project definition file    */
#include "LCD.h"						 /* LCD definitions                   */

const char menu[] =
   "\n"
   "+***************** REMOTE MEASUREMENT RECORDER *****************+\n"
   "| This program is a simple Measurement Recorder. It is based on |\n"
   "| the STR912FW44 and records the state of the Buttons S2,S3 and |\n"
   "| the voltage on the four analog inputs AD0 trough AD3.         |\n"
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
/*          Default Interrupt Service Routine (necessary for STR91x)          */
/******************************************************************************/
__irq void defInterrupt (void) {
  ;                                            /* Do nothing                  */ 
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
    GPIO7->DR[0xFF*4] = 0x01;                  /* Turn LED#1 On               */
  }
  if (current.time.msec == 100) {
    GPIO7->DR[0xFF*4] = 0x00;                  /* Turn LED#1 Off              */
  }
  
  if (measurement_interval || mdisplay)  {     /* process measurement         */
    current.gpio = GPIO3->DR[0x60*4];          /* S2, S3 status               */

    current.analog[0] = ADC->DR0 & 0x03FF;	   /* AD value (10 bit)           */
    current.analog[1] = ADC->DR1 & 0x03FF;	   /* AD value (10 bit)           */
    current.analog[2] = ADC->DR2 & 0x03FF;	   /* AD value (10 bit)           */
    current.analog[3] = ADC->DR3 & 0x03FF;	   /* AD value (10 bit)           */

    mdisplay = 0;                              /* mdisplay = 0 for ready sig. */
  }

  TIM3->SR   &= ~TIM_FLAG_OC2;                 /* Clear Timer Overflow flag   */
  TIM3->CNTR  = 0x0000;                        /* Setup TIM3 counter register */

  VIC0->VAR = 0;                               /* Acknowledge Interrupt       */  
  VIC1->VAR = 0;
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
/***************************      MAIN PROGRAM      ***************************/
/******************************************************************************/
int main (void)  {                             /* main entry for program      */
  char cmdbuf [15];                            /* command input buffer        */
  int i;                                       /* index for command buffer    */
  int idx;                                     /* index for circular buffer   */

  /* General Purpose I/O Port 7 Setup (LED) */
  SCU->GPIOOUT[7] = 0x5555;               /* P7.0..7 output - mode 1          */
  GPIO7->DDR      = 0xFF;                 /* P7.0..7 Outputs (LED Data)       */
  GPIO7->DR[0x3FC]= 0x00;                 /* clear LEDs                       */

  /* General Purpose I/O Port 8,9 Setup (LCD) */
  GPIO8->DDR      = 0xFF;                 /* P8.0..7 Outputs (LCD Data)       */
  GPIO9->DDR      = 0x07;                 /* P9.0..2 Outputs (LCD Control)    */

  /* General Purpose I/O Port 3 Setup (Push Button) */
  /* portA = S2 (P3.5 (INT5)), Port B = S3 (P3.6 (INT6)) */				    
  SCU->GPIOOUT[3] &= 0xC3FF;              /* P3.5, P3.6 input mode            */
  GPIO3->DDR      &= 0x9F;                /* P3.5, P3.6 Input Push Buttons)   */

  /* General Purpose I/O Port 4 Setup (Analog Inputs) */
  SCU->GPIOIN[4]  |= 0x0F;                /* P4.0-P4.3 input  - mode 0        */
  SCU->GPIOOUT[4] &= 0xFF00;              /* P4.0-P4.3 output - mode 0        */
  GPIO4->DDR      &= 0xF0;                /* P4.0-P4.3 direction - input      */
  SCU->GPIOANA    |= 0x000F;              /* P4.0-P4.3 analog mode ON         */

  /* ADC Setup */
  ADC->CR         |= 0x0002;              /* Set POR bit                      */
  for (i = 0; i < 48000; i ++);           /* Wait > 1 ms  (at 48 MHz)         */

  ADC->CR         &= 0xFFF7;              /* Clear STB bit                    */
  for (i = 0; i < (48*15); i ++);         /* Wait > 15 us (at 48 MHz)         */

  ADC->CR  |= 0x0033;                     /* cont. conversion all channels    */
  ADC->CCR |= 0x00FF;                     /* ADC, no WDG, Channel 0..4        */

  /* setup the default interrupt */
  VIC0->DVAR = (unsigned int)defInterrupt;    
  VIC1->DVAR = (unsigned int)defInterrupt;    

  /* setup the timer counter 3 interrupt, Vector Slot 7 */
  VIC0->VAiR[7]  = (unsigned int)tc3;     /* Setup TIM3 IRQ Hndl addr         */
  VIC0->VCiR[7]  = 0x20 | 7;              /* enable vector slot  7            */
  VIC0->INTER   |= (1<<7);                /* Enable TIM3 interrupt            */

  /* Timer 3 Configuration (TIM3) */
  #define _CLOCK       48000000
  #define _TICK            1000           /* 1000  (1ms)                      */

  TIM3->OC2R = (_CLOCK / _TICK / 100) - 5; /* -5 because cnt starts at 0xFFFC */
  TIM3->CR2  =  TIM_FLAG_OC2 | (100 - 1);      /* prescale 100                */
  TIM3->CR1  =  0x8000;                        /* Enable timer                */
                       
  /*--------------------------------------------------------------------------*/
  init_serial ();                              /* initialite serial interface */

  lcd_init();
  lcd_clear();
  lcd_print ("MCBSTR9 Measure ");
  set_cursor (0, 1);
  lcd_print ("  www.keil.com  ");

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
          if (UART0->FR & 0x0040)  {           /* check serial interface      */
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
          while (!(UART0->FR & 0x0040))  {     /* check serial interface      */
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
        printf (ERROR_STR, "UNKNOWN COMMAND");
        printf (menu);                         /* display command menu        */
        break;
    }
  }
}

