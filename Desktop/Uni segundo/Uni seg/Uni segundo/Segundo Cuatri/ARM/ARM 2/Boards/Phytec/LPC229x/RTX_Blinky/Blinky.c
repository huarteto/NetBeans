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
#include <LPC22xx.h>                    /* LPC22xx Peripheral Registers      */


OS_TID t_blink;                         /* assigned task id of task: blink   */
OS_TID t_ledoff;                        /* assigned task id of task: ledoff  */


/*----------------------------------------------------------------------------
 *        Task 1 'blink': Blinks LED
 *---------------------------------------------------------------------------*/
__task void blink (void) {
  for (;;) {
    IOCLR0 = 0x00000100;                /* Turn LED On  (P0.8 = 0)           */
    os_evt_set (0x0001, t_ledoff);      /* send event to task 'ledoff'       */
    os_dly_wait (50);                   /* delay 50 clock ticks              */
  }
}

/*----------------------------------------------------------------------------
 *        Task 2 'blink_off': Blinks LED
 *---------------------------------------------------------------------------*/
__task void ledoff (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);   /* wait for an event flag 0x0001     */
    os_dly_wait (8);                    /* delay 8 clock ticks               */
    IOSET0 = 0x00000100;                /* Turn LED Off (P0.8 = 1)           */
  }
}

/*----------------------------------------------------------------------------
 *        Task 3 'init': Initialize
 *---------------------------------------------------------------------------*/
__task void init (void) {

  IODIR0 = 0x00000100;                   /* P0.8 defined as Output           */

  t_blink  = os_tsk_create (blink, 0);   /* start task 'blink'               */
  t_ledoff = os_tsk_create (ledoff, 0);  /* start task 'ledoff'              */
  os_tsk_delete_self ();
}

/*----------------------------------------------------------------------------
 *        Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/
int main (void) {
  os_sys_init (init);                   /* Initialize RTX and start init     */
}

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/

