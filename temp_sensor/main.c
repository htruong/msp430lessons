/******************************************************************************
 *                  MSP-EXP430G2-LaunchPad User Experience Application
 *
 * 1. Device starts up in LPM3 + blinking LED to indicate device is alive
 *    + Upon first button press, device transitions to application mode
 * 2. Application Mode
 *    + Continuously sample ADC Temp Sensor channel, compare result against
 *      initial value
 *    + Set PWM based on measured ADC offset: Red LED for positive offset, Green
 *      LED for negative offset
 *    + Transmit temperature value via TimerA UART to PC
 *    + Button Press --> Calibrate using current temperature
 *                       Send character '°' via UART, notifying PC
 ******************************************************************************/

//#include <msp430x20x2.h>  <-taken care of by includeing io.h and setting -mmcu=msp430x2012 in cflags
#include <io.h>
#include <signal.h>

#include "msp430g2231.h"
#include "legacymsp430.h"


#define     TXD                   BIT1                      // TXD on P1.1

#define     DEV_MODE

#define     BUZZER                BIT6
#define     BUZZER_DIR            P1DIR
#define     BUZZER_OUT            P1OUT
#define     BUZZER_SEL            P1SEL

//   Conditions for 9600/4=2400 Baud SW UART, SMCLK = 1MHz
#define     Bitime_5              0x05*4                      // ~ 0.5 bit length + small adjustment
#define     Bitime                13*4//0x0D

#define     UART_UPDATE_INTERVAL  60000
#define     TRANSMIT_UART

#define     TIMER_PWM_MODE        0
#define     TIMER_UART_MODE       1
#define     TIMER_PWM_PERIOD      2000
#define     TIMER_PWM_OFFSET      20

#define	    TOO_FUCKING_HOT 15



unsigned char BitCnt;

unsigned char timerMode = TIMER_PWM_MODE;

unsigned int TXByte;

/* Using an 8-value moving average filter on sampled ADC values */
long tempMeasured[8];
unsigned char tempMeasuredPosition=0;
long tempAverage,tempAverage2;

long tempCalibrated;

void ConfigureAdcTempSensor(void);
void ConfigureTimerPwm(void);
void ConfigureTimerUart(void);
void Transmit(void);
void InitializeClocks(void);
void InitializeIO(void);
static void __inline__ delay(register unsigned int n);
void play(unsigned int hz);

void play(unsigned int hz) {
    unsigned int old_P1DIR = BUZZER_DIR;
    unsigned int old_P1OUT = BUZZER_OUT;
    unsigned int old_P1SEL = BUZZER_SEL;

    unsigned int old_CCR0 = CCR0;
    unsigned int old_CCR1 = CCR1;
    unsigned int old_TACTL = TACTL;

    BUZZER_DIR |= BUZZER; // P1.2 to output
    BUZZER_OUT |= ~BUZZER;
    BUZZER_SEL |= BUZZER; // P1.2 to TA0.1

    CCR0 = (1000000/hz) -1;
    CCR1 = (1000000/hz)/2;
    TACTL = TASSEL_2 + MC_1;

    ///////////////////////////
    delay(60000);

    TACTL = TASSEL_2 + MC_3;  //stop
    CCR0 = 0;
    BUZZER_DIR = old_P1DIR;
    BUZZER_OUT = old_P1OUT;
    BUZZER_SEL = old_P1SEL;
    CCR0 = old_CCR0;
    CCR1 = old_CCR1;
    TACTL = old_TACTL;
}


void main(void)
{
    unsigned int uartUpdateTimer = UART_UPDATE_INTERVAL;
    unsigned char i;
    WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
    InitializeClocks();

    /* Application Mode begins */
    ConfigureTimerPwm();

    #ifdef DEV_MODE
    P1DIR |= BIT0;
    P1OUT ^= ~BIT0;
    #endif


    __enable_interrupt();                     // Enable interrupts.

    /* Main Application Loop */
    while(1)
    {
	MeasureTemp();

	#ifdef TRANSMIT_UART
	ConfigureTimerUart();
	TXByte = (unsigned char)( ((tempAverage - 630) * 761) / 1024 );
	Transmit();
	uartUpdateTimer = UART_UPDATE_INTERVAL;
	ConfigureTimerPwm();
	#endif

	if (((unsigned char)( ((tempAverage - 630) * 761) / 1024 )) >= TOO_FUCKING_HOT ) {
	    //play(500);
	}

	#ifdef DEV_MODE
	P1OUT ^= ~BIT0;
	#endif
	__bis_SR_register(LPM3_bits + GIE);
    }

    __bic_SR_register_on_exit(LPM3_bits + GIE);
}

void MeasureTemp(void)
{
    unsigned int i;
    /* Configure ADC Temp Sensor Channel */
    ADC10CTL1 = INCH_10 + ADC10DIV_3;         // Temp Sensor ADC10CLK/4
    ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON + ADC10IE;
    delay(1000);                     // Wait for ADC Ref to settle
    ADC10CTL0 |= ENC + ADC10SC;               // Sampling and conversion start
    ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
    __bis_SR_register(CPUOFF + GIE);        // LPM0 with interrupts enabled

    /* Moving average filter out of 8 values to somewhat stabilize sampled ADC */
    tempMeasured[tempMeasuredPosition++] = ADC10MEM;
    if (tempMeasuredPosition == 8)
	tempMeasuredPosition = 0;
    tempAverage2 = 0;
    for (i = 0; i < 8; i++)
	tempAverage2 += tempMeasured[i];
    tempAverage2 >>= 3; // Divide by 8 to get average
    tempAverage=tempAverage2; //use temporary avarage to prevent ISRs from using intermediate values
}


void ConfigureTimerPwm(void)
{
    timerMode = TIMER_PWM_MODE;

    TACCR0 = TIMER_PWM_PERIOD;                              //
    TACTL = TASSEL_2 | MC_1;                  // TACLK = SMCLK, Up mode.
    TACCTL0 = CCIE;
    TACCTL1 = CCIE + OUTMOD_3;                // TACCTL1 Capture Compare
    TACCR1 = 1;
}

void ConfigureTimerUart(void)
{
    timerMode = TIMER_UART_MODE;               // Configure TimerA0 UART TX

    CCTL0 = OUT;                               // TXD Idle as Mark
    TACTL = TASSEL_2 + MC_2 + ID_3;            // SMCLK/8, continuous mode
    P1SEL |= TXD;                        //
    P1DIR |= TXD;                              //
}

// Function Transmits Character from TXByte
void Transmit()
{
    // while (CCR0 != TAR)                       // Prevent async capture
    //  CCR0 = TAR;                             // Current state of TA counter
    asm(						// Re-implement timer capture in assembly
    "JMP 2f \n"
    "1: \n"
    "MOV &0x0170,&0x0172 \n"
    "2: \n"
    "CMP &0x0170,&0x0172\n"
    "JNZ 1b \n");
    CCR0 += Bitime;  			    // Some time till first bit
    BitCnt = 0xA;                             // Load Bit counter, 8data + ST/SP
    TXByte |= 0x100;                        // Add mark stop bit to TXByte
    TXByte = TXByte << 1;                 // Add space start bit
    CCTL0 =  CCIS0 + OUTMOD0 + CCIE;          // TXD = mark = idle
    while ( CCTL0 & CCIE )delay(5);                   // Wait for TX completion, added short delay
}

// Timer A0 interrupt service routine
interrupt(TIMERA0_VECTOR) Timer_A (void)
{
    if (timerMode == TIMER_UART_MODE)
    {
	CCR0 += Bitime;                           // Add Offset to CCR0
	if (CCTL0 & CCIS0)                        // TX on CCI0B?
    {
	if ( BitCnt == 0)
	    CCTL0 &= ~ CCIE;                        // All bits TXed, disable interrupt
	    else
	    {
		CCTL0 |=  OUTMOD2;                    // TX Space
		if (TXByte & 0x01)
		    CCTL0 &= ~ OUTMOD2;                   // TX Mark
		    TXByte = TXByte >> 1;
		BitCnt --;
	    }
    }
    }
}

interrupt(TIMERA1_VECTOR) ta1_isr(void)
{
    TACCTL1 &= ~CCIFG;

    #ifdef DEV_MODE
    P1OUT |= BIT0;
    #endif
    __bic_SR_register_on_exit(CPUOFF);

}

void InitializeClocks(void)
{
    BCSCTL1 = CALBC1_1MHZ;                    // Set range
    DCOCTL = CALDCO_1MHZ;
    BCSCTL2 &= ~(DIVS_3);                         // SMCLK = DCO / 8 = 1MHz
}

void InitializeIO(void)
{
    BUZZER_DIR |= BUZZER;
    BUZZER_OUT &= ~(BUZZER);
}

// Delay Routine from mspgcc help file
static void __inline__ delay(register unsigned int n)
{
    __asm__ __volatile__ (
	"1: \n"
	" dec %[n] \n"
	" jne 1b \n"
	: [n] "+r"(n));
}

interrupt(WDT_VECTOR) WDT_ISR(void)
{
    IE1 &= ~WDTIE;                   /* disable interrupt */
    IFG1 &= ~WDTIFG;                 /* clear interrupt flag */
    WDTCTL = WDTPW + WDTHOLD;        /* put WDT back in hold state */
}

// ADC10 interrupt service routine
interrupt(ADC10_VECTOR) ADC10_ISR (void)
{
    __bic_SR_register_on_exit(CPUOFF);        // Return to active mode
}



