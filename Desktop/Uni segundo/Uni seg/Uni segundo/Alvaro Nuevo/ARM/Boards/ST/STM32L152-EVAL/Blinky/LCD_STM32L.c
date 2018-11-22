/******************************************************************************/
/* LCD_STM32L.c: STM32L152 low level LCD Functions                            */
/*               LCD glass 32x4 segments (8 digits Liquid Crystal Display)    */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2009 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <stm32l1xx.h>                  /* STM32L1xx.h definitions            */
#include "LCD.h"

/*--------------------------------------------------------------------- 
  15 segment LCD digit is:

               A
      --  -----------
    X \/  |\   |I  /|
         F| H  |  J |B
          |  \ | /  |
          --G-- --K--
          |   /| \  |
        E |  L |  N |C
          | /  |M  \|
          -----------  .DP   
              D

          bit0 bit1 bit2 bit3
   COM1:   X    I    A     H
   COM2:   F    J    B     G 
   COM3:   E    K    C     L
   COM4:   D    N    DP    M

  Character coding:  unsinged short c;
                     c 0.. 3 -> COM1
                     c 4.. 7 -> COM2
                     c 8..11 -> COM3
                     c12..15 -> COM4

  Example:          'A' = E, F, A, B, C, G, K         -> 0x07D4
                    '0' = E, F, A, B, C, D, L, J      -> 0x1D74   

  ---------------------------------------------------------------------*/ 

unsigned short lcdChar[128] = {
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, //   0..9    unused
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, //  10..19   unused
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, //  20..29   unused
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, //  30..39   unused           
  0x0000, 0x0000, 0xAAAA, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1D74, 0x0460, //  40..47   unused, '0', '1'           
  0x13C4, 0x16C4, 0x06D0, 0x1694, 0x1794, 0x0444, 0x17D4, 0x16D4, 0x0000, 0x0000, //  '2'..'9', 58..59 unused           
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x07D4, 0x9646, 0x1114, 0x9446, 0x1194, //  60..64   unused, 'A'..'E'
  0x0194, 0x1714, 0x07D0, 0x9006, 0x1540, 0x21B0, 0x1110, 0x0578, 0x2558, 0x1554, //  'F'..'O'           
  0x03D4, 0x3554, 0x23D4, 0x1694, 0x8006, 0x1550, 0x0930, 0x2D50, 0x2828, 0x8028, //  'P'..'Y'          
  0x1824, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, //  'Z', 91..96 unused, 'a'..'c'           
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, //  'e'..'m'           
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, //  'n'..'w'           
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};                //  'x'..'z', 123..127 unused           



#define COM0_SEG00_31    0
#define COM0_SEG32_43    1
#define COM1_SEG00_31    2
#define COM1_SEG32_43    3
#define COM2_SEG00_31    4
#define COM2_SEG32_43    5
#define COM3_SEG00_31    6
#define COM3_SEG32_43    7


/*************************** Local functions **********************************/

/*----------------------------------------------------------------------------
  LCD_Init:  Initialize the LCD Peripheral
 *----------------------------------------------------------------------------*/
void wrChar (unsigned int pos, unsigned char c) {
  unsigned int c0, c1, c2, c3;

  c0 =  ((lcdChar[c] >>  0) & 0xF);
  c1 =  ((lcdChar[c] >>  4) & 0xF);
  c2 =  ((lcdChar[c] >>  8) & 0xF);
  c3 =  ((lcdChar[c] >> 12) & 0xF);

  switch (pos) {
    case 0:                                     /* SEG40, SEG41, SEG42, SEG43 */
      LCD->RAM[COM0_SEG32_43] &= ~(0xFul << 8);
      LCD->RAM[COM1_SEG32_43] &= ~(0xFul << 8);
      LCD->RAM[COM2_SEG32_43] &= ~(0xFul << 8);
      LCD->RAM[COM3_SEG32_43] &= ~(0xFul << 8);
  
      LCD->RAM[COM0_SEG32_43] |=  (c0 << 8);
      LCD->RAM[COM1_SEG32_43] |=  (c1 << 8);
      LCD->RAM[COM2_SEG32_43] |=  (c2 << 8);
      LCD->RAM[COM3_SEG32_43] |=  (c3 << 8);
      break;

    case 1:                                     /* SEG36, SEG37, SEG38, SEG39 */
      LCD->RAM[COM0_SEG32_43] &= ~(0xFul << 4);
      LCD->RAM[COM1_SEG32_43] &= ~(0xFul << 4);
      LCD->RAM[COM2_SEG32_43] &= ~(0xFul << 4);
      LCD->RAM[COM3_SEG32_43] &= ~(0xFul << 4);
  
      LCD->RAM[COM0_SEG32_43] |=  ((((c0 >> 2) & 0x03) | ((c0 << 2) & 0x0C)) << 4);
      LCD->RAM[COM1_SEG32_43] |=  ((((c1 >> 2) & 0x03) | ((c1 << 2) & 0x0C)) << 4);
      LCD->RAM[COM2_SEG32_43] |=  ((((c2 >> 2) & 0x03) | ((c2 << 2) & 0x0C)) << 4);
      LCD->RAM[COM3_SEG32_43] |=  ((((c3 >> 2) & 0x03) | ((c3 << 2) & 0x0C)) << 4);
      break;
      
    case 2:                                     /* SEG32, SEG33, SEG34, SEG35 */
      LCD->RAM[COM0_SEG32_43] &= ~(0xFul << 0);
      LCD->RAM[COM1_SEG32_43] &= ~(0xFul << 0);
      LCD->RAM[COM2_SEG32_43] &= ~(0xFul << 0);
      LCD->RAM[COM3_SEG32_43] &= ~(0xFul << 0);
  
      LCD->RAM[COM0_SEG32_43] |=  (c0 << 0);
      LCD->RAM[COM1_SEG32_43] |=  (c1 << 0);
      LCD->RAM[COM2_SEG32_43] |=  (c2 << 0);
      LCD->RAM[COM3_SEG32_43] |=  (c3 << 0);
      break;
      
    case 3:                                    /* SEG28, SEG29, SEG30, SEG31 */
      LCD->RAM[COM0_SEG00_31] &= ~(0xFul << 28);
      LCD->RAM[COM1_SEG00_31] &= ~(0xFul << 28);
      LCD->RAM[COM2_SEG00_31] &= ~(0xFul << 28);
      LCD->RAM[COM3_SEG00_31] &= ~(0xFul << 28);
  
      LCD->RAM[COM0_SEG00_31] |=  (c0 << 28);
      LCD->RAM[COM1_SEG00_31] |=  (c1 << 28);
      LCD->RAM[COM2_SEG00_31] |=  (c2 << 28);
      LCD->RAM[COM3_SEG00_31] |=  (c3 << 28);
      break;
      
    case 4:                                    /* SEG20, SEG21, SEG22, SEG23 */
      LCD->RAM[COM0_SEG00_31] &= ~(0xFul << 20);
      LCD->RAM[COM1_SEG00_31] &= ~(0xFul << 20);
      LCD->RAM[COM2_SEG00_31] &= ~(0xFul << 20);
      LCD->RAM[COM3_SEG00_31] &= ~(0xFul << 20);
  
      LCD->RAM[COM0_SEG00_31] |=  (c0 << 20);
      LCD->RAM[COM1_SEG00_31] |=  (c1 << 20);
      LCD->RAM[COM2_SEG00_31] |=  (c2 << 20);
      LCD->RAM[COM3_SEG00_31] |=  (c3 << 20);
      break;
      
    case 5:                                   /* SEG11, SEG16, SEG18, SEG19 */
      LCD->RAM[COM0_SEG00_31] &= ~((1UL << 11) | (1UL << 16) | (3UL << 18));
      LCD->RAM[COM1_SEG00_31] &= ~((1UL << 11) | (1UL << 16) | (3UL << 18));
      LCD->RAM[COM2_SEG00_31] &= ~((1UL << 11) | (1UL << 16) | (3UL << 18));
      LCD->RAM[COM3_SEG00_31] &= ~((1UL << 11) | (1UL << 16) | (3UL << 18));
  
      LCD->RAM[COM0_SEG00_31] |=  (((c0 & 0x01) << 11) | ((c0 & 0x02) << 15) | ((c0 & 0x0C) << 16));
      LCD->RAM[COM1_SEG00_31] |=  (((c1 & 0x01) << 11) | ((c1 & 0x02) << 15) | ((c1 & 0x0C) << 16));
      LCD->RAM[COM2_SEG00_31] |=  (((c2 & 0x01) << 11) | ((c2 & 0x02) << 15) | ((c2 & 0x0C) << 16));
      LCD->RAM[COM3_SEG00_31] |=  (((c3 & 0x01) << 11) | ((c3 & 0x02) << 15) | ((c3 & 0x0C) << 16));
      break;
      
    case 6:                                   /* SEG4, SEG5, SEG6, SEG10 */
      LCD->RAM[COM0_SEG00_31] &= ~((7UL <<  4) | (1UL << 10));
      LCD->RAM[COM1_SEG00_31] &= ~((7UL <<  4) | (1UL << 10));
      LCD->RAM[COM2_SEG00_31] &= ~((7UL <<  4) | (1UL << 10));
      LCD->RAM[COM3_SEG00_31] &= ~((7UL <<  4) | (1UL << 10));
  
      LCD->RAM[COM0_SEG00_31] |=  (((c0 & 0x07) << 4) | ((c0 & 0x08) << 7));
      LCD->RAM[COM1_SEG00_31] |=  (((c1 & 0x07) << 4) | ((c1 & 0x08) << 7));
      LCD->RAM[COM2_SEG00_31] |=  (((c2 & 0x07) << 4) | ((c2 & 0x08) << 7));
      LCD->RAM[COM3_SEG00_31] |=  (((c3 & 0x07) << 4) | ((c3 & 0x08) << 7));
      break;
      
    case 7:                                   /* SEG0, SEG1, SEG2, SEG3 */
      LCD->RAM[COM0_SEG00_31] &= ~(0xFul << 0);
      LCD->RAM[COM1_SEG00_31] &= ~(0xFul << 0);
      LCD->RAM[COM2_SEG00_31] &= ~(0xFul << 0);
      LCD->RAM[COM3_SEG00_31] &= ~(0xFul << 0);
  
      LCD->RAM[COM0_SEG00_31] |=  (c0 << 0);
      LCD->RAM[COM1_SEG00_31] |=  (c1 << 0);
      LCD->RAM[COM2_SEG00_31] |=  (c2 << 0);
      LCD->RAM[COM3_SEG00_31] |=  (c3 << 0);
      break;
  }
} 


/************************ Exported functions **********************************/

/*----------------------------------------------------------------------------
  LCD_Init:  Initialize the LCD Peripheral
 *----------------------------------------------------------------------------*/
void LCD_Init (void) { 

  /* Configure used GPIOs for LCD */
  RCC->AHBENR  |= (1UL <<  0) |             /* enable GPIOA clock             */
                  (1UL <<  1) |             /* enable GPIOB clock             */
                  (1UL <<  2) |             /* enable GPIOC clock             */
                  (1UL <<  3) |             /* enable GPIOD clock             */
                  (1UL <<  4) ;             /* enable GPIOE clock             */


  /* Configure PA.08 to LCD COM0 (AF11),
               PA.09 to LCD COM1 (AF11), 
               PA.10 to LCD COM2 (AF11) */
  GPIOA->AFR[1] &= ~((15UL << 4* 0) | (15UL << 4* 1) | (15UL << 4* 2) );   
  GPIOA->AFR[1] |=  ((11UL << 4* 0) | (11UL << 4* 1) | (11UL << 4* 2) );      
  GPIOA->MODER  &= ~(( 3UL << 2* 8) | ( 3UL << 2* 9) | ( 3UL << 2*10) );   
  GPIOA->MODER  |=  (( 2UL << 2* 8) | ( 2UL << 2* 9) | ( 2UL << 2*10) );

  /* Configure PB.09 to LCD COM3  */
  GPIOB->AFR[1] &= ~((15UL << 4* 1) );
  GPIOB->AFR[1] |=  ((11UL << 4* 1) );
  GPIOB->MODER  &= ~(( 3UL << 2* 9) );
  GPIOB->MODER  |=  (( 2UL << 2* 9) );

  /* Configure PA.01 to LCD SEG0 (AF11), 
               PA.02 to LCD SEG1 (AF11), 
               PA.03 to LCD SEG2 (AF11), 
               PA.06 to LCD SEG3 (AF11), 
               PA.07 to LCD SEG4 (AF11)  */
  GPIOA->AFR[0] &= ~((15UL << 4* 1) | (15UL << 4* 2) | (15UL << 4* 3) | (15UL << 4* 6) | (15UL << 4* 7) );   
  GPIOA->AFR[0] |=  ((11UL << 4* 1) | (11UL << 4* 2) | (11UL << 4* 3) | (11UL << 4* 6) | (11UL << 4* 7) );      
  GPIOA->MODER  &= ~(( 3UL << 2* 1) | ( 3UL << 2* 2) | ( 3UL << 2* 3) | ( 3UL << 2* 6) | ( 3UL << 2* 7) );   
  GPIOA->MODER  |=  (( 2UL << 2* 1) | ( 2UL << 2* 2) | ( 2UL << 2* 3) | ( 2UL << 2* 6) | ( 2UL << 2* 7) );

  /* Configure PB.00 to LCD SEG5 (AF11),
               PB.01 to LCD SEG6 (AF11)  */
  GPIOB->AFR[0] &= ~((15UL << 4* 0) | (15UL << 4* 1) );   
  GPIOB->AFR[0] |=  ((11UL << 4* 0) | (11UL << 4* 1) );   	
  GPIOB->MODER  &= ~(( 3UL << 2* 0) | ( 3UL << 2* 1) );   
  GPIOB->MODER  |=  (( 2UL << 2* 0) | ( 2UL << 2* 1) );

  /* Connect PB.10 to LCD SEG10 (AF11),
             PB.11 to LCD SEG11 (AF11),
             PB.08 to LCD SEG16 (AF11) */
  GPIOB->AFR[1] &= ~((15UL << 4* 2) | (15UL << 4* 3) | (15UL << 4* 0) );   
  GPIOB->AFR[1] |=  ((11UL << 4* 2) | (11UL << 4* 3) | (11UL << 4* 0) );       
  GPIOB->MODER  &= ~(( 3UL << 2*10) | ( 3UL << 2*11) | ( 3UL << 2* 8) );   
  GPIOB->MODER  |=  (( 2UL << 2*10) | ( 2UL << 2*11) | ( 2UL << 2* 8) );  

  /* Connect PC.00 to LCD SEG18 (AF11),
             PC.01 to LCD SEG19 (AF11),
             PC.02 to LCD SEG20 (AF11),
             PC.03 to LCD SEG21 (AF11),
             PC.04 to LCD SEG22 (AF11),
             PC.05 to LCD SEG23 (AF11) */
  GPIOC->AFR[0] &= ~((15UL << 4* 0) | (15UL << 4* 1) | (15UL << 4* 2) | (15UL << 4* 3) | (15UL << 4* 4) | (15UL << 4* 5) );   
  GPIOC->AFR[0] |=  ((11UL << 4* 0) | (11UL << 4* 1) | (11UL << 4* 2) | (11UL << 4* 3) | (11UL << 4* 4) | (11UL << 4* 5) );       
  GPIOC->MODER  &= ~(( 3UL << 2* 0) | ( 3UL << 2* 1) | ( 3UL << 2* 2) | ( 3UL << 2* 3) | ( 3UL << 2* 4) | ( 3UL << 2* 5) );   
  GPIOC->MODER  |=  (( 2UL << 2* 0) | ( 2UL << 2* 1) | ( 2UL << 2* 2) | ( 2UL << 2* 3) | ( 2UL << 2* 4) | ( 2UL << 2* 5) );

  /* Connect PD.08 to LCD SEG28 (AF11),
             PD.09 to LCD SEG29 (AF11),
             PD.10 to LCD SEG30 (AF11),
             PD.11 to LCD SEG31 (AF11),
             PD.12 to LCD SEG32 (AF11),
             PD.13 to LCD SEG33 (AF11),
             PD.14 to LCD SEG34 (AF11),
             PD.15 to LCD SEG35 (AF11) */
  GPIOD->AFR[1] &= ~((15UL << 4* 0) | (15UL << 4* 1) | (15UL << 4* 2) | (15UL << 4* 3) | (15UL << 4* 4) | (15UL << 4* 5) | (15UL << 4* 6) | (15UL << 4* 7) );   
  GPIOD->AFR[1] |=  ((11UL << 4* 0) | (11UL << 4* 1) | (11UL << 4* 2) | (11UL << 4* 3) | (11UL << 4* 4) | (11UL << 4* 5) | (11UL << 4* 6) | (11UL << 4* 7) );        
  GPIOD->MODER  &= ~(( 3UL << 2* 8) | ( 3UL << 2* 9) | ( 3UL << 2*10) | ( 3UL << 2*11) | ( 3UL << 2*12) | ( 3UL << 2*13) | ( 3UL << 2*14) | ( 3UL << 2*15) );   
  GPIOD->MODER  |=  (( 2UL << 2* 8) | ( 2UL << 2* 9) | ( 2UL << 2*10) | ( 2UL << 2*11) | ( 2UL << 2*12) | ( 2UL << 2*13) | ( 2UL << 2*14) | ( 2UL << 2*15) );

  /* Connect PE.00 to LCD SEG36 (AF11),
             PE.01 to LCD SEG37 (AF11),
             PE.02 to LCD SEG38 (AF11),
             PE.03 to LCD SEG39 (AF11) */
  GPIOE->AFR[0] &= ~((15UL << 4* 0) | (15UL << 4* 1) | (15UL << 4* 2) | (15UL << 4* 3) );   
  GPIOE->AFR[0] |=  ((11UL << 4* 0) | (11UL << 4* 1) | (11UL << 4* 2) | (11UL << 4* 3) );        
  GPIOE->MODER  &= ~(( 3UL << 2* 0) | ( 3UL << 2* 1) | ( 3UL << 2* 2) | ( 3UL << 2* 3) );   
  GPIOE->MODER  |=  (( 2UL << 2* 0) | ( 2UL << 2* 1) | ( 2UL << 2* 2) | ( 2UL << 2* 3) ); 

  /* Connect PC.10 to LCD SEG40 (AF11),
             PC.11 to LCD SEG41 (AF11),
             PC.12 to LCD SEG42 (AF11) */
  GPIOC->AFR[1] &= ~((15UL << 4* 2) | (15UL << 4* 3) | (15UL << 4* 4) );   
  GPIOC->AFR[1] |=  ((11UL << 4* 2) | (11UL << 4* 3) | (11UL << 4* 4) );        
  GPIOC->MODER  &= ~(( 3UL << 2*10) | ( 3UL << 2*11) | ( 3UL << 2*12) );   
  GPIOC->MODER  |=  (( 2UL << 2*10) | ( 2UL << 2*11) | ( 2UL << 2*12) );   

  /* Connect PD.02 to LCD SEG43 (AF11) */
  GPIOD->AFR[0] &= ~((15UL << 4* 2) );   
  GPIOD->AFR[0] |=  ((11UL << 4* 2) );        
  GPIOD->MODER  &= ~(( 3UL << 2* 2) );   
  GPIOD->MODER  |=  (( 2UL << 2* 2) );  


  /* Configure LCD */
  RCC->APB1ENR  |= (1UL <<  9) ;             /* enable LCD clock             */

  LCD->CR   = (( 2UL <<  5) |        /* Bias  1/3  */
               ( 3UL <<  2) |        /* Duty  1/4  */
               ( 0UL <<  1) );       /* VSEL  Internal source (voltage step-up converter) */

  LCD->FCR  = (( 3UL << 22) |        /* Prescaler         ck_ps  = LCDCLK/8 */
               ( 0UL << 18) |        /* Divider           ck_div = ck_ps/16 */
               ( 7UL << 10) |        /* Contrast Control  3.3 V             */
               ( 2UL <<  4) );       /* Puls ON Duration  488 us            */     
  while (!(LCD->SR & (1UL << 5)));   /* wait until LCD Frame Control Register synchronized */

  LCD->CR   |= (1UL << 0);           /* enable LCD */         
  while (!(LCD->SR & (1UL << 0)));   /* wait until LCD Controller is enabled */
  while (!(LCD->SR & (1UL << 4)));   /* wait until LCD Booster is ready      */

}

/*----------------------------------------------------------------------------
  LCD_Clear:  Clear the LCD
 *----------------------------------------------------------------------------*/
void LCD_Clear   (void){

  while ( (LCD->SR & (1UL << 2)));   /* wait until LCD RAM update finish      */

  LCD->RAM[COM0_SEG00_31] = 0;
  LCD->RAM[COM0_SEG32_43] = 0;
  LCD->RAM[COM1_SEG00_31] = 0;
  LCD->RAM[COM1_SEG32_43] = 0;
  LCD->RAM[COM2_SEG00_31] = 0;
  LCD->RAM[COM2_SEG32_43] = 0;
  LCD->RAM[COM3_SEG00_31] = 0;
  LCD->RAM[COM3_SEG32_43] = 0;

  LCD->SR  = (1UL << 2);             /* set Update Request                    */
}

/*----------------------------------------------------------------------------
  LCD_DisplayChar:  Dispaly a Character
 *----------------------------------------------------------------------------*/
void LCD_DisplayChar     (unsigned int pos, unsigned char  c) { 

  while ( (LCD->SR & (1UL << 2)));   /* wait until LCD RAM update finish      */

  wrChar (pos, c);                   /* wtite character to LCD RAM            */

  LCD->SR  = (1UL << 2);             /* set Update Request                    */
}

/*----------------------------------------------------------------------------
  LCD_DisplayString:  Dispaly a String
 *----------------------------------------------------------------------------*/
void LCD_DisplayString   (unsigned int pos, unsigned char *s){
  unsigned int i = pos;

  while ( (LCD->SR & (1UL << 2)));   /* wait until LCD RAM update finish      */

  while ((*s != 0) & (i < 8)) {
    wrChar (i++, *s++);              /* write the string */
  }

  LCD->SR  = (1UL << 2);             /* set Update Request                    */
}

/******************************************************************************/
