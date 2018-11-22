/******************************************************************************/
/* MAIN.C: Wave Player                                                        */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <LPC213x.H>                       /* LPC21xx definitions  */
#include <stdio.h>                         /* standard I/O .h-file */
#include <math.h>

extern int getkey(void);

#include "modes.h"
extern enum modes mode;

#include "riffwave.h"
static const unsigned char alignment[][5] = {
  {"ERR"},{"BYTE"},{"WORD"},{"ERR"},{"DWORD"}
};

extern void init_serial (void);            /* Initialize Serial Interface  */
extern void init_timer (void);             /* 8kHz Timer Interrupt Routine */

extern unsigned long timeval;
extern unsigned char print_info;
extern struct wav_hdr* p_wav_hdr;
extern unsigned int blockindex;

signed int volume;

void get_input(void) {                     /* Input processing function */
  unsigned int val;
  unsigned int key;

  key = getkey();
  switch(key){
    case 'n':
    case 'N':
      mode = NEXT;                         /* Next track */
      break;
    case 'p':
    case 'P':
      mode = PREV;                         /* Previous track */
      break;
  }

  AD0CR |= 0x01000000;                     /* Start A/D Conversion */
  do {
    val = AD0DR;                           /* Read A/D Data Register */
  } while ((val & 0x80000000) == 0);       /* Wait for end of A/D Conversion */
  AD0CR &= ~0x01000000;                    /* Stop A/D Conversion */
  volume = ((val >> 1) & 0x7FE0);          /* Extract AD01 Value */
}

int main (void) {
  unsigned int alignidx;

  IODIR1  = 0x00FF0000;                    /* P1.16..23 defined as Outputs  */
  AD0CR   = 0x00200402;                    /* Setup A/D: 10-bit AIN0 @ 3MHz */
  PINSEL1 = 0x01080000;                    /* Enable ADC&DAC Pins */

  get_input();
  
  init_serial();                           /* Initialize Serial Interface */
  init_timer ();
  
  printf ("\nMCB2130 WAVE AUDIO PLAYER\n\n");

  while (1) {                              /* Loop forever */
    if (print_info) {
      alignidx = p_wav_hdr->sample_size;
      if (alignidx > 4) alignidx = 0; 
      print_info = 0;
      printf ("Playing Block %01d\n", blockindex);
      printf ("File information:\n");
      printf ("=================\n");
      printf ("File Size:        : %d\n",      p_wav_hdr->length);
      printf ("Format identifier : %01d\n",    p_wav_hdr->format);
      printf ("Number of Channels: %01d\n",    p_wav_hdr->channels);
      printf ("Sampling Rate     : %05d Hz\n", p_wav_hdr->sample_rate);
      printf ("Bytes per Second  : %05d\n",    p_wav_hdr->bytes_per_second);
      printf ("Sample Alignment  : %s\n",      alignment[alignidx]);
      printf ("Sample resolution : %02d\n\n",  p_wav_hdr->precision);
    }
    get_input();
  }
}
