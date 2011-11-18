#ifndef SERIAL_H_
#define SERIAL_H_

#define RXBUF_SIZE 64
#define SERIAL_BAUDRATE 115200

int head, tail;
char rxbuf[RXBUF_SIZE];

void serial_init();
void serial_putchar(char c);
int serial_getchar();
int serial_avail(); 
void serial_puts(const char *c);	

void serial_receive();
	

#endif /*SERIAL_H_*/
