/*
 * Joystick.h
 *
 *  Created on: Nov 14, 2020
 *
 */

#include "msp430g2553.h"

#ifndef JOYSTICK_H_
#define JOYSTICK_H_

#define PIN_X   BIT4    // P1.4
#define PIN_Y   BIT3    // P1.3
#define PIN_BTN BIT3    // P2.3

enum joystickFlags {
    RIGTH   = 0x01,
    TOP     = 0x02,
    LEFT    = 0x04,
    BOTTOM  = 0x08,

    BUTTON  = 0x10,

    BTNLOCK = 0x20,
    ADCLOCK = 0x80
};

class Joystick
{
public:
    Joystick();
    virtual ~Joystick();

    void init();
    void controlADC();
    void controlButton();

    // Event Handlers
    void onDirectionChanged(void (*callback)(char direction));
    void onButtonPressed(void (*callback)());
    void onBackPressed(void (*callback)());

private:
    int analogVoltage[2];
    char flags;                                      // Joystick Status Register

    void calculateDirection();

    void (*directionChangedEvent)(char direction);  // Address of reference void
    void (*buttonPressedEvent)();                   // Address of reference void
    void (*backPressedEvent)();                     // Address of reference void

};

#endif /* JOYSTICK_H_ */
