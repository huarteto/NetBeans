/******************************************************************************/
/* Retarget.c: IO Retargetting for the Keil Real-Time Agent                   */
/******************************************************************************/
/*                                                                            */                           
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2006 Keil - An ARM Company. All rights reserved.             */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include "RT_Agent.H"

// Serial routines
extern int sendchar (int ch);
extern int getkey (void);

/*==============================================================================
   IO Retargetting 
==============================================================================*/

/* Possible stream sources/destinations */
#define STREAM_NONE           0
#define STREAM_RTA            1
#define STREAM_COM_0          2
#define STREAM_COM_1          3


// <<< Use Configuration Wizard in Context Menu >>>

// <e0>I/O Retargeting
// <i>Enable I/O retargeting
#ifndef RETARGETING_ENABLED
 #define RETARGETING_ENABLED  1
#endif

// <o0>STDIN Source  <i>Select the source of standard input
//   <0=> None
//   <1=> Real-Time Agent
//   <3=> COM Port 1
#ifndef STDIN_SRC
 #define STDIN_SRC            1
#endif

// <o0>STDOUT Destination  <i>Select the destination for standard output
//   <0=> None
//   <1=> Real-Time Agent
//   <3=> COM Port 1
#ifndef STDOUT_DST
 #define STDOUT_DST           1
#endif

// <o0>STDERR Destination  <i>Select the destination for standard error
//   <0=> None
//   <1=> Real-Time Agent
//   <3=> COM Port 1
#ifndef STDERR_DST
 #define STDERR_DST           1
#endif

// <o0.0>Enable RTA_printf()
// <i>Enable Real-Time Agent printf, scanf, etc
#ifndef RTA_PRINTF_ENABLED
 #define RTA_PRINTF_ENABLED   1
#endif

// <o0.0>External file operations defined
// <i>Enable if __fopen(), __fclose(), __write(), __read(), __setfpos(), __flushbuf() and __get_flen() are defined
#ifndef FOPEN_ETC_ENABLED
 #define FOPEN_ETC_ENABLED    0
#endif

// </e>
// <<< end of configuration section >>>

#if (RETARGETING_ENABLED)

#include <stdio.h>
#include <string.h>
#include <rt_misc.h>
#include <rt_sys.h>

#pragma import(__use_no_semihosting_swi)

/* Standard IO device handles - arbitrary, but any real filesystem handles must be
   less than 0x8000. */
#define STDIN     0x8001
#define STDOUT    0x8002
#define STDERR    0x8003
#define RTADBGIN  0x8004
#define RTADBGOUT 0x8005

/* Standard IO device name defines. */
const char __stdin_name[]  = "STDIN";
const char __stdout_name[] = "STDOUT";
const char __stderr_name[] = "STDERR";

struct __FILE { int handle; /* Add whatever you need here */ };


#if (FOPEN_ETC_ENABLED)
  #include <file_config.h>    // Header file containing definitions for... 
  /* __fopen, __fclose, __write, __flushbuf, __read, __setfpos, __get_flen */
#endif


/*--------------------------- _ttywrch --------------------------------------*/

void _ttywrch (int ch) {
  RTA_TermPutChar (ch);
}

/*--------------------------- _sys_open -------------------------------------*/

FILEHANDLE _sys_open (const char *name, int openmode) {
  /* Register standard Input Output devices. */
  if (strcmp(name, "STDIN") == 0) {
    #if ((STDIN_SRC == STREAM_COM_1) || (STDIN_SRC == STREAM_RTA))
      return (STDIN);
    #else // ((STDIN_SRC == STREAM_NONE) || (STDIN_SRC == STREAM_COM_0))
      return (-1);
    #endif  
  }
  if (strcmp(name, "STDOUT") == 0) {
    #if ((STDOUT_DST == STREAM_NONE) || (STDOUT_DST == STREAM_COM_0))
      return (-1);
    #elif ((STDOUT_DST == STREAM_COM_1) || (STDOUT_DST == STREAM_RTA))
      return (STDOUT);
    #endif      
  }
  if (strcmp(name, "STDERR") == 0) {      
    #if ((STDERR_DST == STREAM_COM_1) || (STDERR_DST == STREAM_RTA))
      return (STDERR);
    #else // ((STDERR_DST == STREAM_NONE) || (STDERR_DST == STREAM_COM_0))
      return (-1);
    #endif
  }
  
  /* Register Real-Time Agent debug */
  if (strcmp(name, "RTADBGIN") == 0) {
    #if (RTA_PRINTF_ENABLED) 
      return (RTADBGIN);
    #else
      return (-1);
    #endif
  }
  if (strcmp(name, "RTADBGOUT") == 0) {
    #if (RTA_PRINTF_ENABLED) 
      return (RTADBGOUT);
    #else
      return (-1);
    #endif
  }

  #if (FOPEN_ETC_ENABLED)
    return (__fopen (name, openmode));
  #else
    return (0);
  #endif
}

/*--------------------------- _sys_close ------------------------------------*/

int _sys_close (FILEHANDLE fh) {
  if (fh > 0x8000) {
    return (0);
  }
  #if (FOPEN_ETC_ENABLED)
    return (__fclose (fh));
  #else
    return (0);
  #endif
}

/*--------------------------- _sys_write ------------------------------------*/

int _sys_write (FILEHANDLE fh, const unsigned char *buf, unsigned len, int mode) {   
  if (fh == STDOUT) {    
    #if ((STDOUT_DST == STREAM_COM_1) || (STDOUT_DST == STREAM_RTA))
      for (  ; len; len--) {
        #if (STDOUT_DST == STREAM_COM_1)
          sendchar (*buf++);
        #elif (STDOUT_DST == STREAM_RTA)
          RTA_TermPutChar (*buf++);
        #else // Should not happen
          return (-1);
        #endif
      }
      return (0);
    #else // ((STDOUT_DST == STREAM_NONE) || (STDOUT_DST == STREAM_COM_0))
      return (-1);
    #endif 
  }

  if (fh == STDERR) {    
    #if ((STDERR_DST == STREAM_COM_1) || (STDERR_DST == STREAM_RTA))
      for (  ; len; len--) {
        #if (STDERR_DST == STREAM_COM_1)
          sendchar (*buf++);
        #elif (STDERR_DST == STREAM_RTA)      
          RTA_TermPutChar (*buf++);
        #else // Should not happen
          return (-1);
        #endif
      }
      return (0);
    #else // ((STDERR_DST == STREAM_NONE) || (STDERR_DST == STREAM_COM_0))
      return (-1);
    #endif
  }

  if (fh == RTADBGOUT) {
    #if (RTA_PRINTF_ENABLED)
      for (  ; len; len--) {
        RTA_TermPutChar (*buf++);
      }
      return (0);
    #else
      return (-1);
    #endif
  }

  #if (FOPEN_ETC_ENABLED)
    return (__write (fh, buf, len));
  #else
    return (-1);
  #endif
}

/*--------------------------- _sys_read -------------------------------------*/

int _sys_read (FILEHANDLE fh, unsigned char *buf, unsigned len, int mode) {
  if (fh == STDIN) {
    
    #if ((STDIN_SRC == STREAM_COM_1) || (STDIN_SRC == STREAM_RTA))
      /* only read one byte at a time */
      if (len) {
        #if (STDIN_SRC == STREAM_COM_1)
          *buf++ = getkey ();
        #elif (STDIN_SRC == STREAM_RTA)
          *buf++ = RTA_TermGetChar ();
        #else // Should not happen
          return (-1);
        #endif
      }
      return (len-1);
    #else // ((STDIN_SRC == STREAM_NONE) || (STDIN_SRC == STREAM_COM_0))
      return (-1);
    #endif
  }

  if (fh == RTADBGIN) {
    #if (RTA_PRINTF_ENABLED)
      /* only read one byte at a time */
      if (len) {
        *buf++ = RTA_TermGetChar ();
      }
      return (len-1);
    #else
      return (-1);
    #endif      
  }

  #if (FOPEN_ETC_ENABLED)
    return (__read (fh, buf, len));
  #else
    return (-1);
  #endif
}

/*--------------------------- _sys_istty ------------------------------------*/

int _sys_istty (FILEHANDLE fh) {
  if (fh > 0x8000) {
    return (1);
  }
  return (0);
}

/*--------------------------- _sys_seek -------------------------------------*/

int _sys_seek (FILEHANDLE fh, long pos) {
  if (fh > 0x8000) {
    return (-1);
  }
  
  #if (FOPEN_ETC_ENABLED)
    return (__setfpos (fh, pos));
  #else
    return (-1);
  #endif
}

/*--------------------------- _sys_ensure -----------------------------------*/

int _sys_ensure (FILEHANDLE fh) {
  if (fh > 0x8000) {
    return (-1);
  }

  #if (FOPEN_ETC_ENABLED)
    return (__flushbuf (fh));
  #else
    return (-1);
  #endif
}

/*--------------------------- _sys_flen -------------------------------------*/

long _sys_flen (FILEHANDLE fh) {
  if (fh > 0x8000) {
    return (0);
  }

  #if (FOPEN_ETC_ENABLED)
    return (__get_flen (fh));
  #else
    return (0);
  #endif
}

/*--------------------------- _sys_tmpnam -----------------------------------*/

int _sys_tmpnam (char *name, int sig, unsigned maxlen) {
  return (1);
}

/*--------------------------- _sys_command_string ---------------------------*/

char *_sys_command_string (char *cmd, int len) {
  return (cmd);
}

/*--------------------------- _sys_exit -------------------------------------*/

void _sys_exit (int return_code) {
  /* Endless loop. */
  while (1);
}

#endif /* #if (RETARGETING_ENABLED) */

