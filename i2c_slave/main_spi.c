#include <inttypes.h>
#include "msp430fr5739.h"
#include "legacymsp430.h"

#define DEBUG_CMD
#include "debug.h"

unsigned char TXData[] = {0x00,0x38,0x39,0x14,0x78,0x5E,0x6D,0x0C,0x01,0x06,0x40,'H','e','l','l'};
unsigned char RXData;

unsigned int done = 0;

// These global variables are used in the ISRs
void systemInit(void) {
    // Startup clock system in max. DCO setting ~8MHz
    // This value is closer to 10MHz on untrimmed parts
    CSCTL0_H = 0xA5;                          // Unlock register
    CSCTL1 |= DCOFSEL0 + DCOFSEL1;            // Set max. DCO setting
    CSCTL2 = SELA_1 + SELS_3 + SELM_3;        // set ACLK = vlo; MCLK = DCO
    CSCTL3 = DIVA_0 + DIVS_0 + DIVM_0;        // set all dividers
    CSCTL0_H = 0x01;                          // Lock Register
}


void debug_command(char* str) {
    debug_puts("Program got command: ");
    debug_puts(str);
    debug_puts("\r\n");
}


int main(void) {
    WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
    systemInit();                             // Init the Board

    debug_init();

    PJDIR |= BIT0;
    PJOUT |= 0x01;

    debug_puts("\r\n\r\n\r\n----------------------------\r\n");
    debug_puts("Debug Interface Ready!\r\n");
    debug_puts("----------------------------\r\n");

    // Want this
    // P2.2 = UCB0Clock
    // P1.6 = UCB0SIMO

    // MSP430FR573x document, pp. 70, 71
    P2SEL1 |= BIT2;
    P2SEL0 &= ~(BIT2);
    
    P1SEL1 |= BIT6;
    P1SEL0 &= ~(BIT6);

    UCB0CTLW0 |= UCSWRST;                     // **Put state machine in reset**
    UCB0CTLW0 |= UCMST+UCSYNC+UCCKPL+UCMSB;   // 3-pin, 8-bit SPI master
    // Clock polarity high, MSB
    UCB0CTLW0 |= UCSSEL_1;                    // ACLK
    UCB0BR0 = 0x02;                           // /2
    UCB0BR1 = 0;                              //
    //UCB0MCTLW = 0;                            // No modulation
    UCB0CTLW0 &= ~UCSWRST;                    // **Initialize USCI state machine**
    UCB0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
    //TXData = 0x1;                             // Holds TX data
    

    while(1) {
	UCB0IE |= UCTXIE;
	__bis_SR_register(LPM2_bits + GIE);
    }

    return 0;
}
/*
interrupt(USCI_B0_VECTOR) spi_isr(void) {
    switch(UCB0IV) {
	case 0:
	    break;                          // Vector 0 - no interrupt
	case 2:	    
	    RXData = UCB0RXBUF;
	    UCB0IFG &= ~UCRXIFG;

	    debug_puts("Received crap: ");
	    debug_puti(RXData, 16);
	    
	    __bic_SR_register_on_exit(CPUOFF);// Wake up to setup next TX
	    break;
	case 4:
	    if (!done) {
		debug_puts("\r\nTransmit begins!\r\n");
		UCB0TXBUF = TXData;             // Transmit characters
		UCB0IE &= ~UCTXIE;
		done = 1;
	    }
	    break;
	default:
	    break;
    }
}

*/
