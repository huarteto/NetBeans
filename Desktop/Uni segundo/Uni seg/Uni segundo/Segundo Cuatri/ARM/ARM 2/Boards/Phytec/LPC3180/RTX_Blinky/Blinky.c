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
#include <LPC318x.H>                    /* LPC318x definitions               */

OS_TID t_phaseA;                        /* assigned task id of task: phase_a */
OS_TID t_phaseB;                        /* assigned task id of task: phase_b */
OS_TID t_phaseC;                        /* assigned task id of task: phase_c */
OS_TID t_phaseD;                        /* assigned task id of task: phase_d */

enum LED_ENUM {OFF, ON};

#define LED_0       (1 << 2)
#define LED_1       (1 << 3)
#define LED_2       (1 << 7)
#define LED_3       (1 << 6)

#define LED_DIR_OUT PIO_DIR_SET  = LED_0 | LED_1 | LED_2 | LED_3;
#define LEDS_OFF    PIO_OUTP_CLR = LED_0 | LED_1 | LED_2 | LED_3;
#define LED_A(x)    ((x) ? (PIO_OUTP_SET = LED_0) : (PIO_OUTP_CLR = LED_0));
#define LED_B(x)    ((x) ? (PIO_OUTP_SET = LED_1) : (PIO_OUTP_CLR = LED_1));
#define LED_C(x)    ((x) ? (PIO_OUTP_SET = LED_2) : (PIO_OUTP_CLR = LED_2));
#define LED_D(x)    ((x) ? (PIO_OUTP_SET = LED_3) : (PIO_OUTP_CLR = LED_3));


/*----------------------------------------------------------------------------
 *        Function 'signal_func' called from multiple tasks
 *---------------------------------------------------------------------------*/
void signal_func (OS_TID task)  {
  os_dly_wait (100);                     /* delay 100 clock ticks            */
  os_evt_set  (0x0001, task);            /* send event to task 'task'        */
  os_dly_wait (50);                      /* delay 50 clock ticks             */
}

/*----------------------------------------------------------------------------
 *        Task 1 'phaseA': Phase A output
 *---------------------------------------------------------------------------*/
__task void phaseA (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    LED_A(ON)
    signal_func (t_phaseB);              /* call common signal function      */
    LED_A(OFF)
  }
}

/*----------------------------------------------------------------------------
 *        Task 2 'phaseB': Phase B output
 *---------------------------------------------------------------------------*/
__task void phaseB (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    LED_B(ON)
    signal_func (t_phaseC);              /* call common signal function      */
    LED_B(OFF)
  }
}

/*----------------------------------------------------------------------------
 *        Task 3 'phaseC': Phase C output
 *---------------------------------------------------------------------------*/
__task void phaseC (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    LED_C(ON)
    signal_func (t_phaseD);              /* call common signal function      */
    LED_C(OFF)
  }
}

/*----------------------------------------------------------------------------
 *        Task 4 'phaseD': Phase D output
 *---------------------------------------------------------------------------*/
__task void phaseD (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    LED_D(ON)
    signal_func (t_phaseA);              /* call common signal function      */
    LED_D(OFF)
  }
}

/*----------------------------------------------------------------------------
 *        Task 5 'init': Initialize
 *---------------------------------------------------------------------------*/
__task void init (void) {

  LED_DIR_OUT                            /* Define LED pins as Outputs       */
  LEDS_OFF                               /* Turn all LEDs off                */

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

