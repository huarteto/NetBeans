/*----------------------------------------------------------------------------
 *      RL-ARM - RTX
 *----------------------------------------------------------------------------
 *      Name:    RTX_CONFIG.C
 *      Purpose: Configuration of RTX Kernel for NXP LPC23xx
 *      Rev.:    V4.05
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2009 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include <RTL.h>
#include <LPC23xx.H>                     /* LPC23xx definitions              */

/*----------------------------------------------------------------------------
 *      RTX User configuration part BEGIN
 *---------------------------------------------------------------------------*/

//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------
//
// <h>Task Configuration
// =====================
//
//   <o>Number of concurrent running tasks <0-250>
//   <i> Define max. number of tasks that will run at the same time.
//   <i> Default: 6
#ifndef OS_TASKCNT
 #define OS_TASKCNT     8
#endif

//   <o>Number of tasks with user-provided stack <0-250>
//   <i> Define the number of tasks that will use a bigger stack.
//   <i> The memory space for the stack is provided by the user.
//   <i> Default: 0
#ifndef OS_PRIVCNT
 #define OS_PRIVCNT     0
#endif

//   <o>Task stack size [bytes] <20-4096:8><#/4>
//   <i> Set the stack size for tasks which is assigned by the system.
//   <i> Default: 200
#ifndef OS_STKSIZE
 #define OS_STKSIZE     64
#endif

// <q>Check for the stack overflow
// ===============================
// <i> Include the stack checking code for a stack overflow.
// <i> Note that additional code reduces the RTX performance.
#ifndef OS_STKCHECK
 #define OS_STKCHECK    1
#endif

// </h>
// <h>Tick Timer Configuration
// =============================
//   <o>Hardware timer <0=> Timer 0 <1=> Timer 1 <2=> Timer 2 <3=> Timer 3
//   <i> Define the on-chip timer used as a time-base for RTX.
//   <i> Default: Timer 0
#ifndef OS_TIMER
 #define OS_TIMER       0
#endif

//   <o>Timer clock value [Hz] <1-1000000000>
//   <i> Set the timer clock value for selected timer.
//   <i> Default: 15000000  (15MHz at 60MHz CCLK and VPBDIV = 4)
#ifndef OS_CLOCK
 #define OS_CLOCK       12000000
#endif

//   <o>Timer tick value [us] <1-1000000>
//   <i> Set the timer tick value for selected timer.
//   <i> Default: 10000  (10ms)
#ifndef OS_TICK
 #define OS_TICK        10000
#endif

// </h>

// <h>System Configuration
// =======================
// <e>Round-Robin Task switching
// =============================
// <i> Enable Round-Robin Task switching.
#ifndef OS_ROBIN
 #define OS_ROBIN       1
#endif

//   <o>Round-Robin Timeout [ticks] <1-1000>
//   <i> Define how long a task will execute before a task switch.
//   <i> Default: 5
#ifndef OS_ROBINTOUT
 #define OS_ROBINTOUT   5
#endif

// </e>

//   <o>Number of user timers <0-250>
//   <i> Define max. number of user timers that will run at the same time.
//   <i> Default: 0  (User timers disabled)
#ifndef OS_TIMERCNT
 #define OS_TIMERCNT    0
#endif

//   <o>ISR FIFO Queue size<4=>   4 entries  <8=>   8 entries
//                         <12=> 12 entries  <16=> 16 entries
//                         <24=> 24 entries  <32=> 32 entries
//                         <48=> 48 entries  <64=> 64 entries
//                         <96=> 96 entries
//   <i> ISR functions store requests to this buffer,
//   <i> when they are called from the IRQ handler.
//   <i> Default: 16 entries
#ifndef OS_FIFOSZ
 #define OS_FIFOSZ      16
#endif

// </h>

// <e0>Real-Time Agent
// <i>Enable Real-Time Agent to obtain run-time debug information.
#ifndef __RTA_ENABLED
 #define __RTA_ENABLED         1
#endif

// <o0>Output Buffer Size (bytes) <i>If you are experiencing buffer overruns, increase the buffer size.
// <i>Default: 256
//    <256=>256
//    <512=>512
//    <1024=>1024
//    <2048=>2048
#ifndef __RTA_OUT_SIZE_BYTES
 #define __RTA_OUT_SIZE_BYTES  512
#endif
#define __RTA_OUT_SIZE         (__RTA_OUT_SIZE_BYTES / 4)

// <o>  Invalid Memory Pattern <0x0-0xFFFFFFFF>
// <i>  This pattern is shown at inaccessible locations in the memory window at run-time.
// <i>  Default: 0xCCCCCCCC
#ifndef __RTA_NO_MEM_FLAG
 #define __RTA_NO_MEM_FLAG     0xCCCCCCCC
#endif

// <e0>Enable Real-Time Agent Terminal
//   <o1.0>Echo Characters Automatically<i>Echoes back characters typed in the serial window
//          <i>Default: Off
//   <o2.0>Block on Write<i>Waits for serial buffer to have available space to send data.
//          <i>This option must be disabled to run without the debugger.
//          <i>Default: On
//  </e>
#ifndef __RTA_TERM_SUPPORT
 #define __RTA_TERM_SUPPORT    1
#endif

/* Echo characters back to the serial window automatically */
#ifndef __RTA_TERM_ECHO
 #define __RTA_TERM_ECHO       0
#endif

/* Blocking writes - write  blocks when output buffer full*/
#ifndef __RTA_TERM_W_BLOCKING
 #define __RTA_TERM_W_BLOCKING 1
#endif

// <o0.0>Task switch monitoring (RTX Kernel only)
// =============================
// <i> Enable Real-Time Agent task switch monitoring.
#ifndef __RTA_RTX_TASK_SWITCHING
 #define __RTA_RTX_TASK_SWITCHING     1
#endif

// </e>

// <e0>I/O Retargeting
// <i>Enable I/O retargeting
#ifndef __IO_RETARGET_ENABLED
 #define __IO_RETARGET_ENABLED 1
#endif

/* Possible stream sources/destinations */
#define __IO_RETARGET_STREAM_NONE           0
#define __IO_RETARGET_STREAM_RTA            1
#define __IO_RETARGET_STREAM_COM_0          2
#define __IO_RETARGET_STREAM_COM_1          3

// <o0>STDIO Routing (for printf(), scanf(), etc) <i>Select the route for standard input/output, and standard error
//   <0=> None
//   <1=> Real-Time Agent
//   <3=> COM Port 1
#ifndef __IO_RETARGET_STDIO_ROUTE
 #define __IO_RETARGET_STDIO_ROUTE          1
#endif


// <o0.0>External file operations (eg RL Flash File System)
// <i>Enable if __fopen(), __fclose(), __write(), __read(), __setfpos(), __flushbuf() and __get_flen() are defined
#ifndef __IO_RETARGET_FOPEN_ETC_ENABLED
 #define __IO_RETARGET_FOPEN_ETC_ENABLED    0
#endif

// </e>

//------------- <<< end of configuration section >>> -----------------------

/* Terminal Emulator input buffer size (must be a power of 2)
     This should be modified based on the amount of
     data being sent, and the speed of the JTAG. A faster
     JTAG and/or greater amount of data would require a 
     larger buffer size. */
#ifndef __RTA_TERM_IN_SIZE
 #define __RTA_TERM_IN_SIZE   16
#endif

/* Override the RTA_Config.c hardware type */
#define RTA_HW_LPC21xx        1
#define RTA_HW_LPC23xx        2
#define RTA_HW_STR91x         3
#define RTA_HW_AT91SAM7Sx     4
#define RTA_HW_AT91SAM7Xx     5
#define RTA_HW_LPC22xx        6
#define RTA_HW_AT91SAM9261    7
#define RTA_HW_AT91SAM9263    8
#define RTA_HW_POLLED         99
#define RTA_HW_CUSTOM         100

#ifndef __RTA_HW_TYPE
 #define __RTA_HW_TYPE        RTA_HW_LPC23xx
#endif

/* RTA is configured in file RTX_Config.c */
#define __RTA_RTX_CONFIG      1

// Standard library system mutexes
// ===============================
//  Define max. number system mutexes that are used to protect 
//  the arm standard runtime library. For microlib they are not used.
#ifndef OS_MUTEXCNT
 #define OS_MUTEXCNT    8
#endif

/*----------------------------------------------------------------------------
 *      RTX User configuration part END
 *---------------------------------------------------------------------------*/

#if   (OS_TIMER == 0)                                   /* Timer 0          */
 #define OS_TID_        4                               /*  Timer ID        */
 #define PCON_VAL       1
 #define TIMx(reg)      T0##reg
 #define VICVectAddrX   VICVectAddr4
#elif (OS_TIMER == 1)                                   /* Timer 1          */
 #define OS_TID_        5                               /*  Timer ID        */
 #define PCON_VAL       2
 #define TIMx(reg)      T1##reg
 #define VICVectAddrX   VICVectAddr5
#elif (OS_TIMER == 2)                                   /* Timer 2          */
 #define OS_TID_        26                              /*  Timer ID        */
 #define PCON_VAL       22
 #define TIMx(reg)      T2##reg
 #define VICVectAddrX   VICVectAddr26
#elif (OS_TIMER == 3)                                   /* Timer 3          */
 #define OS_TID_        27                              /*  Timer ID        */
 #define PCON_VAL       23
 #define TIMx(reg)      T3##reg
 #define VICVectAddrX   VICVectAddr27
#else
 #error OS_TIMER invalid
#endif

#define OS_TIM_         (1 << OS_TID_)                  /*  Interrupt Mask  */
#define OS_TRV          ((((1ULL*OS_CLOCK*OS_TICK))/1000000)-1)
#define OS_TVAL         TIMx(TC)                        /*  Timer Value     */
#define OS_TOVF         (TIMx(IR) & 1)                  /*  Overflow Flag   */
#define OS_TFIRQ()      VICSoftInt    = OS_TIM_;        /*  Force Interrupt */
#define OS_TIACK()      TIMx(IR) = 1;                   /*  Interrupt Ack   */ \
                        VICSoftIntClr = OS_TIM_;                               \
                        VICVectAddr   = 0;
#define OS_TINIT()      PCONP |= (1<<PCON_VAL);         /*  Initialization  */ \
                        TIMx(MR0) = OS_TRV;                                    \
                        TIMx(MCR) = 3;                                         \
                        TIMx(TCR) = 1;                                         \
                        VICVectAddrX = (U32)os_clock_interrupt;

#define OS_IACK()       VICVectAddr   = 0;              /* Interrupt Ack    */

#define OS_LOCK()       VICIntEnClr   = OS_TIM_;        /* Task Lock        */
#define OS_UNLOCK()     VICIntEnable  = OS_TIM_;        /* Task Unlock      */

/* WARNING: Using IDLE mode might cause you troubles while debugging. */
#define _idle_()        PCON |= 1;


/*----------------------------------------------------------------------------
 *      Global Functions
 *---------------------------------------------------------------------------*/

/*--------------------------- os_idle_demon ---------------------------------*/

__task void os_idle_demon (void) {
  /* The idle demon is a system task, running when no other task is ready */
  /* to run. The 'os_xxx' function calls are not allowed from this task.  */

  for (;;) {
  /* HERE: include optional user code to be executed when no task runs.*/
  }
}


/*--------------------------- os_tmr_call -----------------------------------*/

void os_tmr_call (U16 info) {
  /* This function is called when the user timer has expired. Parameter  */
  /* 'info' holds the value, defined when the timer was created.         */

  /* HERE: include optional user code to be executed on timeout. */
}


/*--------------------------- os_error --------------------------------------*/

void os_error (U32 err_code) {
  /* This function is called when a runtime error is detected. Parameter */
  /* 'err_code' holds the runtime error code (defined in RTL.H).         */

  /* HERE: include optional code to be executed on runtime error. */
  for (;;);
}


/*----------------------------------------------------------------------------
 *      RTX Configuration Functions
 *---------------------------------------------------------------------------*/


#include <RTX_lib.c>

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/
