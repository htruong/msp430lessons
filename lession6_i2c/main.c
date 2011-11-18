#include <inttypes.h>
#include "msp430fr5739.h"
#include "legacymsp430.h"
#include "TI_USCI_I2C_master_dma.h"


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

/* A crude delay function.  Tune by changing the counter value. */
void delay( unsigned int n ) {
    volatile int i;

    for( ; n; n-- ) {
        for( i = 0; i < 50; i++ );
    }
}

int main(void)
{  
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT  
  systemInit();                             // Init the Board
  PJDIR |= BIT0;
  PJOUT |= BIT0;
  
  TI_USCI_I2C_DMA_transmitinit(0x78, 0x08);
  PJDIR |= BIT1;
  PJOUT |= BIT1;
  
  unsigned char transmit_string[40];

  while ( TI_USCI_I2C_notready() );

  TI_USCI_I2C_DMA_transmit(2, (unsigned char *) "\x00\x38");
  while ( TI_USCI_I2C_notready() );

  TI_USCI_I2C_DMA_transmit(1, (unsigned char *) "\x39");    // 8-bit bus, 2-line display, extension instruction mode.
  while ( TI_USCI_I2C_notready() );

  TI_USCI_I2C_DMA_transmit(7,  (unsigned char *) "\x14\x78\x5e\x6d\x0c\x01\x06");
  while ( TI_USCI_I2C_notready() );

  TI_USCI_I2C_DMA_transmit(2,  (unsigned char *) "\x00\x01");
  while ( TI_USCI_I2C_notready() );

  TI_USCI_I2C_DMA_transmit(21, (unsigned char *) "\x40Hello, world.       ");

  while(1) {
    //Switch2Pressed = 0;
    __bis_SR_register(LPM2_bits + GIE);
    // __no_operation();			            // For debugger
//    while (i2c_avail()) {
//    	int c = i2c_getchar();
//    	i2c_putchar(c);
//    }  
        
  }
  
  return 0;    
}
 
