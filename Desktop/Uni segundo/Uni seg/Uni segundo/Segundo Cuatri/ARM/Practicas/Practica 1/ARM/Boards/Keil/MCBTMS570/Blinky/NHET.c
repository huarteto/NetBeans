#include <TMS570.H>


void NHET_Init(void) {

  // NHET1  drives one LED
  // NHET24 drives the solenoid 1 output
  // NHET25 drives the solenoid 2 output
  // NHET26 drives the solenoid 3 output
  // NHET27 drives the solenoid 4 output
  // NHET28 drives the relay
  // NHET30 drives the speaker
  // NHET31 drives one LED

  // bring output register into a defined state before direction is set
  nhetREG->CCDCLR = 0x80000002; // set pins for both LEDs and the speaker to low 
//nhetREG->CCDSET = 0x80000002; // set pins for both LEDs and the speaker to low 
  // select direction (output)
  nhetREG->CCDIR |= 0x80000002; // configure pins for both LEDs and the speaker as output
}
