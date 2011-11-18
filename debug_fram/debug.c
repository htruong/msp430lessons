#include "msp430fr5739.h"
#include "legacymsp430.h"

#include "debug.h"

void debug_init()
{
  // Configure UART pins P2.0 & P2.1
  P2SEL1 |= BIT0 + BIT1;
  P2SEL0 &= ~(BIT0 + BIT1);

  // Configure UART 0
  UCA0CTL1 |= UCSWRST;
  UCA0CTL1 = UCSSEL_2;                      	// Set SMCLK as UCLk
  UCA0BRW = 8000000/DEBUG_BAUDRATE;			// set baudrate
  UCA0MCTLW = 0x80; 							// set ?
  UCA0CTL1 &= ~UCSWRST;                     	// release from reset

  debug_head = debug_tail = 0;
  UCA0IE = UCRXIE;							// enable interrupt
}

void debug_putc(char c) {
  while (!(UCA0IFG&UCTXIFG));
  UCA0TXBUF = c;
}
/*
 * int debug_getc() {
 *    int result;
 *
 *    if (!debug_avail()) return -1;
 *
 *    result = debug_rxbuf[debug_tail];
 *    debug_tail = ++debug_tail == DEBUG_RXBUF_SIZE ? 0 : debug_tail;
 *
 *    return result;
 }

 int debug_avail() {
   return debug_head != debug_tail;
 }

 */
void debug_puts(char* s) {
  while (*s) {
    debug_putc(*s++);
  }
}


void debug_puti(int value, int radix) {
  char debug_ibuf[DEBUG_MAXITOA] = "NaN";
  itoa(value, debug_ibuf, radix);
  debug_puts(debug_ibuf);
}

/* http://www.daniweb.com/software-development/c/threads/148080 */
char* itoa(int value, char* str, int radix) {

  int n = 0, neg = 0;
  unsigned int v;
  char* p, *q;
  char c;

  if (radix == 10 && value < 0) {
    value = -value;
    neg = 1;
  }
  v = value;
  do {
    str[n++] = (DEBUG_DIGIT_LOOKUP)[v%radix];
    v /= radix;
  } while (v);
  if (neg)
    str[n++] = '-';
  str[n] = '\0';

  for( p = str, q = str+n-1; p<q; p++, q-- )
    c = *p, *p = *q, *q = c;

  return str;
}


void debug_command_default(char* str) {
  debug_puts("Got command: ");
  debug_puts(str);
  debug_puts("\r\n");
}

interrupt(USCI_A0_VECTOR) debug_isr(void)
{
  debug_putc(UCA0RXBUF);

  if (UCA0RXBUF != 13) {
    debug_rxbuf[debug_head] = UCA0RXBUF;
    debug_head = ++debug_head == DEBUG_RXBUF_SIZE ? 0 : debug_head;
  } else {
    if (debug_head != debug_tail) {
      debug_rxbuf[debug_head] = '\0';
      debug_command(debug_rxbuf);
    }
    debug_head = debug_tail = 0;
  }

  //__bic_SR_register_on_exit(LPM3_bits + GIE);// Exit LPM4
}
