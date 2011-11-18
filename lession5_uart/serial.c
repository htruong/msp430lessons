#include "msp430fr5739.h"
#include "legacymsp430.h"

#include "serial.h"

//#include <stdlib.h>

void serial_init()
{
	// Configure UART pins P2.0 & P2.1
	P2SEL1 |= BIT0 + BIT1;
	P2SEL0 &= ~(BIT0 + BIT1);
	
	// Configure UART 0
	UCA0CTL1 |= UCSWRST; 
	UCA0CTL1 = UCSSEL_2;                      	// Set SMCLK as UCLk 
	UCA0BRW = 8000000/SERIAL_BAUDRATE;			// set baudrate
	UCA0MCTLW = 0x80; 							// set ?
	UCA0CTL1 &= ~UCSWRST;                     	// release from reset
	
	head = tail = 0;
	UCA0IE = UCRXIE;							// enable interrupt
}

void serial_receive() 
{
	rxbuf[head] = UCA0RXBUF;
	head = ++head == RXBUF_SIZE ? 0 : head;
}

void serial_putchar(char c) {
	while (!(UCA0IFG&UCTXIFG));
	UCA0TXBUF = c;
}

int serial_getchar() {
	int result;
	
	if (!serial_avail()) return -1;
	
	result = rxbuf[tail];
	tail = ++tail == RXBUF_SIZE ? 0 : tail;
	
	return result;
}

void serial_puts(const char* s) {
  	for (; *s != 0; s++) {
  		serial_putchar(*s);
  	}
}

int serial_avail() {
	return head != tail; 
}

//#pragma vector = USCI_A0_VECTOR
//__interrupt void Serial_ISR(void)
interrupt(USCI_A0_VECTOR) Serial_ISR(void)
{
//	serial_putchar('r');
	int c = UCA0RXBUF;
	if (c == 13) {
		serial_puts("\r\n");
	   	while (serial_avail()) {
        		int ch = serial_getchar();
	        	serial_putchar(ch);
		}
		serial_puts("\r\n$ ");
	} else {
	        serial_putchar(c);
		serial_receive();
	}

	__bic_SR_register_on_exit(LPM3_bits + GIE);// Exit LPM4
}

