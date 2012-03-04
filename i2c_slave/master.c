//******************************************************************************
//  MSP430FR57xx Demo - USCI_B0 I2C Master TX single bytes to MSP430 Slave
//
//  Description: This demo connects two MSP430's via the I2C bus. The master
//  transmits to the slave. This is the master code. It continuously
//  transmits 00h, 01h, ..., 0ffh and demonstrates how to implement an I2C
//  master transmitter sending a single byte using the USCI_B0 TX interrupt.
//  ACLK = n/a, MCLK = SMCLK = BRCLK = default DCO = ~1.045MHz
//
//                                /|\  /|\
//                MSP430FR5739    10k  10k     MSP430FR5739
//                   slave         |    |         master
//             -----------------   |    |   -----------------
//           -|XIN  P1.6/UCB0SDA|<-|----+->|P1.6/UCB0SDA  XIN|-
//            |                 |  |       |                 |
//           -|XOUT             |  |       |             XOUT|-
//            |     P1.7/UCB0SCL|<-+------>|P1.7/UCB0SCL     |
//            |                 |          |                 |
//
//   P. Thanigai
//   Texas Instruments Inc.
//   August 2010
//   Built with CCS V4 and IAR Embedded Workbench Version: 5.10
//******************************************************************************
#include <inttypes.h>
#include "msp430fr5739.h"
#include "legacymsp430.h"


unsigned char TXData[] = {0x00,0x38,0x39,0x14,0x78,0x5E,0x6D,0x0C,0x01,0x06,0x40,'H','e','l','l','o',' ','W', 'o', 'r', 'l', 'd','!'};
unsigned int TXByteCtr = 0;
unsigned int TXByteTotal = 0;

int  main(void) {

    WDTCTL = WDTPW + WDTHOLD;
    // Init SMCLK = MCLk = ACLK = 1MHz
    CSCTL0_H = 0xA5;
    CSCTL1 |= DCOFSEL0 + DCOFSEL1;          // Set max. DCO setting = 8MHz
    CSCTL2 = SELA_3 + SELS_3 + SELM_3;      // set ACLK = MCLK = DCO
    CSCTL3 = DIVA_3 + DIVS_3 + DIVM_3;      // set all dividers to 1MHz

    TXByteTotal = sizeof(TXData)/sizeof(unsigned char);

    PJDIR |= BIT0 + BIT1 + BIT2 + BIT3;
    PJOUT &= ~(BIT0 + BIT1 + BIT2 + BIT3);
    
    // Configure Pins for I2C
    P1SEL1 |= BIT6 + BIT7;                  // Pin init

    UCB0CTLW0 |= UCSWRST;                   // put eUSCI_B in reset state
    UCB0CTLW0 |= UCMODE_3 + UCMST + UCSSEL_2;// I2C master mode, SMCLk
    UCB0BRW = 0x8;                          // baudrate = SMCLK / 8
    UCB0I2CSA = 0x3C;                       // address slave is 48hex
    UCB0CTLW0 &=~ UCSWRST;	            //clear reset register
    UCB0IE |= UCTXIE0 + UCNACKIE;           //transmit and NACK interrupt enable

    while(TXByteCtr < TXByteTotal) {
    __delay_cycles(60000);                   // Delay between transmissions
    TXByteCtr = 1;                          // Load TX byte counter
    while (UCB0CTLW0 & UCTXSTP);            // Ensure stop condition got sent
    UCB0CTLW0 |= UCTR + UCTXSTT;            // I2C TX, start condition

    __bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
    }
    return 0;
}


interrupt(USCI_B0_VECTOR) i2c_isr(void) {
  switch(UCB0IV)
  {
        case 0x00: break;                    // Vector 0: No interrupts break;
        case 0x02: break;
        case 0x04:
          UCB0CTLW0 |= UCTXSTT;             //resend start if NACK
          break;                            // Vector 4: NACKIFG break;
        case 0x18:
	    if (TXByteCtr < TXByteTotal) {
		__delay_cycles(60000);
		UCB0TXBUF = TXData[TXByteCtr];
		PJOUT = TXData[TXByteCtr];
		TXByteCtr++;
	    } else {
		UCB0CTLW0 |= UCTXSTP;
		UCB0IFG &= ~UCTXIFG;
		__bic_SR_register_on_exit(CPUOFF);
	    }
          break;                            // Vector 26: TXIFG0 break;
        default: break;
  }
}