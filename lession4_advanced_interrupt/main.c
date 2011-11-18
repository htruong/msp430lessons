#include "msp430fr5739.h"
#include <legacymsp430.h>

void EnableSwitches(void)
{
  P4IFG = 0;                                // P4 IFG cleared  
  P4IE = BIT0;                         // P4.0 interrupt enabled
}

void StartDebounceTimer(unsigned char delay)
{  
  // default delay = 0
  // Debounce time = 1500* 1/8000 = ~200ms
  TA1CCTL0 = CCIE;                          // TACCR0 interrupt enabled
  if(delay)
  	TA1CCR0 = 750;
  else
    TA1CCR0 = 300;
  TA1CTL = TASSEL_1 + MC_1;                 // ACLK, up mode    
}

void DisableSwitches(void)
{
  // disable switches
  P4IFG = 0;                                // P4 IFG cleared    
  P4IE &= ~BIT0;                     // P4.0 interrupt disabled
}

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
    /* Go into low power mode 0, general interrupts enabled */
        __bis_SR_register( LPM3_bits + GIE);
  }
}

interrupt(PORT4_VECTOR) Port_4(void)
{
    /* 
    if( P4IES & BIT0 ) {
      P1OUT ^= (BIT0);
    }
    */
    
    P4IES ^= BIT0;

    /* Clear the interrupt flag */
    P4IFG = 0;

    DisableSwitches();
    StartDebounceTimer(0);
    
    /* Uncomment the next line if you want button releases also to trigger.
     * That is, we change the interrupt edge, and Hi-to-Low will
     * trigger the next interrupt.
     */
    // P1IES ^= BUTTON;

    /* This line is still magic to me.  I think it exits low power mode 0
     * so the main program can resume running.
     */
    __bic_SR_register_on_exit( LPM3_bits );
}


interrupt(TIMER1_A0_VECTOR) Timer1_A0_ISR(void)
{
  TA1CCTL0 = 0;
  TA1CTL = 0;
  
  if ((P4IN & BIT0) == 0) {
    P1OUT ^= BIT0;
  } 
  
  EnableSwitches(); 
}
