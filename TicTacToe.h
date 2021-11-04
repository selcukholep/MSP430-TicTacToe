/*
 * TicTacToe.h
 *
 *  Created on: Dec 31, 2020
 *      Author: selcuk
 */

#ifndef TICTACTOE_H_
#define TICTACTOE_H_

#include "ST7920/ST7920.h"
#include "ST7920/Graphics.h"
#include "Joystick/Joystick.h"
#include "ESP8266/ESP8266.h"
#include "Config.h"

#define PAGE_INTRO      0
#define PAGE_MENU       1
#define PAGE_CREDITS    2
#define PAGE_KEYBOARD   3
#define PAGE_LOADER     4
#define PAGE_GAME       5
#define PAGE_MESSAGE    6

class TicTacToe
{
public:
    TicTacToe();
    virtual ~TicTacToe();

    void init(ST7920 lcd, ESP8266 wifi);

    void directionControl(char direction);
    void buttonControl();
    void backButtonControl();
    void timerControl();

    void onStringReceivedFromWifi(char * string);
    void onDetectedStatusFromWifi(bool status);

private:
    ST7920 lcd;
    ESP8266 wifi;

    unsigned char currentPage;
    unsigned char currentMenuItem;
    unsigned char currentKeyboardCharIndex;
    char ownerUsername[7] = { 0 };
    char pairUsername[7]  = { 0 };
    unsigned char ownerScore;
    unsigned char pairScore;
    unsigned char xoxCursor;
    unsigned char xoxTable[9];
    unsigned char ownerSymbol;
    bool isOwnerTurn;
    unsigned int timerCounter;
    char* message;

    /*
     *  0 => Initial
     *  1 => TCP Connected
     *  2 => Owner Username Sent
     *  3 => Pair Username Taken
     *  4 => GameInfo Taken
     *  5 =>
     */
    char gameStatus = 0;

    void goToPage(unsigned char page);

    void drawIntro();
    void drawMenu();
    void drawCredits();
    void drawKeyboard();
    void drawLoader();
    void drawGame();
    void drawMessage();

    void waitPairResponse();

    void drawKeyboardSelectedChar(unsigned char previousIndex);
    void drawXOXItem(unsigned char index, unsigned char item);

    void directionChangedOnMenu(char direction);
    void directionChangedOnKeyboard(char direction);
    void directionChangedOnGame(char direction);

    void buttonClickedOnMenu();
    void buttonClickedOnKeyboard();
    void buttonClickedOnGame();

    void timerTickedOnKeyboard();
    void timerTickedOnLoader();
    void timerTickedOnGame();

};

#endif /* TICTACTOE_H_ */
