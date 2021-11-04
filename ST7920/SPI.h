/*
 * SPI.h
 *
 *  Created on: Dec 23, 2020
 *
 */

#include "msp430g2553.h"

#ifndef SPI_H_
#define SPI_H_

#define SCLK    BIT5 // SPI clock at P1.5
#define SIMO    BIT7 // SPI MOSI (Master Out, Slave In) at P1.7

#define CS      BIT0 // CS (Chip Select) at P2.0

class SPI
{
public:
    SPI();
    virtual ~SPI();

    void init();

    void setCS(bool high);
    void write(unsigned char data);
};

#endif /* SPI_H_ */
