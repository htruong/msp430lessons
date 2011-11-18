#include "msp430fr5739.h"

void main(void) {
  
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT  
  
  P1DIR |= (BIT0);
  P1OUT |= (BIT0);

  // Enable switches
  // P4.0 and P4.1 are configured as switches
  // Port 4 has only two pins    
  P4DIR &= ~(BIT0 + BIT1);                  // Direction = input
  P4OUT |= BIT0 +BIT1;                      // Configure pullup resistor  
  /*
  P4REN |= BIT0 + BIT1;                     // Enable pullup resistor
  P4IES &= ~(BIT0+BIT1);                    // P4.0 Lo/Hi edge interrupt  
  P4IE = BIT0+BIT1;                         // P4.0 interrupt enabled
  P4IFG = 0;                                // P4 IFG cleared
  */


  while (1) {
    // busy loop
    
    volatile int i;
    for (i = 0; i < 20000; i++);
    
    if ((P4IN & BIT0)) {
	P1OUT ^= (BIT0);
    }
  }
}
