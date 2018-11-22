/******************************************************************************/
/* T0INT.C: Main Interrupt Routine                                            */
/* Author:  Matthias Hertel                                                   */
/* Date:    21-Oct-2004                                                       */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <LPC213x.H>           /* LPC21xx definitions  */
#include "modes.h"
#include "riffwave.h"

#define T0_RlFreq  1874        /* Timer 0 Reload Frequency in Periph. Clocks  */

/* Data for Record&Block handling */
#define FIRSTBLOCK 1           /* 1st free block - application is in Block 0  */
#define LASTBLOCK  7           /* last block                                  */
#define BLOCKSIZE  0x10000L    /* Size of each block - 64KB                   */
#define SIZE_OF_BOOTLOADER 0x3000  /* Size of Bootloader in last flash Sector */

enum modes mode;               /* Keeps the state that the player is in       */
                                                                                
unsigned int timeval = 0;      /* Global timer - overflows every 10 sec       */
signed short *p_record;        /* pointer to the wave sample                  */
unsigned int eot;              /* end of track                                */
unsigned int blockindex;       /* current Block                               */
extern signed int volume;      /* Volume - Range: 0-1023;                     */

unsigned char print_info = 0;  /* flag for printing track information         */

struct wav_hdr* p_wav_hdr;     /* Wave file header                            */


/* 
 * Timer Counter 0 Interrupt executes in 8kHz frequency 
 */

__irq void tc0 (void) {
  int sample;
  static unsigned short DA_val = 0;

  if (timeval++ >= 40000l) {   /* 5 sec frames */
    timeval =0;               
    mode = NEXT;
  }
  switch (mode) {
    /* State: Playing */
    case PLAY:
      sample = *p_record++;
      sample *= volume;
      DA_val = ((sample >> 15) + 0x8000) & 0xFFFF;
      //DA_val |= 0x10000;     /* Bias */
      DACR = DA_val;
      if (p_record >= (short *)eot) {
        IOCLR1 = 0x00FF0000;   /* Turn off LEDs */
        IOSET1 = 1 << 16;      /* Turn on LED */            
        mode = PAUSE;          /* Next time playing is paused */
      }
      break;
    /* State: Pause - wait until interval counter overflows */
    case PAUSE:
      if (timeval == 0)
        mode = NEXT;           /* After 10 seconds switch to next block */
      break;
    /* State: Next Track */
    case NEXT:
      if (blockindex < LASTBLOCK) {
        blockindex++; 
      } else { 
        blockindex = FIRSTBLOCK;   /* goto first block when last was played */
      }  
    /* State: Previous Track */
    case PREV:
      if (mode != NEXT) {
        if (blockindex > FIRSTBLOCK) { 
          blockindex--;
        } else {
          blockindex = LASTBLOCK;  /* goto last block when first was played */
        }
      }
      {
        unsigned int blockaddress = (blockindex * BLOCKSIZE);    /* absolute memory address of the block */
        p_wav_hdr = (void*) blockaddress;                        /* pointer to Wave/RIFF structure */
        p_record = (signed short*)(void*) blockaddress + 0x24;   /* sample pointer after the wave header */
        
        if (p_wav_hdr->length < BLOCKSIZE){    /* Get the end of file from */
          eot = (unsigned int)(blockaddress + p_wav_hdr->length);  /* Header */
        } else {
          eot = (unsigned int)(blockaddress + BLOCKSIZE) - 1;      /* Block */
        } 
        if (blockindex == LASTBLOCK)           /* skip bootloader */
          eot -= SIZE_OF_BOOTLOADER;
      }

      IOCLR1 = 0x00FF0000;                           /* Turn off LEDs */
      IOSET1 = (1 << (blockindex + 16)) & 0xFF0000;  /* Turn on LED */

      print_info = 1;                          /* print info is on */
      mode = PLAY;                             /* Next time start playing */
      break;
  }
  T0IR        = 1;                             /* Clear interrupt flag  */
  VICVectAddr = 0;                             /* Acknowledge Interrupt */
  
}


/* Setup the Timer Counter 0 Interrupt */
void init_timer (void) {
  T0MR0 = T0_RlFreq;                           /* 0.125uSec = 1875-1 counts   */
  T0MCR = 3;                                   /* Interrupt and Reset on MR0  */
  T0TCR = 1;                                   /* Timer0 Enable               */
  VICVectAddr0 = (unsigned long)tc0;           /* set interrupt vector in 0   */
  VICVectCntl0 = 0x20 | 4;                     /* use it for Timer0 Interrupt */
  VICIntEnable = 0x00000010;                   /* Enable Timer0 Interrupt     */
  mode = NEXT;
  blockindex = FIRSTBLOCK - 1l;
}
