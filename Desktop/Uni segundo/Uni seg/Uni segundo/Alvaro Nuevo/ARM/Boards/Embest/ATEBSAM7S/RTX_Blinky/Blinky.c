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
#include "Board.h"


OS_TID t_phaseA;                        /* assigned task id of task: phase_a */
OS_TID t_phaseB;                        /* assigned task id of task: phase_b */
OS_TID t_phaseC;                        /* assigned task id of task: phase_c */



/*----------------------------------------------------------------------------
 *        Task 1 'phaseA': Phase A output
 *---------------------------------------------------------------------------*/
__task void phaseA (void) {
  for (;;) {
    *AT91C_PIOA_CODR = LED3;
    os_dly_wait(50);

    *AT91C_PIOA_SODR = LED1;
  }
}

/*----------------------------------------------------------------------------
 *        Task 2 'phaseB': Phase B output
 *---------------------------------------------------------------------------*/
__task void phaseB (void) {
  for (;;) {
    *AT91C_PIOA_CODR = LED1;
    os_dly_wait(30);

    *AT91C_PIOA_SODR = LED2;
  }
}

/*----------------------------------------------------------------------------
 *        Task 3 'phaseC': Phase C output
 *---------------------------------------------------------------------------*/
__task void phaseC (void) {
  for (;;) {
    *AT91C_PIOA_CODR = LED2;
    os_dly_wait(20);

    *AT91C_PIOA_SODR = LED3;
  }
}



/*----------------------------------------------------------------------------
 *        Task 4 'init': Initialize
 *---------------------------------------------------------------------------*/
__task void init (void) {

  *AT91C_PMC_PCER = (1 << AT91C_ID_PIOA);   /* Enable Clock for PIO    */

  *AT91C_PIOA_PER  = LED_MASK;              /* Enable PIO for LED1..3  */
  *AT91C_PIOA_OER  = LED_MASK;              /* LED1..3 are Outputs     */
  *AT91C_PIOA_SODR = LED_MASK;              /* Turn off LED's ("1")    */

  t_phaseA = os_tsk_create (phaseA, 3);  /* start task phaseA                */
  t_phaseB = os_tsk_create (phaseB, 2);  /* start task phaseB                */
  t_phaseC = os_tsk_create (phaseC, 1);  /* start task phaseC                */
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

