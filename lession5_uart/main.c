#include <inttypes.h>
#include "msp430fr5739.h"
#include "legacymsp430.h"
#include "serial.h"

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
/*
    // Turn off temp. 
    REFCTL0 |= REFTCOFF; 
    REFCTL0 &= ~REFON;  

    // Terminate Unused GPIOs
    // P1.0 - P1.6 is unused
    P1OUT &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT5 + BIT6 + BIT7);   
    P1DIR &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT5 + BIT6 + BIT7); 
    P1REN |= (BIT0 + BIT1 + BIT2 + BIT3 + BIT5 + BIT6 + BIT7);   

    // P1.4 is used as input from NTC voltage divider
    // Set it to output low
    P1OUT &= ~BIT4;      
    P1DIR |= BIT4; 

    // P2.2 - P2.6 is unused
    P2OUT &= ~(BIT2 + BIT3 + BIT4 + BIT5 + BIT6);    
    P2DIR &= ~(BIT2 + BIT3 + BIT4 + BIT5 + BIT6); 
    P2REN |= (BIT2 + BIT3 + BIT4 + BIT5 + BIT6);   

    // P2.7 is used to power the voltage divider for the NTC thermistor
    P2OUT &= ~BIT7;     
    P2DIR |= BIT7; 

    // P3.0,P3.1 and P3.2 are accelerometer inputs
    P3OUT &= ~(BIT0 + BIT1 + BIT2);   
    P3DIR &= ~(BIT0 + BIT1 + BIT2); 
    P3REN |= BIT0 + BIT1 + BIT2;

    // PJ.0,1,2,3 are used as LEDs
    // crystal pins for XT1 are unused
    PJOUT &= ~(BIT4+BIT5);  
    PJDIR &= ~(BIT4+BIT5);
    PJREN |= BIT4 + BIT5;  
*/

}


int main(void)
{  
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT  
  systemInit();                             // Init the Board
  serial_init();
  serial_puts("\27[2JSerial Hello World!\r\n$ ");

  while(1) {
    //Switch2Pressed = 0;
    __bis_SR_register(LPM2_bits + GIE);
    // __no_operation();			            // For debugger
//    while (serial_avail()) {
//    	int c = serial_getchar();
//    	serial_putchar(c);
//    }  
        
  }
  
  return 0;    
}
 
