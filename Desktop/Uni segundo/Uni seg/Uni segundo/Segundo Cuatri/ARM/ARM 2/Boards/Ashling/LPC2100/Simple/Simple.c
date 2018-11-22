/***************************************************************************
       Module: simple.c
  Description: simple demo program for Netchip eval board
Version     Date           Initials    Description
1.0.5-M     27-Jan-03      ASH         Updated for R1 of EVBA7 board
****************************************************************************/

// LPC2100 definitions
#include "LPC2100_addrs.h"

// write address to register macro
#define REG(addr) (*(volatile unsigned long *)(addr))

// local functions
void OutputTo7SegDisplay(unsigned char ucHex_Val);
void InitialiseI2C(void);
void ClearD3D4D5D6(void);
void SetD3D4D5D6(unsigned char ucBinVal);


// program main

int main() {
  unsigned char i;
  volatile unsigned long * pulGpioReg;
  unsigned long x;

  InitialiseI2C();

  // loop forever,
  while (1) {

    // Set LEDs direction...
    pulGpioReg = (volatile unsigned long *)0xE0028008;
    *pulGpioReg = 0xF0;

    // write to 7-segment LED (via I2C) in HEX and equivalent number (in binary) to D3,D4,D6,D7 
    for (i = 0; i < 16; i ++)
    {
      // delay
      for(x = 0; x < 10000; x++);
  
      // write to 7-Seg
      if (i <=9) 
        OutputTo7SegDisplay( '0' + i);
      else
        OutputTo7SegDisplay( 'a' + (i-10));

      // clear LEDs
      ClearD3D4D5D6();

      // write LEDs
      SetD3D4D5D6(i);
         
      // delay
      for(x = 0; x < 500000; x++);
    }
  }
       
  // never get here
  // return 0;
}
                  


// this function Initialises I2C 

void InitialiseI2C(void) {

  REG(I2C_I2CONCLR) = 0xFF; 

  //  Set pinouts as scl and sda
  REG(PCB_PINSEL0)  = 0x50; 

  REG(I2C_I2CONSET) = 0x40; 
  
  REG(I2C_I2CONSET) = 0x64; 

  REG(I2C_I2DAT) = 0x42; 

  REG(I2C_I2CONCLR) = 0x08;
                                                       
  REG(I2C_I2CONCLR) = 0x20; 
}


// this function outputs the value to EVBA7 7-segment display via the I2C bus

void OutputTo7SegDisplay(unsigned char ucHexVal) {

  switch(ucHexVal) { 
    default:
    case '0' : REG(I2C_I2DAT) = 0xC0;          // 0
               break;
    case '1' : REG(I2C_I2DAT) = 0xF9;          // 1
               break;
    case '2' : REG(I2C_I2DAT) = 0xA4;          // 2
               break;
    case '3' : REG(I2C_I2DAT) = 0xB0;          // 3
               break;
    case '4' : REG(I2C_I2DAT) = 0x99;          // 4
               break;
    case '5' : REG(I2C_I2DAT) = 0x92;          // 5
               break;
    case '6' : REG(I2C_I2DAT) = 0x82;          // 6
               break;
    case '7' : REG(I2C_I2DAT) = 0xF8;          // 7
               break;
    case '8' : REG(I2C_I2DAT) = 0x80;          // 8
               break;
    case '9' : REG(I2C_I2DAT) = 0x90;          // 9
               break;
    case 'a' : REG(I2C_I2DAT) = 0x88;          // A
               break;
    case 'b' : REG(I2C_I2DAT) = 0x83;          // B
               break;
    case 'c' : REG(I2C_I2DAT) = 0xC6;          // C
               break;
    case 'd' : REG(I2C_I2DAT) = 0xA1;          // D
               break;
    case 'e' : REG(I2C_I2DAT) = 0x86;          // E
               break;
    case 'f' : REG(I2C_I2DAT) = 0x8E;          // F
               break;
  }
  REG(I2C_I2CONCLR) = 0x08; 
}


// clear D3, D4, D5, D6 on EVBA7
void ClearD3D4D5D6(void) {
  volatile unsigned long * pulGpioReg;

  pulGpioReg = (volatile unsigned long *)0xE002800C;
  *pulGpioReg = 0xF0;
}

// Set D3, D4, D5, D6 on EVBA7   to binary value
void SetD3D4D5D6(unsigned char ucBinVal) {
  volatile unsigned long * pulGpioReg;

  pulGpioReg = (volatile unsigned long *)0xE0028004;
  switch (ucBinVal) {
    default:
    case 0:
      ClearD3D4D5D6();
      break;
    case 1:
      *pulGpioReg = 0x10;
      break;
    case 2:
      *pulGpioReg = 0x20;
      break;
    case 3:
      *pulGpioReg = 0x30;
      break;
    case 4:
      *pulGpioReg = 0x40;
      break;
    case 5:
      *pulGpioReg = 0x50;
      break;
    case 6:
      *pulGpioReg = 0x60;
      break;
    case 7:
      *pulGpioReg = 0x70;
      break;
    case 8:
      *pulGpioReg = 0x80;
      break;
    case 9:
      *pulGpioReg = 0x90;
      break;
    case 10:
      *pulGpioReg = 0xA0;
      break;
    case 11:
      *pulGpioReg = 0xB0;
      break;
    case 12:
      *pulGpioReg = 0xC0;
      break;
    case 13:
      *pulGpioReg = 0xD0;
      break;
    case 14:
      *pulGpioReg = 0xE0;
      break;
    case 15:
      *pulGpioReg = 0xF0;
      break;
  }  // end switch   
}
