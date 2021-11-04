#include "msp430g2553.h"

#include "ST7920/ST7920.h"
#include "Joystick/Joystick.h"
#include "ESP8266/ESP8266.h"
#include "TicTacToe.h"

ST7920 lcd;
Joystick joystick;
ESP8266 wifi;
TicTacToe ticTacToe;

/*
 * Declarations Start
 */

void onJoystickDirectionChanged(char direction);
void onJoystickButtonClicked();
void onJoystickBackPressed();

void onStringReceivedFromWifi(char * string);
void onDetectedStatus(bool status);

void initializeTimer();

/*
 * Declarations End
 */

void main(void)
{
    while(CALBC1_8MHZ == 0xFF);               // do not load, trap CPU!!

    WDTCTL = WDTPW | WDTHOLD;	              // stop watchdog timer
	
	DCOCTL = 0;                               // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_8MHZ;                    // Set DCO
    DCOCTL = CALDCO_8MHZ;

    BCSCTL2 |= DIVM_1;

	wifi.init();
	lcd.init();
	joystick.init();

    joystick.onDirectionChanged(&onJoystickDirectionChanged);
    joystick.onButtonPressed(&onJoystickButtonClicked);
    joystick.onBackPressed(onJoystickBackPressed);

    wifi.onReceivedStringBetweenBrackets(&onStringReceivedFromWifi);
    wifi.onDetectedStatus(&onDetectedStatus);

	initializeTimer();

	ticTacToe.init(lcd, wifi);

	__bis_SR_register(GIE);                       // Enable Interrupt
}

/*
 * Joystick Events Start
 */

void onJoystickDirectionChanged(char direction)
{
    ticTacToe.directionControl(direction);
}

void onJoystickButtonClicked()
{
    ticTacToe.buttonControl();
}

void onJoystickBackPressed()
{
    ticTacToe.backButtonControl();
}

/*
 * Joystick Events End
 */

/*
 * Wifi Events Start
 */

void onStringReceivedFromWifi(char * string)
{
    ticTacToe.onStringReceivedFromWifi(string);
}

void onDetectedStatus(bool status)
{
    ticTacToe.onDetectedStatusFromWifi(status);
}

/*
 * Wifi Events End
 */

void initializeTimer()
{
    TA0CTL = TASSEL_2 + ID_0 + MC_1;    // Timer A0 with SMCLK, count UP
    TA0CCR0 = 1;

    TA0CCTL0 = 0x10;                    // Enable Timer A0 interrupts
}
volatile int i = 0;
#pragma vector = TIMER0_A0_VECTOR                // Timer0_A0 ISR
__interrupt void Timer0_A0 (void)
{
    if (i % 200 == 0) {
        joystick.controlADC();
        joystick.controlButton();

        ticTacToe.timerControl();
    }
    wifi.controlRX();
    if (i++ > 399) i = 0;
}
