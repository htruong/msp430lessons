//******************************************************************************
//  MSP430F20xx Demo - I2C Slave Receiver, single byte
//
//  Description: I2C Slave communicates with I2C Master using
//  the USI. Master data should increment from 0x00 with each transmitted byte
//  which is verified by the slave.
//  LED off for address or data Ack; LED on for address or data NAck.d by the slave.
//  ACLK = n/a, MCLK = SMCLK = Calibrated 1MHz
//
//  ***THIS IS THE SLAVE CODE***
//
//                  Slave                      Master
//                                      (msp430x20x3_usi_07.c)
//               MSP430F20x2/3              MSP430F20x2/3
//             -----------------          -----------------
//         /|\|              XIN|-    /|\|              XIN|-
//          | |                 |      | |                 |
//          --|RST          XOUT|-     --|RST          XOUT|-
//            |                 |        |                 |
//      LED <-|P1.0             |        |                 |
//            |                 |        |             P1.0|-> LED
//            |         SDA/P1.7|<-------|P1.7/SDA         |
//            |         SCL/P1.6|<-------|P1.6/SCL         |
//
//  Note: internal pull-ups are used in this example for SDA & SCL
//
//  Z. Albus
//  Texas Instruments Inc.
//  May 2006
//  Built with CCE Version: 3.2.0 and IAR Embedded Workbench Version: 3.41A
//******************************************************************************

#include  <msp430x20x3.h>
#include  <legacymsp430.h>

char MST_Data = 0;                     // Variable for received data
char SLV_Addr = 0x90;                  // Address is 0x48<<1 for R/W
int I2C_State = 0;                     // State variable

#define   TXD       BIT1
#define   RXD       BIT2
#define DEBUG_MAXITOA 16
#define DEBUG_DIGIT_LOOKUP "0123456789ABCDEF"


/* Ticks per bit.  Use the following values based on speed:
 * 9600 bps ->  13
 * 2400 bps ->  52
 * 1200 bps -> 104
 * I did not have success with slower speeds, like 300 bps.
 */
#define   TPB      52

/* A pointer to the data to send, and a counter of the bytes. */
unsigned char *data;
unsigned int bytestosend = 0;

/* The actual byte we are transmitting, with its start and stop bits,
 * and a counter of the bits left to send.
 */
int TXWord;
unsigned char bitcnt = 0;

/* function prototypes */
void initUart( void );
int sendByte( unsigned char b );
int sendBytes( const unsigned char *d, int len );
int sendString( const char *str );

void main(void)
{
  WDTCTL = WDTPW + WDTHOLD;            // Stop watchdog
  if (CALBC1_1MHZ ==0xFF || CALDCO_1MHZ == 0xFF)
  {
    while(1);                          // If calibration constants erased
                                       // do not load, trap CPU!!
  }
  BCSCTL1 = CALBC1_1MHZ;               // Set DCO
  DCOCTL = CALDCO_1MHZ;

  P1OUT = 0xC0;                        // P1.6 & P1.7 Pullups
  P1REN |= 0xC0;                       // P1.6 & P1.7 Pullups
  P1DIR = 0xFF;                        // Unused pins as outputs
  P2OUT = 0;
  P2DIR = 0xFF;

  initUart();
  //sendString( "Hello, world!\r\n" );
  
  USICTL0 = USIPE6+USIPE7+USISWRST;    // Port & USI mode setup
  USICTL1 = USII2C+USIIE+USISTTIE;     // Enable I2C mode & USI interrupts
  USICKCTL = USICKPL;                  // Setup clock polarity
  USICNT |= USIIFGCC;                  // Disable automatic clear control
  USICTL0 &= ~USISWRST;                // Enable USI
  USICTL1 &= ~USIIFG;                  // Clear pending flag
  __bis_SR_register(GIE);

  while(1)
  {
      __bis_SR_register(LPM0_bits);     // Enter LPM0, enable interrupts
      __no_operation();                       // Remain in LPM0 until all data
  }
}



/* http://www.daniweb.com/software-development/c/threads/148080 */
char* itoa(const int the_value, char* str, int radix) {

  int n = 0, neg = 0;
  unsigned int v;
  char* p, *q;
  char c;
  int value = the_value;

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


//******************************************************
// USI interrupt service routine
//******************************************************
interrupt(USI_VECTOR) USI_TXRX_handler (void)
{
  char buf[8];
              
  itoa(USISRL,buf,16);
  sendString( buf );
                
  if (USICTL1 & USISTTIFG)             // Start entry?
  {
    P1OUT |= BIT0;                     // LED on: sequence start
    I2C_State = 2;                     // Enter 1st state on start
  }

  switch(I2C_State)
    {
      case 0: // Idle, should not get here
              break;

      case 2: // RX Address
              USICNT = (USICNT & 0xE0) + 0x08; // Bit counter = 8, RX address
              USICTL1 &= ~USISTTIFG;   // Clear start flag
              I2C_State = 4;           // Go to next state: check address

              break;

      case 4: // Process Address and send (N)Ack
              if (USISRL & 0x01)       // If read...
                SLV_Addr++;            // Save R/W bit
              USICTL0 |= USIOE;        // SDA = output
              
              //if (USISRL == (SLV_Addr & 0xFF))  // Address match?
              if (1)
              {
                USISRL = 0x00;         // Send Ack
                P1OUT &= ~BIT0;        // LED off
                I2C_State = 8;         // Go to next state: RX data
              }
              else
              {
                USISRL = 0xFF;         // Send NAck
                P1OUT |= BIT0;         // LED on: error
                //itoa((SLV_Addr & 0xFF),buf,16);
                //sendString( buf );
                I2C_State = 6;         // Go to next state: prep for next Start
              }
              USICNT |= 0x01;          // Bit counter = 1, send (N)Ack bit
              break;

      case 6: // Prep for Start condition
              USICTL0 &= ~USIOE;       // SDA = input
              SLV_Addr = 0x90;         // Reset slave address
              I2C_State = 0;           // Reset state machine
              break;

      case 8: // Receive data byte
              USICTL0 &= ~USIOE;       // SDA = input
              USICNT |=  0x08;         // Bit counter = 8, RX data
              I2C_State = 10;          // Go to next state: Test data and (N)Ack
              break;

      case 10:// Check Data & TX (N)Ack
              USICTL0 |= USIOE;        // SDA = output
              
              if (USISRL == MST_Data)  // If data valid...
              {
                USISRL = 0x00;         // Send Ack
                // Hooray!
                P1DIR |= BIT4;
                P1OUT |= BIT4;
                
                MST_Data++;            // Increment Master data
                P1OUT &= ~BIT0;        // LED off
              }
              else
              {
                USISRL = 0xFF;         // Send NAck
                P1OUT |= BIT0;         // LED on: error
              }
              USICNT |= 0x01;          // Bit counter = 1, send (N)Ack bit
              I2C_State = 6;           // Go to next state: prep for next Start
              break;
    }

  USICTL1 &= ~USIIFG;                  // Clear pending flags
}




void initUart( void ) {
    /* Set up transmit as output pin and set it high */
    P1OUT |= TXD;
    P1DIR |= TXD;

    /* set up the clocks for 1 mhz */
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
    BCSCTL2 &= ~(DIVS_3);

    /* Set timer A to use count up mode 1 mhz / 8 = 125 khz. */
    TACTL = TASSEL_2 + MC_1 + ID_3 + TACLR;

    /* Set ticks-per-bit to specify communication speed */
    TACCR0 = TPB;
}

/* Prepares a block of data to be sent. Returns number of bytes sent. */
int sendBytes( const unsigned char *d, int len ) {
    /* can't queue up data if we're still sending */
    if( bytestosend > 0 ) return 0;

    bitcnt = 0;
    data = d;
    bytestosend = len;

    /* clear interrupt flag, and tell Timer A0 to
     * start triggering interrupts
     */
    TACCTL0 &= ~CCIFG;
    TACCTL0 |= CCIE;

    /* sleep until message sent */
    while( TACCTL0 & CCIE ) {
        __bis_SR_register( LPM0_bits + GIE );
    }

    return len;
}

/* Sends a single byte to the computer.  Returns number of bytes sent. */
int sendByte( unsigned char b ) {
    return sendBytes( &b, 1 );
}

/* Sends a string to the computer.  Returns number of bytes sent. */
int sendString( const char *str ) {
    while(*str)
      sendBytes(str++ , 1);
}

/* This continuously sends bits of the TXWord starting from the
 * least significant bit (the 0 start bit).  One bit is sent every
 * time the handler is activated.  When the bits run out, a new
 * byte is loaded from the data pointer, until bytestosend equals 0.
 */
interrupt(TIMERA0_VECTOR) TimerA0(void) {
    TACCTL0 &= ~CCIFG;

    /* if no bits to send, either load a byte or return */
    if( ! bitcnt ) {
        if( bytestosend > 0 ) {
            /* load the byte */
            TXWord = *data++;
            /* add stop bit */
            TXWord |= 0x100;
            /* add start bit */
            TXWord <<= 1;

            /* 1 start bit + 8 data bits + 1 stop bit */
            bitcnt = 10;

            bytestosend --;
        } else {
            /* no bits left, turn off interrupts and wake up */
            TACCTL0 &= ~ CCIE;
            __bic_SR_register_on_exit( LPM0_bits );
            return;
        }
    }

    /* send least significant bit */
    if( TXWord & 0x01 ) {
        P1OUT |= TXD;
        //P1OUT |= RED_LED;               // for testing
        //P1OUT &= ~ GRN_LED;             // for testing
    } else {
        P1OUT &= ~TXD;
        //P1OUT |= GRN_LED;               // for testing
        //P1OUT &= ~ RED_LED;             // for testing
    }

    /* shift word to remove one bit */
    TXWord >>= 1;
    bitcnt --;
}
