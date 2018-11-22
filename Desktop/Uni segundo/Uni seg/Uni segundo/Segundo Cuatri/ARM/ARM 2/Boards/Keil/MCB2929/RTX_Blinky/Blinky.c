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
#include <LPC29xx.H>                    /* LPC29xx definitions               */
#include "GLCD.h"

OS_TID t_phaseA;                        /* assigned task id of task: phase_a */
OS_TID t_phaseB;                        /* assigned task id of task: phase_b */
OS_TID t_phaseC;                        /* assigned task id of task: phase_c */
OS_TID t_phaseD;                        /* assigned task id of task: phase_d */
OS_TID t_clock;                         /* assigned task id of task: clock   */
OS_TID t_lcd;                           /* assigned task id of task: lcd     */

enum LED_ENUM {OFF, ON};

#define LED_0       (1 << 16)
#define LED_1       (1 << 17)
#define LED_2       (1 << 18)
#define LED_3       (1 << 19)
#define LED_4       (1 << 20)
#define LED_5       (1 << 21)

#define LED_DIR_OUT GPIO2_DR |=  LED_0 | LED_1 | LED_2 | LED_3 | LED_4 | LED_5;
#define LED_A(x)    ((x) ? (GPIO2_OR |= LED_0) : (GPIO2_OR &= ~LED_0));
#define LED_B(x)    ((x) ? (GPIO2_OR |= LED_1) : (GPIO2_OR &= ~LED_1));
#define LED_C(x)    ((x) ? (GPIO2_OR |= LED_2) : (GPIO2_OR &= ~LED_2));
#define LED_D(x)    ((x) ? (GPIO2_OR |= LED_3) : (GPIO2_OR &= ~LED_3));
#define LED_CLK(x)  ((x) ? (GPIO2_OR |= LED_5) : (GPIO2_OR &= ~LED_5));


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
 *        Task 5 'clock': Signal Clock
 *---------------------------------------------------------------------------*/
__task void clock (void) {
  for (;;) {
    os_evt_wait_and (0x0100, 0xffff);    /* wait for an event flag 0x0100    */
    LED_CLK(ON)
    os_dly_wait (8);                     /* delay 8 clock ticks              */
    LED_CLK(OFF)
  }
}

/*----------------------------------------------------------------------------
 *        Task 6 'lcd': LCD Control task
 *---------------------------------------------------------------------------*/
__task void lcd (void) {
  U32 i;

  GLCD_Init ();                          /* GLCD initialization */ 
  GLCD_Clear(White);                     /* Clear the GLCD */

  GLCD_SetBackColor(Blue);               /* Set the Back Color */
  GLCD_SetTextColor(White);              /* Set the Text Color */

  for (;;) {
    GLCD_Clear(White);                   /* Clear the GLCD */
    GLCD_DisplayString(0, 0, "      RL-ARM        ");
    GLCD_DisplayString(1, 0, "   Kernel example   ");
    os_dly_wait (200);
    GLCD_DisplayString(1, 0, "    www.keil.com      ");
    os_dly_wait (400);
    GLCD_Clear(White);                   /* Clear the GLCD */
    GLCD_DisplayString(0, 0, "        KEIL        ");
    GLCD_DisplayString(1, 0, "   An ARM Company   ");
    os_dly_wait (400);
    for (i = 0; i <= 320; i++) {
      GLCD_SetTextColor(Red);
      GLCD_Bargraph (0, 120, 320, 24, i);
      os_dly_wait (5);
    }
    GLCD_SetTextColor(White);            /* Set the Text Color */
    os_dly_wait (50);
  }
}

/*----------------------------------------------------------------------------
 *        Task 7 'init': Initialize
 *---------------------------------------------------------------------------*/
__task void init (void) {

  LED_DIR_OUT                            /* Define LED pins as Outputs       */

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

