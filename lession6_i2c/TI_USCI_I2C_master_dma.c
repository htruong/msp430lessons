//******************************************************************************
//   MSP430 USCI I2C Transmitter and Receiver with DMA support
//
//  Description: This code configures the MSP430's USCI module as 
//  I2C master capable of transmitting and receiving bytes. DMA is used for
//  transmitting and receiving of data.
//
//  ***THIS IS THE MASTER CODE***
//
//                    Master                   
//                 MSP430F2619             
//             -----------------          
//         /|\|              XIN|-   
//          | |                 |     
//          --|RST          XOUT|-    
//            |                 |        
//            |                 |        
//            |                 |       
//            |         SDA/P3.1|------->
//            |         SCL/P3.2|------->
//
// Note: External pull-ups are needed for SDA & SCL
//
// Uli Kretzschmar
// Texas Instruments Deutschland GmbH
// November 2007
// Built with IAR Embedded Workbench Version: 3.42A
//******************************************************************************
#include "msp430fr5739.h"                        // device specific header
#include "legacymsp430.h"                        // device specific header

//#include "msp430x22x4.h"
//#include "msp430x23x0.h"
//#include "msp430xG46x.h"
// ...                                         // more devices are possible

#include "TI_USCI_I2C_master_dma.h"

unsigned char byteCtr;
unsigned char last;
unsigned char *save;

//------------------------------------------------------------------------------
// void TI_USCI_I2C_DMA_receiveinit(unsigned char slave_address, 
//                                  unsigned char prescale)
//
// This function initializes the USCI module for master-receive operation. 
//
// IN:   unsigned char slave_address   =>  Slave Address
//       unsigned char prescale        =>  SCL clock adjustment 
//------------------------------------------------------------------------------
void TI_USCI_I2C_DMA_receiveinit(unsigned char slave_address, 
                                 unsigned char prescale)
{
  // USCI module initialization
  P1SEL1 |= BIT6 + BIT7;
  P1SEL0 |= BIT6 + BIT7;
  P1REN |= BIT6 + BIT7;
  
  UCB0CTL1 = UCSWRST;                         // Enable SW reset
  UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;       // I2C Master, synchronous mode
  UCB0CTL1 = UCSSEL_2 + UCSWRST;              // Use SMCLK, keep SW reset
  UCB0BR0 = prescale;                         // set prescaler
  UCB0BR1 = 0;
  UCB0I2CSA = slave_address;                  // set slave address
  UCB0IE = UCNACKIE;
  UCB0CTL1 &= ~UCSWRST;                       // Clear SW reset, resume operation
  
  // DMA initialization
  DMA0CTL = 0;                                // disable DMA channel 0
  DMACTL0 = DMA0TSEL_12;                      // select UCB0RXIFG0 dma trigger
  DMA0CTL |= DMADSTINCR_3;                    // incremet dest. addr after dma
  DMA0CTL |= (DMADSTBYTE + DMASRCBYTE);       // byte to byte transfer
  DMA0CTL |= DMAIE;                           // enable dma interrupt
  DMA0SA = (unsigned long) &UCB0RXBUF;        // set DMA dest address register
  
}

//------------------------------------------------------------------------------
// void TI_USCI_I2C_DMA_transmitinit(unsigned char slave_address, 
//                                   unsigned char prescale)
//
// This function initializes the USCI module for master-transmit operation. 
//
// IN:   unsigned char slave_address   =>  Slave Address
//       unsigned char prescale        =>  SCL clock adjustment 
//------------------------------------------------------------------------------
void TI_USCI_I2C_DMA_transmitinit( unsigned char slave_address, 
                                   unsigned char prescale)
{
  // USCI module initialization
  P1SEL1 |= BIT6 + BIT7;
  P1SEL0 |= BIT6 + BIT7;
  P1REN |= BIT6 + BIT7;
  
  UCB0CTL1 = UCSWRST;                        // Enable SW reset
  UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;       // I2C Master, synchronous mode
  UCB0CTL1 = UCSSEL_2 + UCSWRST;              // Use SMCLK, keep SW reset
  UCB0BR0 = prescale;                         // set prescaler
  UCB0BR1 = 0;
  UCB0I2CSA = slave_address;                  // Set slave address
  UCB0IE = UCNACKIE;
  UCB0CTL1 &= ~UCSWRST;                       // Clear SW reset,resume operation

  // DMA initialization
  DMA0CTL &= 0;                               // disable DMA channel 0
  DMACTL0 = DMA0TSEL_13;                      // select UCB0RXIFG0 is dma trigger
  DMA0CTL |= DMASRCINCR_3;                    // incremet source addr after dma
  DMA0CTL |= (DMADSTBYTE + DMASRCBYTE);       // byte to byte transfer
  DMA0CTL |= DMAIE;                           // enable dma interrupt
  DMA0DA = (unsigned long) &UCB0TXBUF;        // set DMA dest address register

}

//------------------------------------------------------------------------------
// void TI_USCI_I2C_DMA_receive(unsigned char byteCount, unsigned char *field)
//
// This function is used to start an I2C commuincation in master-receiver mode. 
//
// IN:   unsigned char byteCount  =>  number of bytes that should be read
//       unsigned char *field     =>  array variable used to store received data
//------------------------------------------------------------------------------
void TI_USCI_I2C_DMA_receive(unsigned char byteCount, unsigned char *field)
{
  DMA0DA = (unsigned long) field;             // set DMA src address register  
  if (byteCount > 2){
    DMA0SZ = byteCount-2;                     // block size 
    DMA0CTL |= DMAEN;                         // enable DMA channel 0 .... 
    last = 2;                                 //..if more than 2 bytes to be rxd
  } else {
    //UCB0IE |= UCB0RXIE;
    last = byteCount;
  }

  byteCtr = byteCount;

  save = field;
  if ( byteCount == 1 ){
    __disable_interrupt();
    UCB0CTL1 |= UCTXSTT;                      // I2C start condition
    while (UCB0CTL1 & UCTXSTT);               // Start condition sent?
    UCB0CTL1 |= UCTXSTP;                      // I2C stop condition
    __enable_interrupt();
  } else if ( byteCount > 1 ) {
    UCB0CTL1 |= UCTXSTT;                      // I2C start condition
  } else
    while (1);                                // illegal parameter
}

//------------------------------------------------------------------------------
// void TI_USCI_I2C_DMA_transmit(unsigned char byteCount, unsigned char *field)
//
// This function is used to start an I2C commuincation in master-transmit mode. 
//
// IN:   unsigned char byteCount  =>  number of bytes that should be transmitted
//       unsigned char *field     =>  array variable. Its content will be sent.
//------------------------------------------------------------------------------
void TI_USCI_I2C_DMA_transmit(unsigned char byteCount, unsigned char *field){
  DMA0SA = (unsigned long) field;             // set DMA src address register  
  DMA0SZ = byteCount ;                    // block size 
  DMA0CTL |= DMAEN;                           // enable DMA channel 0  
  
  UCB0CTL1 |= UCTR + UCTXSTT;                 // I2C TX, start condition
}

//------------------------------------------------------------------------------
// unsigned char TI_USCI_I2C_notready()
//
// This function is used to check if there is commuincation in progress. 
//
// OUT:  unsigned char  =>  0: I2C bus is idle, 
//                          1: communication is in progress
//-----------------------------------------------------------------------------
unsigned char TI_USCI_I2C_notready(){
  return (UCB0STAT & UCBBUSY);
}
/*
//pragma vector = USCIAB0RX_VECTOR
//__interrupt void USCIAB0RX_ISR(void)
interrupt(USCIAB0RX_VECTOR) I2C_RX(void)
{
  if (UCB0STAT & UCNACKIFG){            // send STOP if slave sends NACK
    UCB0CTL1 |= UCTXSTP;
    UCB0STAT &= ~UCNACKIFG;
  }

}
*/

//#pragma vector = USCIAB0TX_VECTOR
//__interrupt void USCIAB0TX_ISR(void)
interrupt(USCI_B0_VECTOR) I2C_TX(void)
{
  PJDIR |= BIT2;
  PJOUT |= BIT2;
  if ((UCB0IFG & DMA0TSEL__UCB0RXIFG0) && (DMA0CTL & DMAIE)){
    if (last == 2){
      UCB0CTL1 |= UCTXSTP;
      save[byteCtr-2] = UCB0RXBUF;
      last--;
    } else {
      save[byteCtr-1] = UCB0RXBUF;
      UCB0IFG &= ~DMA0TSEL__UCB0RXIFG0;
      UCB0IE &= UCRXIE0;
    }
  } else {
    UCB0CTL1 |= UCTXSTP;
    UCB0IFG &= ~DMA0TSEL__UCB0TXIFG0;
    UCB0IE &= UCTXIE0;
  }
  //PJDIR |= BIT2;
  //PJOUT &= ~BIT2;
}

// DMA interrupt service routine
//#pragma vector=DMA_VECTOR
//__interrupt void dma (void)
interrupt(DMA_VECTOR) I2C_DMA(void)
{ 
  DMA0CTL &= ~DMAIFG;
  if (UCB0CTL1 & UCTR)            // wait til next TX-int to send stop condition
    UCB0IE |= UCTXIE0;
  else 
  {
    UCB0IE |= UCRXIE0;                      // enable RX int to receive last
  }
}

