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
#include <LPC325x.h>                    /* LPC325x definitions               */

OS_TID t_phaseA;                        /* assigned task id of task: phaseA  */
OS_TID t_taskB;                         /* assigned task id of task: taskB   */

#define LED_1   0x0002
#define LED_2   0x4000
#define LED_ALL 0x4002

#define LED_ON(led)  P3_OUTP_SET = led;
#define LED_OFF(led) P3_OUTP_CLR = led;

/*----------------------------------------------------------------------------
 *        Task 1 'phaseA': Phase A output
 *---------------------------------------------------------------------------*/
__task void phaseA (void) {
  for (;;) {
    LED_ON (LED_1);
    os_dly_wait (30);
    LED_OFF(LED_1);
    os_dly_wait (70);
  }
}

/*----------------------------------------------------------------------------
 *        Task 2 'taskB': Task B output (depending on millisecond timer)
 *---------------------------------------------------------------------------*/
__task void taskB (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    LED_ON (LED_2);
    os_dly_wait (2);
    LED_OFF(LED_2);
  }
}

/*----------------------------------------------------------------------------
 *        Task 5 'init': Initialize
 *---------------------------------------------------------------------------*/
__task void init (void) {

  LED_OFF (LED_ALL);

  t_phaseA = os_tsk_create (phaseA, 0);  /* start task phaseA                */
  t_taskB  = os_tsk_create (taskB,  0);  /* start task taskB                */
  os_evt_set (0x0001, t_phaseA);         /* send signal event to task phaseA */


  /* Initialize millisecond timer to generate interrupt every 100 ms         */
  MSTIM_MATCH1  =  3200 - 1;
  MSTIM_COUNTER =  0;
  MSTIM_INT     =  (1 <<  1);
  MSTIM_MCTRL   =  (1 <<  4) | (1 << 3);
  MSTIM_CTRL    =  (1 <<  0);
  MIC_APR      |=  (1 << 27);
  MIC_ATR      &= ~(1 << 27);
  MIC_ITR      &= ~(1 << 27);
  MIC_ER       |=  (1 << 27);

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

