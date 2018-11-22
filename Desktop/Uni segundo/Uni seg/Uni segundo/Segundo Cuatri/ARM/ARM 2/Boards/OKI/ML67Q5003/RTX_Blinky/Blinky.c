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
#include <ML675001.H>
#include "Blinky.h"


/* 7-segment LED Display Pattern Table */
const unsigned char LED_Table[16] = {
  LED_0, LED_1, LED_2, LED_3, LED_4, LED_5, LED_6, LED_7,
  LED_8, LED_9, LED_A, LED_b, LED_C, LED_d, LED_E, LED_F
};

volatile unsigned long Counter;          /* Counter Value */
volatile unsigned long Button;           /* Button: 0 - Off, 1 - On */
volatile unsigned long Direction;        /* Count Direction: 0 - Up, 1 - Down */

/*
 * LED Initialization
 */

void LED_Init (void) {

  REG(GPPMB) = (REG(GPPMB) & ~LED_BM) | LED_BM;
  REG(GPPMC) = (REG(GPPMC) & ~LED_CM) | LED_CM;
}


/*
 * LED Display
 *   Parameter: LED Num
 */

void LED_Display (unsigned long num) {
  unsigned long val;

  val = LED_Table[num & 0x0F];
  REG(GPPOB) = (REG(GPPOB) & ~LED_BM) | ((val >> 0) & LED_BM);
  REG(GPPOC) = (REG(GPPOC) & ~LED_CM) | ((val >> 6) & LED_CM);
}


/*----------------------------------------------------------------------------
 *        Task 1 'count': LED Counter
 *---------------------------------------------------------------------------*/
__task void count (void) {

  for (;;) {
    LED_Display(Counter);                /* Display Counter */
    os_dly_wait(50);                     /* Wait */
    if (Direction == 0) {                /* Check Direction */
      Counter++;                         /* Count Up */
    } else {
      Counter--;                         /* Count Down */
    }
  }
}


/*----------------------------------------------------------------------------
 *        Task 2 'key': Key Input
 *---------------------------------------------------------------------------*/
__task void key (void) {

  for (;;) {
    if ((REG(GPPIE) & EXINT0) == 0) {    /* Check EXINT0 Pin */
      if (!Button) {                     /* Button not active */
        Direction ^= 1;                  /* Change Direction */
        Button = 1;                      /* Button On */
      }
    } else {
      Button = 0;                        /* Button Off */
    }
    os_dly_wait(10);                     /* Wait */
  }
}


/*----------------------------------------------------------------------------
 *        Task 3 'init': Initialize
 *---------------------------------------------------------------------------*/
__task void init (void) {

  LED_Init();                            /* LED Initialization */

  os_tsk_create(count, 0);               /* start task 'count' */
  os_tsk_create(key,   0);               /* start task 'key'   */
  os_tsk_delete_self();
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

