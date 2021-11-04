/*
 * UART.cpp
 *
 *  Created on: Dec 30, 2020
 *      Author: selcuk
 */

#include <ESP8266/UART.h>


UART::UART()
{
    // TODO Auto-generated constructor stub

}

UART::~UART()
{
    // TODO Auto-generated destructor stub
}

void UART::init()
{
    P1SEL = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2 = TXD
    P1SEL2 = BIT1 + BIT2;                     // P1.4 = SMCLK, others GPIO

    UCA0CTL1 |= UCSSEL_2;                     // SMCLK

    UCA0BR0 = 0x41;                           // 4MHz 115200
    UCA0BR1 = 0x03;                           // 4MHz 115200
    UCA0MCTL |= 0x92;                         // Modulation


    UCA0CTL1 &= ~UCSWRST;                     // Initialize USCI state machine
    IE2 |= UCA0TXIE;                          // Enable USCI_A0 TX interrupt
    IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
}

void UART::sendByte(const unsigned char data)
{
    while (!(IFG2 & UCA0TXIFG));
    UCA0TXBUF = data;
}

void UART::sendString(const char* data)
{
    for (int i = 0; i < strlen(data); i++) {
        sendByte(data[i]);
    }
}

void UART::sendEndLine()
{
    sendString("\r\n");
}

char UART::readByte()
{
    while (!(IFG2 & UCA0RXIFG));
    return UCA0RXBUF;
}

