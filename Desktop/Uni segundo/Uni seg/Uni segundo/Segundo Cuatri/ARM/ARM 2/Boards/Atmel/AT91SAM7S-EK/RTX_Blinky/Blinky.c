/*----------------------------------------------------------------------------
 *      RL-ARM - RTX
 *----------------------------------------------------------------------------
 *      Name:    BLINKY.C
 *      Purpose: RTX example program
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2009 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include <RTL.h>
#include <AT91SAM7S64.H>                /* AT91SAM7S64 definitions */
#include "..\Board.h"


OS_TID t_phaseA;                        /* assigned task id of task: phase_a */
OS_TID t_phaseB;                        /* assigned task id of task: phase_b */
OS_TID t_phaseC;                        /* assigned task id of task: phase_c */
OS_TID t_phaseD;                        /* assigned task id of task: phase_d */


/*----------------------------------------------------------------------------
 *        Function 'signal_func' called from multiple tasks
 *---------------------------------------------------------------------------*/
void signal_func (OS_TID task)  {
  os_dly_wait (100);                     /* delay 100 clock ticks            */
  os_evt_set (0x0001, task);             /* send event to task 'task'        */
  os_dly_wait (50);                      /* delay 50 clock ticks             */
}

/*----------------------------------------------------------------------------
 *        Task 1 'phaseA': Phase A output
 *---------------------------------------------------------------------------*/
__task void phaseA (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    *AT91C_PIOA_CODR = LED1;
    signal_func (t_phaseB);              /* call common signal function      */
    *AT91C_PIOA_SODR = LED1;
  }
}

/*----------------------------------------------------------------------------
 *        Task 2 'phaseB': Phase B output
 *---------------------------------------------------------------------------*/
__task void phaseB (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    *AT91C_PIOA_CODR = LED2;
    signal_func (t_phaseC);              /* call common signal function      */
    *AT91C_PIOA_SODR = LED2;
  }
}

/*----------------------------------------------------------------------------
 *        Task 3 'phaseC': Phase C output
 *---------------------------------------------------------------------------*/
__task void phaseC (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    *AT91C_PIOA_CODR = LED3;
    signal_func (t_phaseD);              /* call common signal function      */
    *AT91C_PIOA_SODR = LED3;
  }
}

/*----------------------------------------------------------------------------
 *        Task 4 'phaseD': Phase D output
 *---------------------------------------------------------------------------*/
__task void phaseD (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    *AT91C_PIOA_CODR = LED4;
    signal_func (t_phaseA);              /* call common signal function      */
    *AT91C_PIOA_SODR = LED4;
  }
}

/*----------------------------------------------------------------------------
 *        Task 5 'init': Initialize
 *---------------------------------------------------------------------------*/
__task void init (void) {

  *AT91C_PMC_PCER = (1 << AT91C_ID_PIOA);   /* Enable Clock for PIO    */

  *AT91C_PIOA_PER  = LED_MASK;              /* Enable PIO for LED1..4  */
  *AT91C_PIOA_OER  = LED_MASK;              /* LED1..4 are Outputs     */
  *AT91C_PIOA_SODR = LED_MASK;              /* Turn off LED's ("1")    */

  t_phaseA = os_tsk_create (phaseA, 0);  /* start task phaseA                */
  t_phaseB = os_tsk_create (phaseB, 0);  /* start task phaseB                */
  t_phaseC = os_tsk_create (phaseC, 0);  /* start task phaseC                */
  t_phaseD = os_tsk_create (phaseD, 0);  /* start task phaseD                */
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

