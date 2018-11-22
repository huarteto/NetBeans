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
#include <LM3Sxxxx.H>

OS_TID t_phaseA;                        /* assigned task id of task: phase_a */
OS_TID t_phaseB;                        /* assigned task id of task: phase_b */
OS_TID t_clock;                         /* assigned task id of task: clock   */

#define LED_A    GPIO_PIN_3
#define LED_B    GPIO_PIN_2
#define LED_CLK  GPIO_PIN_0
#define LED_PINS (LED_A | LED_B | LED_CLK)

/*--------------------------- LED_Out ---------------------------------------*/

static void LED_Out (U32 led, U32 st) {
  GPIOPinWrite (GPIO_PORTF_BASE, led, st ? led : 0);
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
    LED_Out (LED_B, 1);
    signal_func (t_phaseA);              /* call common signal function      */
    LED_Out (LED_B, 0);
  }
}

/*----------------------------------------------------------------------------
 *        Task 3 'clock': Signal Clock
 *---------------------------------------------------------------------------*/
__task void clock (void) {
  for (;;) {
    os_evt_wait_and (0x0100, 0xffff);    /* wait for an event flag 0x0100    */
    LED_Out (LED_CLK, 1);
    os_dly_wait (8);                     /* delay 8 clock ticks              */
    LED_Out (LED_CLK, 0);
  }
}

/*----------------------------------------------------------------------------
 *        Task 4 'init': Initialize
 *---------------------------------------------------------------------------*/
__task void init (void) {

  /* Set the clocking to run directly from the crystal                        */
  SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_6MHZ);

  /* Set GPIOF LEDs pins as outputs */
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
  GPIOPadConfigSet(GPIO_PORTF_BASE, LED_PINS, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);
  GPIODirModeSet(GPIO_PORTF_BASE, LED_PINS, GPIO_DIR_MODE_OUT);
  LED_Out (LED_PINS, 0);

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

  os_sys_init (init);                    /* Initialize RTX and start init    */
}

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/

