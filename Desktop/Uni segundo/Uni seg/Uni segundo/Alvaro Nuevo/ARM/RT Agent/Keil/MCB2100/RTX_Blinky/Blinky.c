/*----------------------------------------------------------------------------
 *      R T L   K e r n e l   E x a m p l e
 *----------------------------------------------------------------------------
 *      Name:    BLINKY.C
 *      Purpose: RTX example program
 *      Rev.:    V3.40
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2008 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include <RTL.h>
#include <LPC21xx.H>                    /* LPC21xx definitions               */
#include <stdio.h>
#include <RT_Agent.h>

OS_TID t_phaseA;                        /* assigned task id of task: phase_a */
OS_TID t_phaseB;                        /* assigned task id of task: phase_b */
OS_TID t_phaseC;                        /* assigned task id of task: phase_c */
OS_TID t_phaseD;                        /* assigned task id of task: phase_d */
OS_TID t_clock;                         /* assigned task id of task: clock   */


/* The LEDs of MCB2103 and MCB2100/2130 are connected to different ports     */
#define IOSET      IOSET1
#define IOCLR      IOCLR1
#define IODIR      IODIR1
#define LED_A      0x010000
#define LED_B      0x020000
#define LED_C      0x040000
#define LED_D      0x080000
#define LED_CLK    0x800000

typedef struct _tag_state_info_t {
  U32 clk;
  U8  phase;
} state_info_t;

state_info_t stateInfo;

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
    stateInfo.phase = 0x0A;
    RTA_printf("A");
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    IOSET = LED_A;
    signal_func (t_phaseB);              /* call common signal function      */
    IOCLR = LED_A;    
  }
}

/*----------------------------------------------------------------------------
 *        Task 2 'phaseB': Phase B output
 *---------------------------------------------------------------------------*/
__task void phaseB (void) {
  for (;;) {
    stateInfo.phase = 0x0B;
    RTA_printf("B");
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    IOSET = LED_B;
    signal_func (t_phaseC);              /* call common signal function      */
    IOCLR = LED_B;    
  }
}

/*----------------------------------------------------------------------------
 *        Task 3 'phaseC': Phase C output
 *---------------------------------------------------------------------------*/
__task void phaseC (void) {
  for (;;) {
    stateInfo.phase = 0x0C;
    RTA_printf("C");
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    IOSET = LED_C;
    signal_func (t_phaseD);              /* call common signal function      */
    IOCLR = LED_C;    
  }
}

/*----------------------------------------------------------------------------
 *        Task 4 'phaseD': Phase D output
 *---------------------------------------------------------------------------*/
__task void phaseD (void) {
  for (;;) {
    stateInfo.phase = 0x0D;
    RTA_printf("D\n");
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    IOSET = LED_D;
    signal_func (t_phaseA);              /* call common signal function      */
    IOCLR = LED_D;    
  }
}

/*----------------------------------------------------------------------------
 *        Task 5 'clock': Signal Clock
 *---------------------------------------------------------------------------*/
__task void clock (void) {
  for (;;) { 
    stateInfo.clk++;
    RTA_printf(".");
    os_evt_wait_and (0x0100, 0xffff);    /* wait for an event flag 0x0100    */
    IOSET = LED_CLK;
    os_dly_wait (8);                     /* delay 8 clock ticks              */
    IOCLR = LED_CLK;    
  }
}

/*----------------------------------------------------------------------------
 *        Task 6 'init': Initialize
 *---------------------------------------------------------------------------*/
__task void init (void) {

  IODIR = 0xFF0000;                      /* P1.16..23 defined as Outputs     */

  t_phaseA = os_tsk_create (phaseA, 0);  /* start task phaseA                */
  t_phaseB = os_tsk_create (phaseB, 0);  /* start task phaseB                */
  t_phaseC = os_tsk_create (phaseC, 0);  /* start task phaseC                */
  t_phaseD = os_tsk_create (phaseD, 0);  /* start task phaseD                */
  t_clock  = os_tsk_create (clock, 0);   /* start task clock                 */
  os_evt_set (0x0001, t_phaseA);         /* send signal event to task phaseA */
  os_tsk_delete_self ();
}

/*----------------------------------------------------------------------------
 *        Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/
int main (void) {
  U32 volatile start;

#ifndef SIMULATOR
                                         /* Wait for debugger connection 0.3s*/
  for (start = 0; start < 1000000; start++) { ; }
#endif

  os_sys_init (init);                    /* Initialize RTX and start init    */
}

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/

