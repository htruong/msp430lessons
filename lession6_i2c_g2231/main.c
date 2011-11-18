#include <inttypes.h>
#include "msp430g2231.h"
#include "legacymsp430.h"
#include "TI_USCI_I2C_master.h"


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
  BCSCTL1 = CALBC1_8MHZ;
  DCOCTL = CALDCO_8MHZ;

  TI_USCI_I2C_DMA_transmitinit(0x78, 0x08);

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
    // __no_operation();                        // For debugger
//    while (i2c_avail()) {
//      int c = i2c_getchar();
//      i2c_putchar(c);
//    }

  }

  return 0;
}
 
