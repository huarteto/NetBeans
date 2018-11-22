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
#include <ADuC7024.H>                   /* ADuC7024 definitions              */


OS_TID t_keyin;                         /* assigned task id of task: keyin   */
OS_TID t_blink;                         /* assigned task id of task: blink   */

U16    delay;                           /* clock tick delay global variable  */


/*----------------------------------------------------------------------------
 *        Task 1 'keyin': Key Input
 *---------------------------------------------------------------------------*/
__task void keyin (void) {
  os_itv_set (5);                       /* set wait interval: 5 clock ticks  */
  for (;;) {
    if (GP0DAT & 0x00000010) {          /* check P0.4 key input              */
      delay = 10;                       /* set 10 clock tick delay           */
    } else {
      delay = 20;                       /* set 20 clock tick delay           */
    }
    os_itv_wait ();                     /* wait interval                     */
  }
}

/*----------------------------------------------------------------------------
 *        Task 2 'blink': Blinks LED
 *---------------------------------------------------------------------------*/
__task void blink (void) {
  for (;;) {
    GP4DAT ^= 0x00040000;               /* toggle P4.2 LED                   */
    os_dly_wait (delay);                /* programmed delay                  */
  }
}

/*----------------------------------------------------------------------------
 *        Task 3 'init': Initialize
 *---------------------------------------------------------------------------*/
__task void init (void) {

  GP4DAT = 0x04040000;                   /* P4.2 defined as output           */

  t_keyin = os_tsk_create (keyin, 0);    /* start task 'keyin'               */
  t_blink = os_tsk_create (blink, 0);    /* start task 'blink'               */
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

