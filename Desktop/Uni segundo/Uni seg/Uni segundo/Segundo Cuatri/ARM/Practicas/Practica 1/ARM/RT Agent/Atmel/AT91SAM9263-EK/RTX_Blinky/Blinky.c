/*----------------------------------------------------------------------------
 *      R T L   K e r n e l   E x a m p l e
 *----------------------------------------------------------------------------
 *      Name:    BLINKY.C
 *      Purpose: RTX example program
 *      Rev.:    V3.20
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2008 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include <RTL.h>
#include <AT91SAM9263.H>                /* AT91SAM9263 definitions           */
#include <stdio.h>                      /* Standard IO definitions           */
#include <RT_Agent.h>                   /* Real-Time Agent definitions       */

const int led_mask[] = { AT91C_PIO_PB8, AT91C_PIO_PC29 };  /* LEDs DS1 and DS2*/



OS_TID t_phaseA;                        /* assigned task id of task: phase_a */
OS_TID t_phaseB;                        /* assigned task id of task: phase_b */

/*
 * Activate LED with index, state 0 = OFF, 1 = ON
 */  

void LED (unsigned int index, unsigned int state)  {
  switch (index) {
    case 0:
      if (state) AT91C_BASE_PIOB->PIO_CODR = led_mask[0];
      else       AT91C_BASE_PIOB->PIO_SODR = led_mask[0];
      break;
    case 1:
      if (state) AT91C_BASE_PIOC->PIO_CODR = led_mask[1];
      else       AT91C_BASE_PIOC->PIO_SODR = led_mask[1];
      break;
  }
}

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
    RTA_printf("A");
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    LED (0, 0);
    signal_func (t_phaseB);              /* call common signal function      */
    LED (0, 1);
  }
}

/*----------------------------------------------------------------------------
 *        Task 2 'phaseB': Phase B output
 *---------------------------------------------------------------------------*/
__task void phaseB (void) {
  for (;;) {
    RTA_printf("B\n");
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    LED (1, 0);
    signal_func (t_phaseA);              /* call common signal function      */
    LED (1, 1);
  }
}

/*----------------------------------------------------------------------------
 *        Task 0 'init': Initialize
 *---------------------------------------------------------------------------*/
__task void init (void) {

  // Enable the Clock of the PIO for LEDs
  AT91C_BASE_PMC->PMC_PCER  = 1 << AT91C_ID_PIOB;
  AT91C_BASE_PMC->PMC_PCER  = 1 << AT91C_ID_PIOCDE;

  // Configure the PIO Lines corresponding to LEDs DS1 and DS2 as Output
  AT91C_BASE_PIOB->PIO_OER  = led_mask[0];
  AT91C_BASE_PIOC->PIO_OER  = led_mask[1];

  // Clear the LEDs. On the Board we must apply a "1" to turn off LEDs
  AT91C_BASE_PIOB->PIO_SODR = led_mask[0];
  AT91C_BASE_PIOC->PIO_SODR = led_mask[1];

  t_phaseA = os_tsk_create (phaseA, 0);  /* start task phaseA                */
  t_phaseB = os_tsk_create (phaseB, 0);  /* start task phaseB                */
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

