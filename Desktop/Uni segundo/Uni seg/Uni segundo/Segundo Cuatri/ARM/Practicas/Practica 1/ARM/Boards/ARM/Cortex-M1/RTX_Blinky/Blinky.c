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
#include "..\EB.h"                        /* Emulation Baseboard definitions */

OS_TID t_lights;                              /* assigned task id's          */
OS_TID t_blinking;

#define RED     0x01
#define YELLOW  0x02
#define GREEN   0x04
#define STOP    0x40
#define WALK    0x80

BIT normal_mode;

__task void init (void);                      /* Function prototypes         */
__task void blinking (void);
__task void lights (void);

/*--------------------------- LED_Out ---------------------------------------*/

static void LED_Out (U32 led) {
  HWR(BRD_BASE + BRD_LED) = led;              /* Output to LEDs              */
}

/*----------------------------------------------------------------------------
 *      Task 1 'lights': Normal Traffic light operation
 *---------------------------------------------------------------------------*/
__task void lights (void) {
  while (1) {                                 /* endless loop                */
    LED_Out (RED|STOP);                       /* RED & STOP lights on        */
    os_dly_wait (50);                         /* wait for timeout: 50 ticks  */
    if (normal_mode == __FALSE) {             /* if traffic signal time over */
      os_tsk_create (blinking, 0);            /* start blinking              */
      os_tsk_delete_self ();                  /* STOP lights                 */
    }
    LED_Out (RED|STOP|YELLOW);
    os_dly_wait (50);                         /* wait for timeout: 50 ticks  */
    LED_Out (STOP|GREEN);                     /* GREEN light for cars        */
    os_evt_clr (0x0010, t_lights);
    os_dly_wait (50);                         /* wait for timeout: 50 ticks  */
    os_evt_wait_and (0x0010, 750);            /* wait for event with timeout */
    LED_Out (STOP|YELLOW);                    /* timeout value: 750 ticks    */
    os_dly_wait (50);                         /* wait for timeout: 50 ticks  */
    LED_Out (STOP|RED);                       /* RED light for cars          */
    os_dly_wait (50);                         /* wait for timeout: 50 ticks  */
    LED_Out (WALK|RED);                       /* GREEN light for walkers     */    
    os_dly_wait (250);                        /* wait for timeout: 250 ticks */
  }
}

/*----------------------------------------------------------------------------
 *      Task 2 'blinking': runs outside normal traffic light operation
 *---------------------------------------------------------------------------*/
__task void blinking (void) {                 /* blink YELLOW light          */
  while (1) {                                 /* endless loop                */
    LED_Out (YELLOW);                         /* YELLOW light on             */
    os_dly_wait (30);                         /* wait for timeout: 30 ticks  */
    LED_Out (0);                              /* YELLOW light off            */
    os_dly_wait (30);                         /* wait for timeout: 30 ticks  */
    if (normal_mode == __TRUE) {              /* if blinking time over       */
      t_lights = os_tsk_create (lights, 0);   /* start lights                */
      os_tsk_delete_self ();                  /* and STOP blinking           */
    }
  }
}

/*----------------------------------------------------------------------------
 *        Task 5 'init': Initialize
 *---------------------------------------------------------------------------*/
__task void init (void) {

  normal_mode = __FALSE;
  t_blinking  = os_tsk_create (blinking,0);   /* start blinking              */
  os_dly_wait (500);
  normal_mode = __TRUE;
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
