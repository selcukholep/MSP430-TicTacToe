/*
 * ST7920.h
 *
 *  Created on: Dec 23, 2020
 *
 */

#include <ST7920/Graphics.h>
#include "msp430g2553.h"
#include "SPI.h"
#include <string.h>

#ifndef ST7920_H_
#define ST7920_H_

#define LCD_CLS         0x01
#define LCD_HOME        0x02
#define LCD_ADDRINC     0x06
#define LCD_DISPLAYON   0x0C
#define LCD_DISPLAYOFF  0x08
#define LCD_CURSORON    0x0E
#define LCD_CURSORBLINK 0x0F
#define LCD_BASIC       0x30
#define LCD_EXTEND      0x34
#define LCD_GFXMODE     0x36
#define LCD_TXTMODE     0x34
#define LCD_STANDBY     0x01
#define LCD_SCROLL      0x03
#define LCD_SCROLLADDR  0x40
#define LCD_ADDR        0x80

class ST7920
{
public:
    ST7920();
    virtual ~ST7920();
    void init();

    void sendCommand(unsigned char cmd);
    void sendData(unsigned char data);

    void gotoXY(int x, int y);
    void fillScreen(unsigned char background);
    void fillRectangle(unsigned char background, int x, int y, int width, int height);

    void drawMultipleVerticalLines(unsigned char times, int x, int y, int height);

    void printChar(const char c, int x, int y);
    void printString(const char* string, int x, int y);
    void blinkString(const char* string, int x, int y, unsigned char times);
    void drawImage(const char* image, int x, int y, int width, int height);
    void drawRightArrow(int x, int y);

private:
    SPI spi;
};

#endif /* ST7920_H_ */
