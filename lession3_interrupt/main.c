#include "msp430fr5739.h"

void main(void) {
  
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT  
  
  P1DIR |= (BIT0);
  P1OUT |= (BIT0);

  // Enable switches
  // P4.0 and P4.1 are configured as switches
  // Port 4 has only two pins
  P4DIR &= ~BIT0;                  // Direction = input
  
  P4OUT |= BIT0;                      // Configure pullup resistor  
  P4REN |= BIT0;                     // Enable pullup resistor
  P4IES |= BIT0;                       // P4.0 Lo/Hi edge interrupt  
  P4IE = BIT0;                         // P4.0 interrupt enabled
  P4IFG = 0;                                // P4 IFG cleared

  while (1) {
    // busy loop
    /*
    volatile int i;
    for (i = 0; i < 20000; i++);
    if (!(P4IN & BIT0)) {
	P1OUT ^= (BIT0);
    }*/
    
    /* Go into low power mode 0, general interrupts enabled */
        __bis_SR_register( LPM0_bits + GIE );
  }
}


void Port_4 (void) __attribute__((interrupt(PORT4_VECTOR)));
void Port_4(void)
{
    if( P4IES & BIT0 ) {
      P1OUT ^= (BIT0);
    }
    
    P4IES ^= BIT0;

    /* Clear the interrupt flag */
    P4IFG = 0;
    
    volatile int i;
    // Wait for some time before continue
    for (i = 0; i < 500; i++);

    /* Uncomment the next line if you want button releases also to trigger.
     * That is, we change the interrupt edge, and Hi-to-Low will
     * trigger the next interrupt.
     */
    // P1IES ^= BUTTON;

    /* This line is still magic to me.  I think it exits low power mode 0
     * so the main program can resume running.
     */
    __bic_SR_register_on_exit( LPM0_bits );
}