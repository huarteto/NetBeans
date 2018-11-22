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
#include "lm3s_cmsis.h"
#include <system_lm3s.h>

OS_TID t_phaseA;                        /* assigned task id of task: phase_a */
OS_TID t_phaseB;                        /* assigned task id of task: phase_b */
OS_TID t_clock;                         /* assigned task id of task: clock   */

#define LED_A    (1 << 3)
#define LED_PINS (LED_A)

/*--------------------------- LED_Out ---------------------------------------*/

static void LED_Out (U32 led, U32 st) {
  if (st) {
    GPIOB->DATA |=  led;
  } else {
    GPIOB->DATA &= ~led;
  }
}

/*----------------------------------------------------------------------------
 *        Function 'signal_func' called from multiple tasks
 *---------------------------------------------------------------------------*/
void signal_func (OS_TID task)  {
  os_evt_set (0x0100, t_clock);          /* send event signal to clock task  */
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
    LED_Out (LED_A, 1);
    signal_func (t_phaseB);              /* call common signal function      */
    LED_Out (LED_A, 0);
  }
}

/*----------------------------------------------------------------------------
 *        Task 2 'phaseB': Phase B output
 *---------------------------------------------------------------------------*/
__task void phaseB (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    signal_func (t_phaseA);              /* call common signal function      */
  }
}

/*----------------------------------------------------------------------------
 *        Task 3 'clock': Signal Clock
 *---------------------------------------------------------------------------*/
__task void clock (void) {
  for (;;) {
    os_evt_wait_and (0x0100, 0xffff);    /* wait for an event flag 0x0100    */
    os_dly_wait (8);                     /* delay 8 clock ticks              */
  }
}

/*----------------------------------------------------------------------------
 *        Task 4 'init': Initialize
 *---------------------------------------------------------------------------*/
__task void init (void) {

  SYSCTL->RCGC2 |= (1 << 1);             /* Enable GPIOB clock               */
  __nop(); __nop(); __nop(); __nop();    /* Wait for peripheral ready        */
  GPIOB->DEN    |=  LED_PINS;            /* GPIOB.3 digital enable           */
  GPIOB->DIR    |=  LED_PINS;            /* GPIOB.3 output direction         */

  t_phaseA = os_tsk_create (phaseA, 0);  /* start task phaseA                */
  t_phaseB = os_tsk_create (phaseB, 0);  /* start task phaseB                */
  t_clock  = os_tsk_create (clock, 0);   /* start task clock                 */
  os_evt_set (0x0001, t_phaseA);         /* send signal event to task phaseA */
  os_tsk_delete_self ();
}

/*----------------------------------------------------------------------------
 *        Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/
int main (void) {

  SystemInit ();                         /* Sys setup (must be called first) */

  os_sys_init (init);                    /* Initialize RTX and start init    */
}

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/

