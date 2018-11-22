/******************************************************************************/
/* Blinky.c: LED Flasher                                                      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2008 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <TMS470R1.h>                   /* TMS470R1 definitions               */
#include <RTL.h>
#include <stdio.h>                      /* Standard I/O definitions           */
#include <RT_Agent.h>                   /* RT Agent definitions               */


const int LED_TABLE[] = {0x00000001, 0x00000004, 0x00000010, 0x00000040,
                         0x00000080, 0x00000100, 0x00000400, 0x00000800,
                         0x00001000, 0x00002000, 0x00040000, 0x00080000,
                         0x00100000, 0x00200000, 0x01000000, 0x80000000 };


OS_TID t_clock;                         /* assigned task id of task: clock   */
OS_TID t_led;                           /* assigned task id of task: led     */

 /* LED_reroute --------------------------------------------------------------\
|                                                                              |
|  Make a output value according to input value of LEDs states (LEDs are not   |
|  sequential on the port pins                                                 |
 \---------------------------------------------------------------------------*/

unsigned int LED_reroute (unsigned int LED_states)  {
  unsigned int i, j;
  unsigned int temp = 0;

  j = 0;
  for (i = 0x0001; i <= 0x8000; i <<= 1)  {
    if (LED_states & i)
      temp |= LED_TABLE[j];
    j++;
  } 

  return (~temp);
}

 /* Setup --------------------------------------------------------------------\
|                                                                              |
|  This function setups the HETs as outputs for driving LEDs and setups RTI    |
|  interrupt, it is implemented through SWI call because it is required that   |
|  setting be done in Supervisory Mode of ARM                                  |
|  Parameters:                                                                 |
|  Return value:                                                               |
|                                                                              |
 \---------------------------------------------------------------------------*/

void __swi(8) LED_Setup (void);
void __SWI_8      (void) {

  pHET->HETDIR = 0xFFFFFFFF;            /* Setup HETs as outputs (for LED)    */
}


 /* HET_LED ------------------------------------------------------------------\
|                                                                              |
|  Set values for LEDs                                                         |
|  Parameters:      val  - LEDs states requested                               |
|  Return value:                                                               |
|                                                                              |
 \---------------------------------------------------------------------------*/

void __swi(9) HET_LED (unsigned int val);
void __SWI_9      (unsigned int val) {

  pHET->HETDOUT = LED_reroute(val);
}

/*----------------------------------------------------------------------------
 *        Task 1 'clock': Signal Clock
 *---------------------------------------------------------------------------*/
__task void clock (void) {
  for (;;) { 
    os_dly_wait(10);                    /* Delay (10 * OS_TICK = 100ms)       */
    os_evt_set (0x0001, t_led);         /* send event to 't_led' task         */
  }
}

/*----------------------------------------------------------------------------
 *        Task 2 'lcd': LCD Control task
 *---------------------------------------------------------------------------*/
__task void led (void) {
  static unsigned int ledShift = 0x0011;
  unsigned int k, cnt=0;

  for (;;) {                              /* Loop forever                     */
    os_evt_wait_and (0x0001, 0xffff);     /* wait for an event flag 0x0001    */

    /* Shift LEDs and turn them ON according to state                         */
    k = (ledShift >> 16) | ledShift;
    HET_LED (k);                          /* Activate LEDs                    */
    ledShift <<= 1;                       /* Shift LEDs for next time         */
    if (ledShift > 0x00088000) {
      ledShift = 0x0011;                    /* Reset the leds to start        */
      printf("Completed %d blinkies...\n", ++cnt);
    }
  }
}

/*----------------------------------------------------------------------------
 *        Task 3 'init': Initialize
 *---------------------------------------------------------------------------*/
__task void init (void) {
  printf("Initializing...\n");
  LED_Setup ();                         /* Setup HETs for LEDs                */

  t_clock  = os_tsk_create (clock, 0);   /* start task clock                 */
  t_led    = os_tsk_create (led, 0);     /* start task lcd                   */
  os_tsk_delete_self ();
}


/******************************************************************************
* Main Program                                                                *
*******************************************************************************/

int main (void) {
  os_sys_init (init);                    /* Initialize RTX and start init    */
}
