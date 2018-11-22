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
#include <LPC22xx.H>                    /* LPC21xx definitions               */


OS_TID t_Display;                        /* assigned task id of task: Display */
OS_TID t_Control;                        /* assigned task id of task: Control */



/* GPIO port */
#define P0	0x1
#define P1	0x2
#define P2	0x4
#define P3	0x8
#define P4	0x10
#define P5	0x20
#define P6	0x40
#define P7	0x80
#define P8	0x100
#define P9	0x200
#define P10	0x400
#define P11	0x800
#define P12	0x1000
#define P13	0x2000
#define P14	0x4000
#define P15	0x8000
#define P16	0x10000
#define P17	0x20000
#define P18	0x40000
#define P19	0x80000
#define P20	0x100000
#define P21	0x200000
#define P22	0x400000
#define P23	0x800000
#define P24	0x1000000
#define P25	0x2000000
#define P27	0x8000000
#define P28	0x10000000
#define P29	0x20000000
#define P30	0x40000000
#define P31	0x80000000


#define		LED_CON			P29							
#define		LED_BIT_CON		P19	
#define		LED_OUT_CON		(P10 | P11 | P12 | P13 | P14 | P15 | P16 | P17)
#define		LED_SEG_CON		P18


#define		LED_SEG_A		P17								// A SEG cortrol
#define		LED_SEG_B		P15								// B SEG cortrol
#define		LED_SEG_C		P13								// C SEG cortrol
#define		LED_SEG_D		P11								// D SEG cortrol
#define		LED_SEG_E		P10								// E SEG cortrol
#define		LED_SEG_F		P16								// F SEG cortrol
#define		LED_SEG_G		P14								// G SEG cortrol
#define		LED_SEG_DP		P12								// DP SEG cortrol


/*SEG CODE*/
#define		LED_CHAR_0	LED_SEG_A + LED_SEG_B + LED_SEG_C + LED_SEG_D + LED_SEG_E + LED_SEG_F
																//0
#define		LED_CHAR_1	LED_SEG_B + LED_SEG_C
																//1
#define		LED_CHAR_2	LED_SEG_A + LED_SEG_B + LED_SEG_D + LED_SEG_E + LED_SEG_G
																//2
#define		LED_CHAR_3	LED_SEG_A + LED_SEG_B + LED_SEG_C + LED_SEG_D + LED_SEG_G
																//3
#define		LED_CHAR_4	LED_SEG_B + LED_SEG_C + LED_SEG_F + LED_SEG_G 
																//4
#define		LED_CHAR_5	LED_SEG_A + LED_SEG_C + LED_SEG_D + LED_SEG_F + LED_SEG_G
																//5
#define		LED_CHAR_6	LED_SEG_A + LED_SEG_C + LED_SEG_D + LED_SEG_E + LED_SEG_F + LED_SEG_G
																//6
#define		LED_CHAR_7	LED_SEG_A + LED_SEG_B + LED_SEG_C
																//7	 
#define		LED_CHAR_8	LED_SEG_A + LED_SEG_B + LED_SEG_C + LED_SEG_D + LED_SEG_E+LED_SEG_F+LED_SEG_G
																//8
#define		LED_CHAR_9	LED_SEG_A + LED_SEG_B + LED_SEG_C + LED_SEG_D + LED_SEG_F + LED_SEG_G
																//9
#define		LED_CHAR_A	LED_SEG_A + LED_SEG_B + LED_SEG_C + LED_SEG_E + LED_SEG_F + LED_SEG_G
																//A
#define		LED_CHAR_B	LED_SEG_C + LED_SEG_D + LED_SEG_E + LED_SEG_F + LED_SEG_G
																//B
#define		LED_CHAR_C	LED_SEG_A + LED_SEG_D + LED_SEG_E + LED_SEG_F
																//C
#define		LED_CHAR_D	LED_SEG_B + LED_SEG_C + LED_SEG_D + LED_SEG_E + LED_SEG_G
																//D
#define		LED_CHAR_E	LED_SEG_A + LED_SEG_D + LED_SEG_E + LED_SEG_F + LED_SEG_G
																//E
#define		LED_CHAR_F	LED_SEG_A + LED_SEG_E + LED_SEG_F + LED_SEG_G
																//F
#define		LED_DEC	  LED_SEG_G
																//-

// led control variable

U8 szLedChr[8], szLedIndex, szLedBase, szLedLast,szShowIndex,i ;


// led show variable 0-F
 
const U32 lLedDisp[16] = {	LED_CHAR_0, LED_CHAR_1, LED_CHAR_2, LED_CHAR_3,
								LED_CHAR_4, LED_CHAR_5, LED_CHAR_6, LED_CHAR_7,
								LED_CHAR_8, LED_CHAR_9, LED_CHAR_A, LED_CHAR_B,
								LED_CHAR_C, LED_CHAR_D, LED_CHAR_E, LED_CHAR_F }; 





/*----------------------------------------------------------------------------
 *        Led Display Function
 *---------------------------------------------------------------------------*/

void  led_show( U32 szshowBit,U32  szshowchar)
{
	// Led bit cortrol
	IOSET0 = LED_BIT_CON;
	IOSET0 = LED_OUT_CON;
	IOCLR0 = LED_BIT_CON;

	// Led segment cortrol
	IOSET0 = LED_SEG_CON;
	IOCLR0 = LED_OUT_CON;
	IOSET0 = szshowchar;
	IOCLR0 = LED_SEG_CON;

	// Led bit cortrol
	IOSET0 = LED_BIT_CON;
	IOSET0 = LED_OUT_CON;
	IOCLR0 = szshowBit;
	IOCLR0 = LED_BIT_CON;
}





/*----------------------------------------------------------------------------
 *        Task 1 'Display'
 *---------------------------------------------------------------------------*/
__task  void Display (void) {
  for (;;) {
   
   led_show((0x01 << szLedIndex+10), lLedDisp[szLedChr[szLedIndex]]);	/* display the seg code 0 - F */
   
   os_evt_set (0x0001, t_Control);             /* send event to task 'Control'        */
   
   os_dly_wait (50);                      /* delay 50 clock ticks             */
  }					   
}

/*----------------------------------------------------------------------------
 *        Task 2 'Control '
 *---------------------------------------------------------------------------*/
__task void Control (void) {
  for (;;) {
    
	os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
	
	szLedIndex += 1;
	
	if (szLedIndex > szLedLast)  szLedIndex = szLedBase;		   /* Change display seg code */

  }
}

/*----------------------------------------------------------------------------
 *        Task 3 'init': Initialize
 *---------------------------------------------------------------------------*/
__task void init (void) {

  
  
  IODIR0 |= LED_BIT_CON | LED_SEG_CON | LED_OUT_CON | LED_CON;
 
  IOCLR0 =  LED_BIT_CON | LED_SEG_CON | LED_CON;	

  // init Led value
	szLedLast  = 7;
	szLedBase  = 0;
	szLedIndex = 0;

	for (i=0; i<=szLedLast ; i++) {
		szLedChr[i] = i;	}

  t_Display = os_tsk_create (Display, 1);  /* start task Display                */
  t_Control = os_tsk_create (Control, 2);  /* start task Control                */
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

