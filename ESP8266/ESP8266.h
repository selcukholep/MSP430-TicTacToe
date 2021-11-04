/*
 * ESP8266.h
 *
 *  Created on: Dec 25, 2020
 *      Author: selcuk
 */
#include "ESP8266/UART.h"
#include "string.h"

#ifndef ESP8266_H_
#define ESP8266_H_

#define AT_TEST                 "AT"
#define AT_CONNECT_WIFI         "AT+CWJAP"
#define AT_DISCONNECT_SERVER    "AT+CIPCLOSE"
#define AT_CONNECT_SERVER       "AT+CIPSTART"
#define AT_SEND_DATA            "AT+CIPSEND"

#define MAX_RECEIVED_BYTE       50

#define RX_STATUS_BRACKET_SEEN            1

class ESP8266
{
public:
    ESP8266();
    virtual ~ESP8266();

    void init();

    bool connectWifi(const char* ssid, const char* pwd);
    bool connectTCP(const char* ip, const char* port);
    void disconnectTCP();
    void sendToServer(const char* data);
    void sendByteToServer(unsigned char data);

    void readBetweenBrackets(char *string);
    char readChar(unsigned int maxTry);

    void controlRX();

    // Event Handlers
    void onReceivedStringBetweenBrackets(void (*callback) (char * string));
    void onDetectedStatus(void (*callback) (bool status));

private:
    UART uart;
    char stringBuffer[MAX_RECEIVED_BYTE] = { 0 };
    int bufferSize = 0;
    char rxStatus;

    const char* error = (char *) "ERROR";
    const char* ok    = (char *) "OK";
    char detectionSequenceIndexes[2];

    bool detectStatus(unsigned int maxTry);

    // Callback pointers

    void (*receivedStringBetweenBrackets) (char * string);
    void (*detectedStatus) (bool status);                   // False => Error | True => OK

};

#endif /* ESP8266_H_ */
