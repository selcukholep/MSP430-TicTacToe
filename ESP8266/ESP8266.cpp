/*
 * ESP8266.cpp
 *
 *  Created on: Dec 25, 2020
 *      Author: selcuk
 */

#include <ESP8266/ESP8266.h>


ESP8266::ESP8266()
{
    // TODO Auto-generated constructor stub

}

ESP8266::~ESP8266()
{
    // TODO Auto-generated destructor stub
}

void ESP8266::init()
{
    uart.init();
}

bool ESP8266::connectWifi(const char* ssid, const char* pwd) //  AT+CWJAP="ssid","pwd"
{
    uart.sendString(AT_CONNECT_WIFI);
    uart.sendString("=\"");
    uart.sendString(ssid);
    uart.sendString("\",\"");
    uart.sendString(pwd);
    uart.sendByte('"');
    uart.sendEndLine();

    return true; //detectStatus(80); // 66

}

bool ESP8266::connectTCP(const char* ip, const char* port) // AT+CIPSTART="TCP","192.168.0.22",3000\r\n
{
    disconnectTCP();                       // Disconnect first anyway

    __delay_cycles(500000);

    uart.sendString(AT_CONNECT_SERVER);    // Connect TCP
    uart.sendString("=\"TCP\",\"");
    uart.sendString(ip);
    uart.sendString("\",");
    uart.sendString(port);
    uart.sendEndLine();

    return true; //detectStatus(60); // 66
}

void ESP8266::disconnectTCP()
{
    uart.sendString(AT_DISCONNECT_SERVER);
    uart.sendEndLine();
}


void ESP8266::sendToServer(const char* data) // AT+CIPSEND=4\r\n
{
    uart.sendString(AT_SEND_DATA);
    uart.sendString("=");

    uart.sendByte(strlen(data) + 0x30);

    uart.sendEndLine();

    detectStatus(20);

    uart.sendString(data);

    uart.sendEndLine();
}

void ESP8266::sendByteToServer(unsigned char data)
{
    uart.sendString(AT_SEND_DATA);
    uart.sendString("=");

    uart.sendByte(0x31);

    uart.sendEndLine();
    detectStatus(20);

    uart.sendByte(data);

    uart.sendEndLine();

}

void ESP8266::readBetweenBrackets(char *string)
{
    char rec = uart.readByte();
    int index = -1;

    while (rec != ']') {

        if (index > -1) {
            string[index] = rec;
            index++;
        }

        if (rec == '[') index = 0;

        rec = uart.readByte();
    }

    sendToServer("OK");
}

char ESP8266::readChar(unsigned int maxTry)
{
    char received = uart.readByte();

    while (maxTry > 0 && received != ':') {
        maxTry--;
        received = uart.readByte();
    }

    if (maxTry > 0) {
        received = uart.readByte();

        sendToServer("OK");

        return received;
    }
    return 0;
}


bool ESP8266::detectStatus(unsigned int maxTry) {

    char received = uart.readByte();

    char* error = (char *) "ERROR";
    char* ok    = (char *) "OK";
    char sequenceIndex[2];
    // ERROR CLOSED OK
    while (maxTry > 0) {

        if (error[sequenceIndex[0]] == received) {
            sequenceIndex[0]++;
        } else  sequenceIndex[0] = 0;

        if (ok[sequenceIndex[1]] == received) {
            sequenceIndex[1]++;
        } else  sequenceIndex[1] = 0;

        if (sequenceIndex[1] == 2) return true;
        if (sequenceIndex[0] == 5) return false;

        maxTry--;
        received = uart.readByte();
    }
    return false;
}

void ESP8266::controlRX()
{
    if ((IFG2 & UCA0RXIFG)) {

            char received = UCA0RXBUF;

            switch (rxStatus) {
                case RX_STATUS_BRACKET_SEEN:
                    stringBuffer[strlen(stringBuffer)] = received;
                break;
            }

            switch (received) {
                case ']':
                    stringBuffer[strlen(stringBuffer) - 1] = 0;

                    (*receivedStringBetweenBrackets) (stringBuffer);

                    memset(stringBuffer, 0, MAX_RECEIVED_BYTE);

                case '[':
                    rxStatus ^= RX_STATUS_BRACKET_SEEN;
                break;

                default:

                    if (error[detectionSequenceIndexes[0]] == received) {
                        detectionSequenceIndexes[0]++;
                    } else detectionSequenceIndexes[0] = 0;

                    if (ok[detectionSequenceIndexes[1]] == received) {
                        detectionSequenceIndexes[1]++;
                    } else  detectionSequenceIndexes[1] = 0;

                    if (detectionSequenceIndexes[1] == 2 | detectionSequenceIndexes[0] == 5) {
                        (*detectedStatus) (detectionSequenceIndexes[1] == 2);
                    }

                break;

            }
        }
}

// Event Handlers

void ESP8266::onReceivedStringBetweenBrackets(void (*callback) (char * string))
{
    receivedStringBetweenBrackets = callback;
}

void ESP8266::onDetectedStatus(void (*callback) (bool status))
{
    detectedStatus = callback;
}

