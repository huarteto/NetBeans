/*----------------------------------------------------------------------------
 *      RL-ARM - RTX
 *----------------------------------------------------------------------------
 *      Name:    RTX_CONFIG.C
 *      Purpose: Configuration of RTX Kernel for TI TMS470R1
 *      Rev.:    V4.05
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2009 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include <RTL.h>
#include <TMS470R1.h>                    /* TMS470R1 definitions             */
#include <rt_agent.h>

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
 #define OS_TASKCNT     9
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
//   <o>Hardware timer <0=> RTI
//   <i> Define the on-chip timer used as a time-base for RTX.
//   <i> Default: RTI
#ifndef OS_TIMER
 #define OS_TIMER       0
#endif

//   <o>Timer clock value [Hz] <1-1000000000>
//   <i> Set the timer clock value for selected timer.
//   <i> Default: 96000000  (SYSCLK = XTAL*8/1 = 60.0 MHz)
#ifndef OS_CLOCK
 #define OS_CLOCK       96000000
#endif

//   <o>Timer tick value [us] <1-1000000>
//   <i> Set the timer tick value for selected timer (in microseconds).
//   <i> Default: 10000  (10ms = 10000us)
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
 #define __RTA_HW_TYPE        RTA_HW_POLLED
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

#if   (OS_TIMER == 0)                                   /* RTI              */
 #define OS_PRES_       (1000)                          /*  Prescale value  */
 #define OS_MOD_        ((OS_PRES_ - 1) & 0x7FF)        /*  Modulo M reload */
 #define OS_TIM_        (1 <<  1)                       /*  Interrupt Mask  */
 #define OS_FRC_        (1 << 21)                       /*  Force Irq Mask  */
 #define OS_CMP2_EN_    (1 << 4)                        /*  Enable CMP2 Irq */
 #define OS_CMP2_FLG_   (1 << 6)                        /*  CMP2 Irq Flag   */
#if (__RTA_ENABLED == 1)
 /* If using the RT Agent, set the timer period to 250us                    */
 #define OS_RTA_TICK    (250)                           
 #define OS_TRV         (((1ULL*OS_CLOCK*OS_RTA_TICK))/1000000)
 #define OS_TICK_MUL    ((U32)(OS_TICK/OS_RTA_TICK))
#else   
 #define OS_TRV         (((1ULL*OS_CLOCK*OS_TICK))/1000000)
#endif
 #define OS_TVAL        (pSM->RTICNTR >> 11)            /*  Timer Value     */
 #define OS_TOVF        (pSM->RTICINT & OS_CMP2_FLG_)   /*  Overflow Flag   */
 #define OS_TREL()      pSM->RTICNTR = 0;               /*  Timer Reload    */
 #define OS_TFIRQ()     pSM->SSIR = (0x75 << 8) | 0xFF; /*  Force Interrupt */
 #define OS_TIACK()     if ((pSM->SSIF & 1) &&          /*  Interrupt Ack   */ \
                           ((pSM->SSIR & 0xFF) == 0xFF)) {                     \
                          pSM->SSIF    =  0;                                   \
                          pSM->INTREQ &= ~OS_FRC_;                             \
                        }                                                      \
                        pSM->RTICINT  &= ~OS_CMP2_FLG_;                        \
                        pSM->INTREQ   &= ~OS_TIM_;
 #define OS_TINIT()     pSM->RTIPCTL   =  (7 << 11) |   /*  Initialization  */ \
                                          OS_MOD_;                             \
                        pSM->RTICMP2   =  (OS_TRV / OS_PRES_);                 \
                        pSM->RTICINT   =  0;                                   \
                        pSM->RTICINT   =  OS_CMP2_EN_;                         \
                        pSM->RTICNTR   =  0;                                   \
                        pSM->REQMASK  |=  OS_TIM_ | OS_FRC_;
#else
 #error OS_TIMER invalid
#endif

#define OS_IACK()       ;                               /* Interrupt Ack    */

#define OS_LOCK()       pSM->REQMASK &= ~(OS_TIM_ | OS_FRC_);   /* Lock     */
#define OS_UNLOCK()     pSM->REQMASK |=  (OS_TIM_ | OS_FRC_);   /* Unlock   */

/* WARNING: Using IDLE mode might cause you troubles while debugging. */
#define _idle_()        ;


/*----------------------------------------------------------------------------
 *      Global Functions
 *---------------------------------------------------------------------------*/

extern void os_clock_interrupt (void);
extern void os_def_interrupt   (void) __irq;

/*--------------------------- IRQ_Handler -----------------------------------*/

/* This is a common IRQ handler for all interrupts. The correct interrupt is */
/* called dependent on the value in INTREQ. The timing of the system timer   */
/* interrupt depends on if the RTA is enabled or not. In the case where the  */
/* RTA is not enabled, the system timer interrupt is called every OS_TICK.   */
/* In the case where the RT Agent is enabled, the system timer interrupt is  */
/* called every 250us, and a multiplier is used to call os_clock_interrupt   */
/* every OS_TICK. When the OS_TICK is not an integer multiple of 250us, this */
/* can result in an inaccuracy of up to 250us. If a more accurate OS_TICK is */
/* required in this case, the 250us value (defined in OS_RTA_TICK) should    */
/* be modified to an integer multiple of OS_TICK (preferably still operating */
/* approximately once every 250us).                                          */

//extern __irq void GPIOA_IRQHandler (void);
//extern __irq void HET_IRQHandler   (void);

//#define GPIOA_IRQ       (1 << 5)
//#define HET_IRQ         (1 << 7)


#if (__RTA_ENABLED == 0)

__asm void IRQ_Handler (void) {
   /* Common IRQ Handler */
        PRESERVE8
        ARM

        STMDB   SP!,{R0}                       ; Save R0
        LDR     R0,=__cpp((U32)&pSM->INTREQ)   ; Load INTREQ Address
        LDR     R0,[R0]                        ; Load INTREQ Value

        TST     R0,#OS_TIM_                    ; Check RTI_COMP2_IRQ Flag
        LDMNEIA SP!,{R0}                       ; Restore R0
        LDRNE   PC,=__cpp(os_clock_interrupt)  ; OS Clock IRQ Function

//      TST     R0,#GPIOA_IRQ                  ; Check GPIOA_IRQ Flag
//      LDMNEIA SP!,{R0}                       ; Restore R0
//      LDRNE   PC,=__cpp(GPIOA_IRQHandler)    ; GPIOA_IRQ Function

//      TST     R0,#HET_IRQ                    ; Check HET_IRQ Flag
//      LDMNEIA SP!,{R0}                       ; Restore R0
//      LDRNE   PC,=__cpp(HET_IRQHandler)      ; HET_IRQ Function

        TST     R0,#OS_FRC_                    ; Check SYSTEM_IRQ Flag
        LDRNE   R0,=__cpp((U32)&pSM->SSIR)     ; Load SSIR Address
        LDRNE   R0,[R0]                        ; Load SSIR Value
        ANDNE   R0,R0, #0xFF                   ; Leave only SSDATA
        TEQ     R0,#0xFF                       ; Check if forced timer IRQ
        LDMEQIA SP!,{R0}                       ; Restore R0
        LDREQ   PC,=__cpp(os_clock_interrupt)  ; OS Clock IRQ Function
        LDMIA   SP!,{R0}                       ; Restore R0
        LDR     PC,=__cpp(os_def_interrupt)    ; OS Default IRQ Function
}

#else // #if (__RTA_ENABLED == 0)

U16 g_rtx_timer_cnt[2] = { OS_TICK_MUL, OS_TICK_MUL };

__irq void os_rtx_interrupt(void) {
   OS_TREL();                                /* Reload the OS timer */
   OS_TIACK();                               /* Acknowledge the current IRQ */
}

__asm void IRQ_Handler (void) {
   /* Common IRQ Handler (with RT Agent) */
        PRESERVE8
        ARM

        IMPORT    g_rtx_timer_cnt

        STMDB   SP!,{R0-R1}                    ; Save R0-R1
        LDR     R0,=__cpp((U32)&pSM->INTREQ)   ; Load INTREQ Address
        LDR     R0,[R0]                        ; Load INTREQ Value
        TST     R0,#OS_TIM_                    ; Check RTI_COMP2_IRQ Flag   
        BNE     os_irq                         ; Branch to the OS timer IRQ

//      TST     R0,#GPIOA_IRQ                  ; Check GPIOA_IRQ Flag
//      LDMNEIA SP!,{R0-R1}                    ; Restore R0-R1
//      LDRNE   PC,=__cpp(GPIOA_IRQHandler)    ; GPIOA_IRQ Function

//      TST     R0,#HET_IRQ                    ; Check HET_IRQ Flag
//      LDMNEIA SP!,{R0-R1}                    ; Restore R0-R1
//      LDRNE   PC,=__cpp(HET_IRQHandler)      ; HET_IRQ Function

        TST     R0,#OS_FRC_                    ; Check SYSTEM_IRQ Flag
        LDRNE   R0,=__cpp((U32)&pSM->SSIR)     ; Load SSIR Address
        LDRNE   R0,[R0]                        ; Load SSIR Value
        ANDNE   R0,R0,#0xFF                    ; Leave only SSDATA
        TEQ     R0,#0xFF                       ; Check if forced timer IRQ
        LDMIA   SP!,{R0-R1}                    ; Restore R0-R1
        LDREQ   PC,=__cpp(os_clock_interrupt)  ; OS Clock IRQ Function
        LDR     PC,=__cpp(os_def_interrupt)    ; OS Default IRQ Function


os_irq  STMDB   SP!,{R2-R12,LR}                ; Save R2-R12,LR
        BL    __cpp(RTA_rx_word_ext)           ; Call RTA RX IRQ Function
        BL    __cpp(RTA_tx_word_ext)           ; Call RTA TX IRQ Function  
        LDMIA   SP!,{R2-R12,LR}                ; Restore R2-R12,LR

        ; Decrement and check counter
        LDR     R0,=g_rtx_timer_cnt            ; Load timer count address
        LDRH    R1,[R0]                        ; Load timer count
        SUBS    R1,#1                          ; Decrement timer count
        LDREQH  R1,[R0,#2]                     ; Reload if R1 == 0
        STRH    R1,[R0]                        ; Write the decremented timer count
        LDMIA   SP!,{R0-R1}                    ; Restore R0-R1
        LDRNE   PC,=__cpp(os_rtx_interrupt)    ; RTX IRQ Function (acknowledges timer IRQ)
        LDR     PC,=__cpp(os_clock_interrupt)  ; OS Clock IRQ Function   
}

#endif // #if (__RTA_ENABLED == 0)



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

void os_def_interrupt (void) __irq  {
  /* Default Interrupt Function: may be called when timer ISR is disabled */
  OS_IACK();
}

#if (__RTA_ENABLED == 1)
#define RTA_irq_enable_ext  tsk_unlock
#define RTA_irq_disable_ext tsk_lock
#endif

#include <RTX_lib.c>

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/
