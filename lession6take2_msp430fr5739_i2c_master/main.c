#include <msp430fr5739.h>
#include <legacymsp430.h>

#define I2C_SDA BIT7   // Serial Data line
#define I2C_SCL BIT6   // Serial Clock line

/* A crude delay function.  Tune by changing the counter value. */
void delay( unsigned int n ) {
    volatile int i;

    for( ; n; n-- ) {
        for( i = 0; i < 500; i++ );
    }
}

void data_read(void ) {
    P1DIR &= ~I2C_SDA; // float to get ready to read
}

void data_high(void ) {
    P1DIR &= ~I2C_SDA; // float pin to go high
    delay( 5 );
}

void data_low(void ) {
    P1OUT &= ~I2C_SDA; // assert low
    P1DIR |= I2C_SDA;
    delay( 5 );
}

void clk_high(void) {
    P1DIR &= ~I2C_SCL;  // float pin to go high
    delay( 10 );
}

void clk_low(void) {
    P1OUT &= ~I2C_SCL;  // assert low
    P1DIR |= I2C_SCL;
    delay( 5 );
}

/* I2C communication starts when both the data and clock
 * lines go low, in that order. */
void I2C_Start(void) {
    clk_high();
    data_high();
    data_low();
    clk_low();
}

/* I2C communication stops with both the clock and data
 * lines going high, in that order. */
void I2C_Stop(void) {
    data_low();
    clk_low();
    clk_high();
    data_high();
}

/* Outputs 8-bit command or data via I2C lines. */
void I2C_out(unsigned char d) {
    int n;

    for( n = 0; n < 8; n++ ) {
        if( d & 0x80 ) {
            data_high();
        } else {
            data_low();
        }

        clk_high();
        clk_low();

        d <<= 1;        // Shift next bit into position.
    }

    data_read();        // Set data line to receive.
    clk_high();         // Clock goes high to wait for acknowledge.

    // Slave will pull data line low to acknowledge.
    while( P1IN & I2C_SDA ) {
        // Else toggle the clock line and check again
        PJOUT ^= BIT0;
        clk_low();
        clk_high();
    }

    clk_low();
}

/* Initializes the LCD panel. */
void init_LCD(void) {
    I2C_Start();

    I2C_out( 0x78 );    // Slave address of the LCD panel.
    I2C_out( 0x00 );    // Control byte: all following bytes are commands.
    I2C_out( 0x38 );    // 8-bit bus, 2-line display, normal instruction mode.
    delay( 10 );

    I2C_out( 0x39 );    // 8-bit bus, 2-line display, extension instruction mode.
    delay( 10 );

    I2C_out( 0x14 );    // Bias set to 1/5.
    I2C_out( 0x78 );    // Contrast set.
    I2C_out( 0x5E );    // Icon display on, booster on, contrast set.
    I2C_out( 0x6D );    // Follower circuit on, amplifier=1?
    I2C_out( 0x0C );    // Display on, cursor off.
    I2C_out( 0x01 );    // Clear display.
    I2C_out( 0x06 );    // Entry mode set to cursor-moves-right.
    delay( 10 );

    I2C_Stop();
}

/* Sends the "clear display" command to the LCD. */
void clear_display(void) {
    I2C_Start();

    I2C_out( 0x78 ); // Slave address of panel.
    I2C_out( 0x00 ); // Control byte: all following bytes are commands.
    I2C_out( 0x01 ); // Clear display.

    I2C_Stop();
}

/* Writes a 20-char string to the RAM of the LCD. */
void show( unsigned char *text ) {
    int n;

    I2C_Start();

    I2C_out( 0x78 ); // Slave address of panel.
    I2C_out( 0x40 ); // Control byte: data bytes follow, data is RAM data.

    for( n = 0; n < 20; n++ ) {
        I2C_out( *text );
        text++;
    }

    I2C_Stop();
}

int main(void) {
    int i;

    /* Stop the watchdog timer so it doesn't reset our chip */
    WDTCTL = WDTPW + WDTHOLD;
    P1REN = I2C_SDA + I2C_SCL;
    // PJOUT |= BIT0;
    PJDIR |= BIT0;

    init_LCD();

    delay(500);
    PJOUT &= ~BIT0;


    show( "Hello, world.       " );
    clear_display();
    show( "Goodbye, world.     " );

    __bis_SR_register( LPM3_bits );     /* go to sleep */
}

