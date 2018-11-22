/******************************************************************************/
/* LEDmatrix.C: Functions for displaying text to LED matrix                   */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <LPC21XX.H>                    /* LPC21XX Peripheral Registers */

#define ALL_ROWS   0x07D00404           /* All rows deselect constant */
#define ALL_COLS   0x002930A8           /* All columns deselect constant */
#define NUM_ROWS   8                    /* Number of rows */
#define NUM_COLS   8                    /* Number of columns */
#define TIME1MS    14745                /* Counts 14746-1 @ 14.7456 MHz PCLK */
#define TIME100US  1474                 /* Counts 1475-1 @ 14.7456 MHz PCLK */
#define TIMECYC    TIME100US

//#define TEST

extern const unsigned char font5x7[256][6]; /* Font 5x7 */

unsigned int ROW[8]   = {1<<2, 1<<10, 1<<20, 1<<22, 1<<23, 1<<24, 1<<25, 1<<26};
unsigned int COL[8]   = {1<<3, 1<< 5, 1<< 7, 1<<12, 1<<13, 1<<16, 1<<19, 1<<21};
unsigned int INTEN[9] = {0, TIMECYC/12,  TIMECYC/10,   TIMECYC/8,    TIMECYC/4,
                            TIMECYC/2,   TIMECYC*3/4,  TIMECYC*7/8,  TIMECYC*250/256 };

/* Scrolling direction (-1 = left, 1 = right) */
int scrl_dir = 1;

/* Intensity for display */
char intensity = 7;

/* Memory reserved for virtual screen for displaying on LED matrix
   one unsigned long represents active pins for that row */
unsigned char mem_screen[192];

/* Starting location to display from memory screen */
int disp_start = 0;

/* Length of displaying colums in virtual screen */
int screen_len = 0;

unsigned long ByteToLED (unsigned char *byte, unsigned char bit);

/* Timer0 interrupt function happens twice in one full pwm cycle.
   It first happens on Match0 and then new column value is calculated
   and if pwm is shorter than full cycle pwm current row is deactivated.
   Second time it happens on Match1 then new column walue is selected and 
   row is activated. */
__irq void MatrixInterrupt (void) {
  static long volatile timeval;
  static unsigned char row = 7;
  static unsigned int  next_pwm;
  static unsigned long col_val;

  if (next_pwm < INTEN[8])              /* If requested intensity not maximum */
    IOSET0 = ALL_ROWS;                  /* Deactivate all rows */

  if ((T0IR & 0x03) == 0x01) {          /* If MR0 interrupt occured */

    /* Prepare for next column activation */
    row++;
    row &= 0x07;
  
    /* Prepare columns value for next row */
    col_val = ByteToLED(&mem_screen[disp_start], row);

    /* Calculate length of column active according to needed intensity */
    next_pwm = INTEN[intensity];
  
    T0IR = 1;                           /* Clear interrupt flag */
    VICVectAddr = 0;                    /* Acknowledge Interrupt */
  } 
  else {                                /* If MR1 interrupt occured */
    IOCLR0 = ALL_COLS;                  /* Deactivate all columns */
    IOSET0 = ALL_ROWS;                  /* Deactivate all rows */
    if (next_pwm == 0)  {
      T0MR0  = 100;
    } else {
      T0MR0  = next_pwm;                /* Load the row active time */
      IOSET0 = col_val;                 /* Activate requested columns value */
      IOCLR0 = ROW[row];                /* Activate row */
    }

    if (++timeval == 1000) {            /* Scroll 1 tick every 100 * 100us */
      timeval = 0;
      disp_start += scrl_dir;
      if (disp_start < 0)
        disp_start = screen_len-1;
      else if (disp_start >= screen_len)
        disp_start = 0;
    }

    T0IR = 3;                           /* Clear interrupt flag */
    VICVectAddr = 0;                    /* Acknowledge Interrupt */
  }
}

/* This function converts bit position from 8 bytes in a row to  
   values of columns for LED matrix */
unsigned long ByteToLED (unsigned char *byte, unsigned char bit) {
  char i;
  unsigned long result = 0;
  unsigned char mask   = 1 << bit;

  for (i = 0; i < 8; i++) {
    if ((byte-mem_screen) >= screen_len) byte -= screen_len;
    if (*byte++ & mask) { result |= COL[i]; }
  }

  return (result);
}


/* This function prints string to memory in format to be output 
   on the LED matrix */
void PrintToMem (unsigned char *mem_ptr, unsigned char *string) {
  while (*string) {
    *mem_ptr++ = font5x7[*string][0];
    *mem_ptr++ = font5x7[*string][1];
    *mem_ptr++ = font5x7[*string][2];
    *mem_ptr++ = font5x7[*string][3];
    *mem_ptr++ = font5x7[*string][4];
    *mem_ptr++ = font5x7[*string++][5];
    screen_len += 6;
  }
}


/* This function initializes LED matrix and setups timer interrupt 
   for generating time multiplex for outputing to LED matrix */
void MatrixInit (void) {

  IODIR0 = ALL_COLS | ALL_ROWS;         /* Define all LED matrix pins as Outputs */
  IOCLR0 = ALL_COLS;                    /* Clear all columns */
  IOSET0 = ALL_ROWS;                    /* Disable all rows */

  /* Output text to memory buffer that will be displayed */
  PrintToMem(mem_screen, "Keil SILICA-LPC2103 Demo ");

  #ifdef TEST                           /* Test screen */
    mem_screen[0] = 0xFF;
    mem_screen[1] = 0x7F;
    mem_screen[2] = 0x3F;
    mem_screen[3] = 0x1F;
    mem_screen[4] = 0x0F;
    mem_screen[5] = 0x07;
    mem_screen[6] = 0x03;
    mem_screen[7] = 0x01;
  #endif

  /* Setup timer to generate interrupt for time multiplex of LED matrix */
  T0MR0 = 100;                          /* Initial first match set at dummy value 100 */
  T0MR1 = TIMECYC;                      /* Load constant for time of 1 ms for pwm full cycle */
  T0MCR = 0x19;                         /* MR0 - int; MR1 - int, reset */
  T0TCR = 1;                            /* Timer0 Enable and Run */
  VICVectAddr0 = (unsigned long)MatrixInterrupt; /* set interrupt vector in 0 */
  VICVectCntl0 = 0x20 | 4;              /* use it for Timer 0 Interrupt */
  VICIntEnable = 0x00000010;            /* Enable Timer0 Interrupt */
}
