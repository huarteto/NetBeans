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
#include <LPC13xx.H>                    /* LPC13xx definitions                */

OS_TID t_phaseA;                        /* assigned task id of task: phase_a */
OS_TID t_phaseB;                        /* assigned task id of task: phase_b */
OS_TID t_phaseC;                        /* assigned task id of task: phase_c */
OS_TID t_phaseD;                        /* assigned task id of task: phase_d */
OS_TID t_clock;                         /* assigned task id of task: clock   */
OS_TID t_lcd;                           /* assigned task id of task: lcd     */

#define LED_A      0x01
#define LED_B      0x02
#define LED_C      0x04
#define LED_D      0x08
#define LED_CLK    0x80

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
    LPC_GPIO2->DATA &= ~LED_A;
    signal_func (t_phaseB);              /* call common signal function      */
    LPC_GPIO2->DATA |=  LED_A;
  }
}

/*----------------------------------------------------------------------------
 *        Task 2 'phaseB': Phase B output
 *---------------------------------------------------------------------------*/
__task void phaseB (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    LPC_GPIO2->DATA &= ~LED_B;
    signal_func (t_phaseC);              /* call common signal function      */
    LPC_GPIO2->DATA |=  LED_B;
  }
}

/*----------------------------------------------------------------------------
 *        Task 3 'phaseC': Phase C output
 *---------------------------------------------------------------------------*/
__task void phaseC (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    LPC_GPIO2->DATA &= ~LED_C;
    signal_func (t_phaseD);              /* call common signal function      */
    LPC_GPIO2->DATA |=  LED_C;
  }
}

/*----------------------------------------------------------------------------
 *        Task 4 'phaseD': Phase D output
 *---------------------------------------------------------------------------*/
__task void phaseD (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    LPC_GPIO2->DATA &= ~LED_D;
    signal_func (t_phaseA);              /* call common signal function      */
    LPC_GPIO2->DATA |=  LED_D;
  }
}

/*----------------------------------------------------------------------------
 *        Task 5 'clock': Signal Clock
 *---------------------------------------------------------------------------*/
__task void clock (void) {
  for (;;) {
    os_evt_wait_and (0x0100, 0xffff);    /* wait for an event flag 0x0100    */
    LPC_GPIO2->DATA &= ~LED_CLK;
    os_dly_wait (8);                     /* delay 8 clock ticks              */
    LPC_GPIO2->DATA |=  LED_CLK;
  }
}

/*----------------------------------------------------------------------------
 *        Task 6 'init': Initialize
 *---------------------------------------------------------------------------*/
__task void init (void) {

  LPC_SYSCON->SYSAHBCLKCTRL |= 1 << 6;   /* Enable clock for GPIO            */
  LPC_GPIO2->DIR = 0x000000FF;           /* LEDs on PORT2 defined as Output  */

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

  SystemInit();                          /* Initialize the MCU clocks        */
  LPC_SYSCON->TRACECLKDIV = 1;           /* enable trace for RTX awareness   */

  os_sys_init (init);                    /* Initialize RTX and start init    */
}

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/

