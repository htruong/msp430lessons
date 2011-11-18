#include <inttypes.h>
#include <msp430g2231.h>
#include <legacymsp430.h>

#include "I2C.h"

// These global variables are used in the ISRs 
void systemInit(void)
{
  P1DIR |= BIT0;
  P1DIR &= ~(BIT6 + BIT7);
  P1REN |= (BIT6 + BIT7);
  P1OUT = 0x00;
  P1IE |= BIT3 + BIT6 + BIT7;
  P1IES |= (BIT3 + BIT6 + BIT7);
  P1IFG = 0x00;
  __bis_SR_register(GIE);
  /*
    // Startup clock system in max. DCO setting ~8MHz
    // This value is closer to 10MHz on untrimmed parts  
    CSCTL0_H = 0xA5;                          // Unlock register
    CSCTL1 |= DCOFSEL0 + DCOFSEL1;            // Set max. DCO setting
    CSCTL2 = SELA_1 + SELS_3 + SELM_3;        // set ACLK = vlo; MCLK = DCO
    CSCTL3 = DIVA_0 + DIVS_0 + DIVM_0;        // set all dividers 
    CSCTL0_H = 0x01;                          // Lock Register
  */
}

unsigned char i2c_super_command( )
{

  if (usi_command == 0x01) {
    P1OUT |= BIT0;
  } else if (usi_command == 0xFF) {
    P1OUT &= ~BIT0;
  }
  return 0;
}

int main(void)
{  
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT  
  systemInit();                             // Init the Board
  
  blink();blink();blink();
  
  initialize_i2c();
  //i2c_puts("\27[2JSerial Hello World!\r\n$ ");
  //i2c_puts("\x00\x00\x00\x00\x01\x01\x01\x01", 8);
  blink();blink();blink();

  while(1) {
    //Switch2Pressed = 0;
    __bis_SR_register(LPM3_bits + GIE);
    // __no_operation();			            // For debugger
//    while (i2c_avail()) {
//    	int c = i2c_getchar();
//    	i2c_putchar(c);
//    }  
  }

  
  
  return 0;    
}
 
