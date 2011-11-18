#include <inttypes.h>
#include "msp430fr5739.h"
#include "legacymsp430.h"

#define DEBUG_CMD
#include "debug.h"

unsigned char TXData;
unsigned char TXByteCtr;

// These global variables are used in the ISRs
void systemInit(void)
{
  // Startup clock system in max. DCO setting ~8MHz
  // This value is closer to 10MHz on untrimmed parts
  CSCTL0_H = 0xA5;                          // Unlock register
  CSCTL1 |= DCOFSEL0 + DCOFSEL1;            // Set max. DCO setting
  CSCTL2 = SELA_1 + SELS_3 + SELM_3;        // set ACLK = vlo; MCLK = DCO
  CSCTL3 = DIVA_0 + DIVS_0 + DIVM_0;        // set all dividers
  CSCTL0_H = 0x01;                          // Lock Register
}


void debug_command(char* str) {
  unsigned int i;
  
  debug_puts("Program got command: ");
  debug_puts(str);
  debug_puts("\r\n");

  if (str[0] == 's') {
  }
}


//#pragma vector = USCI_B0_VECTOR
interrupt(USCI_B0_VECTOR) USCI_B0_ISR(void)
{
  int t = UCB0IV;
  debug_puts("Got UCB0IV=");
  debug_puti(t,16);
  debug_puts("\r\n");

  switch(t) {
    case 0x00: // Vector 0: No interrupts
      break;
    case 0x02: // Vector 2: ALIFG
      break;
    case 0x04: // Vector 4: NACKIFG
      
      break;
    case 0x06: // Vector 6: STTIFG
      break;
    case 0x08: // Vector 8: STPIFG
      break;
    case 0x0a: // Vector 10: RXIFG3
      break;
    case 0x0c: // Vector 14: TXIFG3
      break;
    case 0x0e: // Vector 16: RXIFG2
      break;
    case 0x10: // Vector 18: TXIFG2
      break;
    case 0x12: // Vector 20: RXIFG1
      break;
    case 0x14: // Vector 22: TXIFG1
      break;
    case 0x16: // Vector 24: RXIFG0
      break;
    case 0x18: // Vector 26: TXIFG0
      debug_puts("TXIFG0\r\n");
      if (TXByteCtr)                          // Check TX byte counter
      {
        debug_puts("About to send ");
        debug_puti(TXData,16);
        debug_puts("\r\n");
        
        UCB0TXBUF = TXData;                   // Load TX buffer
        TXData++;
        TXByteCtr--;                          // Decrement TX byte counter
      }
      else
      {
        debug_puts("About to send stop bit");
        UCB0CTL1 |= UCTXSTP;                  // I2C stop condition
        UCB0IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
        __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
      }
      break;
    case 0x1a: // Vector 28: BCNTIFG
      break;
    case 0x1c: // Vector 30: clock low timeout
      break;
    case 0x1e: // Vector 32: 9th bit
      break;
    default:
      break;
  }
}

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT


  systemInit();                             // Init the Board

  debug_init();

  debug_puts("\r\n\r\n\r\n----------------------------\r\n");
  debug_puts("Debug Interface!\r\n");
  debug_puts("Test: ");
  debug_puti(255, 10);
  debug_puts(" base10 = ");
  debug_puti(255, 16);
  debug_puts(" base16\r\n");
  debug_puts("----------------------------\r\n");

  ///////////////////////////////////////////////////////////
    debug_puts("Starting I2C\r\n");

    TXData = 0x00;

    P1SEL1 |= BIT6 + BIT7;
    P1SEL0 &= ~(BIT6 + BIT7);

    TXByteCtr = 1;                          // Load TX byte counter

    UCB0CTL1 |= UCSWRST;                      // Enable SW reset
    UCB0CTLW0 |= UCMODE_3 + UCMST + UCSYNC;              // I2C master mode
    //UCB0CTLW1 = UCASTP_2;                      // autom. STOP assertion
    UCB0BR0 = 120;                             // fSCL = SMCLK/12 = ~100kHz
    UCB0BR1 = 0;
    //UCB0TBCNT = 0x00; // TX 7 bytes of data
    UCB0I2CSA = 0x48 + 0x01; // address slave is 12hex
    UCB0CTL1 &= ~UCSWRST; // eUSCI_B in operational state
    UCB0IE |= UCTXIE + UCRXIE + UCSTTIE + UCSTPIE + UCNACKIE + UCALIE;                         // Enable TX interrupt

    while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
    UCB0CTL1 |= UCTR + UCTXSTT;
    while (UCB0CTL1 & UCTXSTT);             // Ensure start condition got sent

    while (1) {
      //while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
      //debug_puts("Passed UCTXSTP\r\n");
      //UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
      __bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
      __no_operation();                       // Remain in LPM0 until all data
                                              // Remain in LPM0 until all data
                                              // is TX'd
      //TXData++;                               // Increment data byte
    }

  
  while (1)
  {
    __bis_SR_register(LPM0_bits + GIE);     // Enter LPM0, enable interrupts
    __no_operation();                       // Remain in LPM0 until all data
    // is TX'd
  }

  return 0;
}