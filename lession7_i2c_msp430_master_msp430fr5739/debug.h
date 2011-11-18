#ifndef DEBUG_H_
#define DEBUG_H_

#define DEBUG_RXBUF_SIZE 16
#define DEBUG_BAUDRATE 9600
#define DEBUG_MAXITOA 16
#define DEBUG_DIGIT_LOOKUP "0123456789ABCDEF"

#ifdef DEBUG_CMD
    #define debug_command_default(x) debug_command(x)
#endif

//////////////////////////////////////////////////////////////////////////////
// Essential functions
// Please implement debug_command and define DEBUG_CMD if you want to intercept
// the debug_command call

void debug_command(char* str);

void debug_init();

void debug_puts(char* c);
void debug_puti(int value, int base);
void debug_putc(char c);

//////////////////////////////////////////////////////////////////////////////
// Misc functions go here, don't bother

int debug_head, debug_tail;

char debug_rxbuf[DEBUG_RXBUF_SIZE];

/*
int debug_getc();
int debug_avail();
*/

char* itoa(const int the_value, char* str, int radix);

#endif /*DEBUG_H_*/
