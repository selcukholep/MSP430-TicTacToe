/*
 * SPI.cpp
 *
 *  Created on: Dec 23, 2020
 *
 */

#include <ST7920/SPI.h>

SPI::SPI()
{
    // TODO Auto-generated constructor stub

}

SPI::~SPI()
{
    // TODO Auto-generated destructor stub
}

void SPI::init()
{
    UCB0CTL1 = UCSWRST;

    P2DIR  |= CS;
    P2OUT  |= CS;
    P1SEL  |= SIMO + SCLK;
    P1SEL2 |= SIMO + SCLK;

    UCB0CTL0 |= UCCKPH + UCMSB + UCMST + UCSYNC;
    UCB0CTL1 |= UCSSEL_2;

    UCB0BR0 = 0x08;                           // 8MHz 115200
    UCB0BR1 = 0x00;                           // 8MHz 115200

    UCB0CTL1 &= ~UCSWRST;

    setCS(false);
}

void SPI::setCS(bool high)
{
    if (high)   P2OUT |=  CS;
    else        P2OUT &= ~CS;
}

void SPI::write(unsigned char data)
{
    while (!(IFG2 & UCB0TXIFG));

    UCB0TXBUF = data;

    while (!(IFG2 & UCB0TXIFG));
}
