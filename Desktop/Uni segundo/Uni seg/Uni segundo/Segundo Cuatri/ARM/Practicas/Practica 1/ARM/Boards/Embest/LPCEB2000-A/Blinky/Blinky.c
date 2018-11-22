#define	LPCEB2000_I

/*********************************************************************************************
* File£º	spi_test.c
* Author:	Embest w.h.xie
* Desc£º	test spi bus
* History:	
*********************************************************************************************/

/*------------------------------------------------------------------------------------------*/
/*								include files												*/
/*------------------------------------------------------------------------------------------*/

#include   "target.h"


// led control variable

char szLedChr[8], szLedIndex, szLedBase, szLedLast,szShowIndex,i ;


// led show variable 0-F
 
const int lLedDisp[16] = {	LED_CHAR_0, LED_CHAR_1, LED_CHAR_2, LED_CHAR_3,
								LED_CHAR_4, LED_CHAR_5, LED_CHAR_6, LED_CHAR_7,
								LED_CHAR_8, LED_CHAR_9, LED_CHAR_A, LED_CHAR_B,
								LED_CHAR_C, LED_CHAR_D, LED_CHAR_E, LED_CHAR_F }; 



/*********************************************************************************************
* name:		time_dly
* func:		display code
* para:		dly			--in, delay value
* ret:		none
* modify:
* comment:		
*********************************************************************************************/
void  time_dly(int dly)
{  
	int32  i;

    szLedIndex += 1;
	
	if (szLedIndex > szLedLast)  szLedIndex = szLedBase;		   /* Change display seg code */


	for(; dly>0; dly--) 
		for(i=0; i<500; i++);
}

/*----------------------------------------------------------------------------
 *        Led Display Function
 *---------------------------------------------------------------------------*/

void  led_show( int szshowBit,int  szshowchar)
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



/*********************************************************************************************
* name:		Main
* func:		main fun
* para:		none
* ret:		none
* modify:
* comment:	
*********************************************************************************************/

int  main(void)
{
	 
  IODIR0 |= LED_BIT_CON | LED_SEG_CON | LED_OUT_CON | LED_CON;
 
  IOCLR0 =  LED_BIT_CON | LED_SEG_CON | LED_CON;	

  // init Led value
	szLedLast  = 7;
	szLedBase  = 0;
	szLedIndex = 0;

	for (i=0; i<=szLedLast ; i++) {
		szLedChr[i] = i;	}

	while(1) {
    
    time_dly(10);
	
	led_show((0x01 << szLedIndex+10), lLedDisp[szLedChr[szLedIndex]]);

	}

}






