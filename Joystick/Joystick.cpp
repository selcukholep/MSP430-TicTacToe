/*
 * Joystick.cpp
 *
 *  Created on: Nov 14, 2020
 *
 */

#include <Joystick/Joystick.h>


Joystick::Joystick()
{
    // TODO Auto-generated constructor stub
}

Joystick::~Joystick()
{
    // TODO Auto-generated constructor stub
}

void Joystick::init()
{
    ADC10CTL1 = INCH_4 + CONSEQ_3;  // A4/A3, once multi channel
    ADC10CTL0 = ADC10SHT_3 + MSC + REFON + ADC10ON + ADC10IE;
    ADC10AE0 = PIN_X + PIN_Y; // P1.4,P1.3 ADC option select
    ADC10DTC1 = 0x02; // 2 conversions


    P2DIR &= ~PIN_BTN;  // Set P2.0 as input
    P2REN |= PIN_BTN;
    P2OUT |= PIN_BTN;
    P2IES |= PIN_BTN;   // Set interrupt edge as rising
    P2IE |= PIN_BTN;    // Enable interrupt for P2.0
    P2IFG &= ~PIN_BTN;  // Clear interrupt flag for P2.0
}

void Joystick::controlADC()
{
    ADC10CTL0 &= ~ENC;
    while (ADC10CTL1 & BUSY);       // Wait if ADC10 core is active
    ADC10SA = (int) analogVoltage;  // Data buffer start
    ADC10CTL0 |= ENC + ADC10SC;     // Sampling and conversion ready

    calculateDirection();
}

void Joystick::controlButton() {

    if ((flags & BTNLOCK) == 0 && (P2IN & PIN_BTN) == 0) {
        flags |= BUTTON | BTNLOCK;

    } else if ((flags & BTNLOCK) != 0 && P2IN & PIN_BTN) {
        flags &= ~BUTTON & ~BTNLOCK;

        if ((flags & LEFT) == 0) {
            (*buttonPressedEvent) ();
        }
    }

    P2IFG &= ~PIN_BTN;
}


void Joystick::onDirectionChanged(void (*callback)(char jsr)) {
    directionChangedEvent = callback;
}

void Joystick::onButtonPressed(void (*callback)()) {
    buttonPressedEvent = callback;
}

void Joystick::onBackPressed(void (*callback)()) {
    backPressedEvent = callback;
}

// Private

void Joystick::calculateDirection() {

    unsigned char tempFlags = flags;

    int X = analogVoltage[0];
    int Y = analogVoltage[1];

    if (X < 300)    flags |=  LEFT | ADCLOCK;
    else            flags &= ~LEFT;

    if (Y < 300)    flags |=  BOTTOM | ADCLOCK;
    else            flags &= ~BOTTOM;

    if (X > 800)   flags |=  RIGTH | ADCLOCK;
    else            flags &= ~RIGTH;

    if (Y > 800)   flags |=  TOP | ADCLOCK;
    else            flags &= ~TOP;

    if ((flags & (RIGTH | TOP | LEFT | BOTTOM)) == 0) {
        flags &= ~ADCLOCK;
    }

    if (
            ((tempFlags & ADCLOCK) == 0) && ((flags & ADCLOCK) != 0) &&
            (tempFlags & (RIGTH | TOP | LEFT | BOTTOM)) != (flags & (RIGTH | TOP | LEFT | BOTTOM))) {
        if ((flags & BUTTON) != 0 && (flags & LEFT) != 0) {
            (*backPressedEvent) ();
        } else {
            (*directionChangedEvent) (flags & (RIGTH | TOP | LEFT | BOTTOM));
        }
    }
}
