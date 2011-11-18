#include <inttypes.h>
#include "msp430fr5739.h"
#include "legacymsp430.h"

#define DEBUG_CMD
#include "debug.h"

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
  debug_puts("Program got command: ");
  debug_puts(str);
  debug_puts("\r\n");
}


int main(void)
{  
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  systemInit();                             // Init the Board

  debug_init();
  
  debug_puts("\r\n\r\n\r\n----------------------------\r\n");
  debug_puts("Debug Interface Hello World!\r\n");
  debug_puts("Test: ");
  debug_puti(255, 10);
  debug_puts(" base10 = ");
  debug_puti(255, 16);
  debug_puts(" base16\r\n");
  debug_puts("----------------------------\r\n");
  
  while(1) {
    __bis_SR_register(LPM2_bits + GIE);
  }

  return 0;
}