/*----------------------------------------------------------------------------
 *      RL-ARM - RTX
 *----------------------------------------------------------------------------
 *      Name:    BLINKY.C
 *      Purpose: RTX example program
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2010 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include <RTL.h>
#include <stdio.h>
#include <TMS570.H>
#include "GLCD.h"

extern void EMIF_MPU_init(void);

OS_TID t_phaseA;                        /* assigned task id of task: phase_a */
OS_TID t_phaseB;                        /* assigned task id of task: phase_b */
OS_TID t_phaseC;                        /* assigned task id of task: phase_c */
OS_TID t_phaseD;                        /* assigned task id of task: phase_d */
OS_TID t_clock;                         /* assigned task id of task: clock   */
OS_TID t_lcd;                           /* assigned task id of task: lcd     */

#define LED_A      0
#define LED_B      1
#define LED_C      2
#define LED_D      3
#define LED_CLK    5

const U32 LED_mask[6] = {1u<<0, 1u<<3, 1u<<4, 1u<<5, 1u<<1, 1u<<31};

static void LED_on (U32 led) {
  if (led < 4) {
    gioPORTA->DSET = LED_mask[led];
  }
  else {
    nhetREG->CCDSET= LED_mask[led];
  }
}

static void LED_off (U32 led) {
  if (led < 4) {
    gioPORTA->DCLR = LED_mask[led];
  }
  else {
    nhetREG->CCDCLR= LED_mask[led];
  }
}

/*----------------------------------------------------------------------------
 *        Function 'signal_func' called from multiple tasks
 *---------------------------------------------------------------------------*/
void signal_func (OS_TID task)  {
  os_evt_set (0x0100, t_clock);          /* send event signal to clock task  */
  os_dly_wait (50);                      /* delay 50 clock ticks             */
  os_evt_set (0x0100, t_clock);          /* send event signal to clock task  */
  os_dly_wait (50);                      /* delay 50 clock ticks             */
  os_evt_set (0x0001, task);             /* send event to task 'task'        */
  os_dly_wait (50);                      /* delay 50 clock ticks             */
}

/*----------------------------------------------------------------------------
 *        Task 1 'phaseA': Phase A output
 *---------------------------------------------------------------------------*/
__task void phaseA (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    LED_on (LED_A);
    signal_func (t_phaseB);              /* call common signal function      */
    LED_off (LED_A);
  }
}

/*----------------------------------------------------------------------------
 *        Task 2 'phaseB': Phase B output
 *---------------------------------------------------------------------------*/
__task void phaseB (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    LED_on (LED_B);
    signal_func (t_phaseC);              /* call common signal function      */
    LED_off (LED_B);
  }
}

/*----------------------------------------------------------------------------
 *        Task 3 'phaseC': Phase C output
 *---------------------------------------------------------------------------*/
__task void phaseC (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    LED_on (LED_C);
    signal_func (t_phaseD);              /* call common signal function      */
    LED_off (LED_C);
  }
}

/*----------------------------------------------------------------------------
 *        Task 4 'phaseD': Phase D output
 *---------------------------------------------------------------------------*/
__task void phaseD (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    LED_on (LED_D);
    signal_func (t_phaseA);              /* call common signal function      */
    LED_off (LED_D);
  }
}

/*----------------------------------------------------------------------------
 *        Task 5 'clock': Signal Clock
 *---------------------------------------------------------------------------*/
__task void clock (void) {
  for (;;) {
    os_evt_wait_and (0x0100, 0xffff);    /* wait for an event flag 0x0100    */
    LED_on (LED_CLK);
    os_dly_wait (8);                     /* delay 8 clock ticks              */
    LED_off (LED_CLK);
  }
}

/*----------------------------------------------------------------------------
 *        Task 6 'lcd': LCD Control task
 *---------------------------------------------------------------------------*/

__task void lcd (void) {
  U32 i;
  char *tp;
                                         /* !! MUST be in priviliged mode !! */
  EMIF_MPU_init ();                      /* EMIF MPU Initialization          */
  GLCD_Init ();                          /* GLCD Initialization              */

  GLCD_Clear (White);                    /* Clear graphical LCD display      */
  GLCD_SetBackColor (Blue);
  GLCD_SetTextColor (White);
  GLCD_DisplayString (0, 0, "       RL-ARM       ");
  GLCD_DisplayString (1, 0, "   Kernel example   ");
  GLCD_DisplayString (2, 0, "    www.keil.com    ");
  GLCD_SetBackColor (White);
  GLCD_SetTextColor(Blue);
  os_dly_wait (200);

  for (;;) {
    tp = "KEIL";
    for (i = 0; i < 4; i++) {
      GLCD_DisplayChar (4, i+8, *tp++);
      os_dly_wait (20);
    }
    tp = "An ARM Company";
    for (i = 0; i < 14; i++) {
      GLCD_DisplayChar (5, i+3, *tp++);
      os_dly_wait (20);
    }
    os_dly_wait (200);
    GLCD_ClearLn (4);
    GLCD_ClearLn (5);
    os_dly_wait (100);
  }
}

/*----------------------------------------------------------------------------
 *        Task 7 'init': Initialize
 *---------------------------------------------------------------------------*/
__task void init (void) {

  gioREG->GCR0    = 1;
  gioPORTA->DIR  |= 0x00000039;          /* GIO LEDs defined as outputs      */
  gioPORTA->DCLR  = 0x00000039;
  nhetREG->CCDIR |= 0x80000002;          /* HET LEDs defined as outputs      */
  nhetREG->CCDCLR = 0x80000002;

  t_phaseA = os_tsk_create (phaseA, 0);  /* start task phaseA                */
  t_phaseB = os_tsk_create (phaseB, 0);  /* start task phaseB                */
  t_phaseC = os_tsk_create (phaseC, 0);  /* start task phaseC                */
  t_phaseD = os_tsk_create (phaseD, 0);  /* start task phaseD                */
  t_clock  = os_tsk_create (clock, 0);   /* start task clock                 */
  t_lcd    = os_tsk_create (lcd, 0);     /* start task lcd                   */
  os_evt_set (0x0001, t_phaseA);         /* send signal event to task phaseA */
  os_tsk_delete_self ();
}

/*----------------------------------------------------------------------------
 *        Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/
int main (void) {
  os_sys_init (init);                    /* Initialize RTX and start init    */
}

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/

