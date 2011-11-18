#ifndef USCI_LIB
#define USCI_LIB

#include <msp430g2231.h>
#include <legacymsp430.h>

#define SDA_PIN BIT7                                  // msp430x261x UCB0SDA pin
#define SCL_PIN BIT6                                  // msp430x261x UCB0SCL pin

void TI_USCI_I2C_receiveinit(unsigned char slave_address, unsigned char prescale);
void TI_USCI_I2C_transmitinit(unsigned char slave_address, unsigned char prescale);


void TI_USCI_I2C_receive(unsigned char byteCount, unsigned char *field);
void TI_USCI_I2C_transmit(unsigned char byteCount, unsigned char *field);


unsigned char TI_USCI_I2C_slave_present(unsigned char slave_address);
unsigned char TI_USCI_I2C_notready();


#endif
