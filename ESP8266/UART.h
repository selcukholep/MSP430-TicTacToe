/*
 * UART.h
 *
 *  Created on: Dec 30, 2020
 *      Author: selcuk
 */

#include "msp430g2553.h"
#include "string.h"

#ifndef UART_H_
#define UART_H_

class UART
{
public:
    UART();
    virtual ~UART();

    void init();

    void sendByte(const unsigned char data);
    void sendString(const char* data);
    void sendEndLine();
    char readByte();
};

#endif /* UART_H_ */
