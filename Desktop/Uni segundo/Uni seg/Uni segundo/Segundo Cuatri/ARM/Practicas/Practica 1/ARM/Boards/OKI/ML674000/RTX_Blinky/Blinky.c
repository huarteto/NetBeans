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
#include <ML674000.H>
#include "Blinky.h"


OS_TID t_key;                            /* Task ID of task: key */
OS_TID t_cnt;                            /* Task ID of task: count */


/* 7-segment LED Display Pattern Table */
const unsigned char LED_Table[16] = {
  LED_0, LED_1, LED_2, LED_3, LED_4, LED_5, LED_6, LED_7,
  LED_8, LED_9, LED_A, LED_b, LED_C, LED_d, LED_E, LED_F
};

volatile unsigned long Counter;          /* Counter Value */
volatile unsigned long Direction;        /* Count Direction: 0 - Up, 1 - Down */

extern void (*IRQ_ISR[32])(void);        /* IRQ Interrupt Service Routines */


/*
 * LED Initialization
 */

void LED_Init (void) {
  REG(GPPMB) = LED_MSK;
}


/*
 * LED Display
 *   Parameter: LED Num
 */

void LED_Display (unsigned long num) {
  REG(GPPOB) = LED_Table[num & 0x0F];
}


/*
 * External Interrupt 0 Service Routine
 *   executed on EXINT0 Falling Edge
 */

void extint0_isr (void) {

  isr_evt_set(0x0001, t_key);            /* Send Event to task 'key' */
  REG(IRQA) = IRQA_IRQ28;                /* nIRQ[28]: Clear Interrupt */
}


/*
 * Setup External Interrupt 0
 */

void exint0_init (void) {

  IRQ_ISR[28] = &extint0_isr;            /* nIRQ[28]: ISR */
  REG(IDM)   |= IDM_IDM28;               /* nIRQ[28]: Falling Edge */
  REG(IRQA)   = IRQA_IRQ28;              /* nIRQ[28]: Clear Interrupt */
  REG(ILC)   |= ILC_ILC28 & ILC_INT_LV1; /* nIRQ[28]: Level 1 */
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
    os_evt_wait_and(0x0001, 0xFFFF);     /* Wait for an Event flag 0x0001 */
    Direction ^= 1;                      /* Change Direction */
  }
}


/*----------------------------------------------------------------------------
 *        Task 3 'init': Initialize
 *---------------------------------------------------------------------------*/
__task void init (void) {

  exint0_init();                         /* Ext. Int. 0 Initialization */
  LED_Init();                            /* LED Initialization */

  t_cnt = os_tsk_create(count, 0);       /* start task 'count' */
  t_key = os_tsk_create(key,   0);       /* start task 'key'   */
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

