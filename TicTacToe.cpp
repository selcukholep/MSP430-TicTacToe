/*
 * TicTacToe.cpp
 *
 *  Created on: Dec 31, 2020
 *      Author: selcuk
 */

#include <TicTacToe.h>

TicTacToe::TicTacToe()
{
    currentPage = 0;
    currentMenuItem = 1;
    currentKeyboardCharIndex = 0;

    ownerScore = 0;
    pairScore  = 0;

    gameStatus = 0;
    ownerSymbol = 0;
    isOwnerTurn = false;
}

TicTacToe::~TicTacToe()
{
    // TODO Auto-generated destructor stub
}

void TicTacToe::init(ST7920 lcd, ESP8266 wifi)
{
    TicTacToe::lcd  = lcd;
    TicTacToe::wifi = wifi;

    goToPage(PAGE_INTRO);
}

void TicTacToe::directionControl(char direction)
{
    switch(currentPage) {
        case PAGE_MENU:
            directionChangedOnMenu(direction);
        break;

        case PAGE_KEYBOARD:
            directionChangedOnKeyboard(direction);
        break;

        case PAGE_GAME:
            directionChangedOnGame(direction);
        break;
    }
}

void TicTacToe::buttonControl()
{
    switch(currentPage) {
        case PAGE_MENU:
            buttonClickedOnMenu();
        break;

        case PAGE_KEYBOARD:
            buttonClickedOnKeyboard();
        break;

        case PAGE_GAME:
            buttonClickedOnGame();
        break;

        case PAGE_MESSAGE:
            goToPage(PAGE_MENU);
            break;
    }
}

void TicTacToe::backButtonControl()
{
    switch(currentPage) {
        case PAGE_MENU:
            goToPage(PAGE_INTRO);
        break;

        case PAGE_CREDITS:
            goToPage(PAGE_MENU);
        break;

        case PAGE_KEYBOARD:
            memset(ownerUsername, 0, 7);
            currentKeyboardCharIndex = 0;
            goToPage(PAGE_MENU);
        break;

        case PAGE_LOADER:
            memset(ownerUsername, 0, 7);
            currentKeyboardCharIndex = 0;
            goToPage(PAGE_KEYBOARD);
            break;

        case PAGE_GAME:
            memset(ownerUsername, 0, 7);
            currentKeyboardCharIndex = 0;
            wifi.disconnectTCP();
            __delay_cycles(50000);
            gameStatus = 0;
            goToPage(PAGE_MENU);

        break;
    }
}

void TicTacToe::timerControl()
{
    switch(currentPage) {
        case PAGE_KEYBOARD:
            timerTickedOnKeyboard();
        break;

        case PAGE_LOADER:
            timerTickedOnLoader();
        break;

        case PAGE_GAME:
            timerTickedOnGame();
        break;
    }

    if (timerCounter++ > 3000)   timerCounter = 0;
}

void TicTacToe::onStringReceivedFromWifi(char * string)
{
    switch (gameStatus) {

        case 2:
            gameStatus++;
            for (int i = 0; i < strlen(string); i++) {
                pairUsername[i] = string[i];
            }
            wifi.sendToServer("OK");
            lcd.printString("  Starting ", 1, 52);
        break;

        case 3:             // GameInfo received.
          gameStatus++;

          wifi.sendToServer("OK");
          __delay_cycles(50000);

          isOwnerTurn = string[0]  == 'S';
          ownerSymbol = (string[1] - '0') + 1;

          goToPage(PAGE_GAME);
       break;

        case 4:             // GameStarted
            if (string[0] == 'E') {
                ownerScore = 0;
                pairScore = 0;
                memset(ownerUsername, 0, 7);
                memset(pairUsername, 0, 7);
                gameStatus = 0;
                ownerSymbol = 0;
                isOwnerTurn = false;
                message = (char*) "    Pair Left   ";
                memset(xoxTable, 0, 9);
                goToPage(PAGE_MESSAGE);

            } else if (!isOwnerTurn) {
                wifi.sendToServer("OK");
                __delay_cycles(50000);
                if (string[0] != 'W') {
                    char location = string[1] - '0';
                    xoxTable[location] = ownerSymbol == 1 ? 2 : 1;
                    drawXOXItem(location, xoxTable[location]);
                }
                switch (string[0]) {
                    case 'C':
                        break;
                    case 'W':
                        ownerScore++;
                        lcd.printChar(ownerScore + 0x30, 4, 16);

                        lcd.blinkString("Win )", 1, 29, 10);
                        lcd.printString(ownerSymbol == 1 ? "O" : "X", 1, 16);
                        lcd.printString(ownerSymbol == 1 ? "X" : "O", 1, 42);
                        memset(xoxTable, 0, 9);
                        for (unsigned char i = 0; i < 9; i++) {
                            drawXOXItem(i, 0);
                        }
                        return;
                    case 'L':
                        pairScore++;
                        lcd.printChar(pairScore  + 0x30, 4, 42);

                        lcd.blinkString("Lose (", 1, 29, 10);
                        lcd.printString(ownerSymbol == 1 ? "O" : "X", 1, 16);
                        lcd.printString(ownerSymbol == 1 ? "X" : "O", 1, 42);
                        memset(xoxTable, 0, 9);
                        for (unsigned char i = 0; i < 9; i++) {
                            drawXOXItem(i, 0);
                        }

                        break;
                    case 'D':
                        lcd.blinkString("Drawn /", 1, 29, 10);
                        lcd.printString(ownerSymbol == 1 ? "O" : "X", 1, 16);
                        lcd.printString(ownerSymbol == 1 ? "X" : "O", 1, 42);
                        memset(xoxTable, 0, 9);
                        for (unsigned char i = 0; i < 9; i++) {
                            drawXOXItem(i, 0);
                        }
                        if (string[2] == 'N')
                            return;
                        break;
                }
                isOwnerTurn = true;
                lcd.drawRightArrow(0, 4 + (1 - isOwnerTurn) * 48);
                lcd.fillRectangle(0x00, 0, 4 + isOwnerTurn * 48, 2, 8);

            }

            break;
    }
}

void TicTacToe::onDetectedStatusFromWifi(bool status)
{
    switch (gameStatus) {

        case 0: // TCP Connection Status Received.

            if (status) {   // If TCP connected
                gameStatus++;
                lcd.printString("  Waiting  ", 1, 52);
                __delay_cycles(5000);
                wifi.sendToServer(ownerUsername);
                gameStatus++;   // Owner Username sent.

            } else {
                for (int i = 0; i < 10; i++)
                    __delay_cycles(5000);
                drawLoader();   // Wait and retry to TCP connect.
            }

        break;
    }
}

// Private

void TicTacToe::goToPage(unsigned char page)
{
    currentPage = page;
    lcd.fillScreen(0x00);

    switch (currentPage) {
        case PAGE_INTRO:    drawIntro();        break;
        case PAGE_MENU:     drawMenu();         break;
        case PAGE_CREDITS:  drawCredits();      break;
        case PAGE_KEYBOARD: drawKeyboard();     break;
        case PAGE_LOADER:   drawLoader();       break;
        case PAGE_GAME:     drawGame();         break;
        case PAGE_MESSAGE:  drawMessage();      break;
    }
}

void TicTacToe::drawIntro()
{
    lcd.drawImage(LOGO, 1, 8, 32, 48);

    lcd.printString("HACETTEPE", 3, 8);
    lcd.printString("UNIVERSITY", 3, 18);
    lcd.printString("ELE417", 3, 28);
    lcd.printString("TicTacToe", 3, 48);

     for (int i = 0; i < 8; i++)
         __delay_cycles(1000000);
    bool connectionStatus = true; //wifi.connectWifi(WIFI_SSID, WIFI_PASSWORD);

    if (connectionStatus) {
        goToPage(PAGE_MENU);
    } else {
        drawIntro();
    }
}

void TicTacToe::drawMenu()
{
    lcd.printString("GAME MENU", 1, 8);

    lcd.fillRectangle(0xAA, 2, 17, 9, 1);

    lcd.printString("Find a match", 1, 30);
    lcd.printString("Credits", 1, 40);

    lcd.drawRightArrow(0, 10 * (currentMenuItem + 3));
}

void TicTacToe::drawCredits()
{
    lcd.printString("Selcuk Holep", 1, 8);
    lcd.printString("Burak C. Kan", 1, 18);
    lcd.printString("ELE417 2020", 1, 48);
}

void TicTacToe::drawKeyboard()
{
    lcd.printString("Uname:",     0, 4);
    lcd.printString("ABCDEFGHIJ", 2, 24);
    lcd.printString("KLMNOPQRST", 2, 38);
    lcd.printString("UVWXYZ<-OK", 2, 52);

    drawKeyboardSelectedChar(0);
}

void TicTacToe::drawLoader()
{
    lcd.printString("Welcome", 0, 4);
    lcd.printString(ownerUsername, 4, 4);
    lcd.printString(" Connecting", 1, 52);
    lcd.printString(" *    ", 2, 28);

    wifi.connectTCP(SERVER_IP, SERVER_PORT);

}

void TicTacToe::drawGame()
{
    lcd.fillRectangle(0xFF, 10,  8, 6, 1);
    lcd.fillRectangle(0xFF, 10, 24, 6, 1);
    lcd.fillRectangle(0xFF, 10, 40, 6, 1);
    lcd.fillRectangle(0xFF, 10, 56, 6, 1);

    lcd.fillRectangle(0b1, 9, 8, 1, 49);

    lcd.printString(ownerUsername, 1, 4);
    lcd.printString(pairUsername,  1, 52);

    for (unsigned char i = 0; i < 9; i++) {
       drawXOXItem(i, 0);
   }

   lcd.printChar(ownerScore + 0x30, 4, 16);
   lcd.printChar(pairScore  + 0x30, 4, 42);

   lcd.drawRightArrow(0, 4 + (1 - isOwnerTurn) * 48);
   lcd.printString(ownerSymbol == 1 ? "O" : "X", 1, 16);
   lcd.printString(ownerSymbol == 1 ? "X" : "O", 1, 42);
}

void TicTacToe::drawMessage()
{
    lcd.printString("ERROR", 3, 20);
    lcd.fillRectangle(0xAA, 4, 31, 9, 1);
    lcd.printString(message, 0, 34);
}

void TicTacToe::drawKeyboardSelectedChar(unsigned char previousIndex)
{
    if (previousIndex == 26 || previousIndex == 28) {
        lcd.fillRectangle(0x00, 4 + previousIndex % 10, 33 + 14 * (previousIndex / 10), 2, 1);
    } else {
        lcd.fillRectangle(0x00, 4 + previousIndex % 10, 33 + 14 * (previousIndex / 10), 1, 1);
    }

    if (currentKeyboardCharIndex == 26 || currentKeyboardCharIndex == 28) {
        lcd.fillRectangle(0xF8, 5 + currentKeyboardCharIndex % 10, 33 + 14 * (currentKeyboardCharIndex / 10), 1, 1);
        lcd.fillRectangle(0xFF, 4 + currentKeyboardCharIndex % 10, 33 + 14 * (currentKeyboardCharIndex / 10), 1, 1);
    } else {
        lcd.fillRectangle(0xF8, 4 + currentKeyboardCharIndex % 10, 33 + 14 * (currentKeyboardCharIndex / 10), 1, 1);
    }
}

void TicTacToe::drawXOXItem(unsigned char index, unsigned char item)
{
    item &= 0x07;
    char b[30] = { 0 };

    switch (item) {
        case 0b000:                        break;
        case 0b001: memcpy(b, O_HEX, 30);  break;
        case 0b010: memcpy(b, X_HEX, 30);  break;

        case 0b111:                        break;
        case 0b110: memcpy(b, O_HEX, 30);  break;
        case 0b101: memcpy(b, X_HEX, 30);  break;
    }

    for (int q = 0; q < 30; q++) {

        if (item > 2) {
            b[q] = ~(b[q]);
        }
        if ( q % 2 == 1) {
            b[q] |= 1;
        }
    }
    lcd.drawImage(b, 10 + (index % 3) * 2, 9 + (index / 3) * 16, 16, 15);
}

/*
 * Direction Handlers
 */

void TicTacToe::directionChangedOnMenu(char direction)
{
    unsigned char previousMenuItem = currentMenuItem;

    if (direction & BOTTOM) {
        currentMenuItem++;
        if (currentMenuItem > 1) currentMenuItem = 0;

    } else if (direction & TOP) {
        if (currentMenuItem == 0) currentMenuItem = 1;
        else currentMenuItem--;
    }

    if (currentMenuItem != previousMenuItem) {
        lcd.fillRectangle(0x00, 0, 10 * (previousMenuItem + 3), 2, 8);
        lcd.drawRightArrow(0, 10 * (currentMenuItem + 3));
    }
}

void TicTacToe::directionChangedOnKeyboard(char direction)
{
    unsigned char previousKeyboardCharIndex = currentKeyboardCharIndex;

    if (direction & LEFT) {
        if (currentKeyboardCharIndex > 0)   currentKeyboardCharIndex--;
        else                                currentKeyboardCharIndex = 28;
    }

    if (direction & RIGTH) {
        if (currentKeyboardCharIndex < 28)  currentKeyboardCharIndex++;
        else                                currentKeyboardCharIndex = 0;
    }

    if (direction & TOP) {
        if (currentKeyboardCharIndex > 9)   currentKeyboardCharIndex -= 10;
        else                                currentKeyboardCharIndex = 20 + currentKeyboardCharIndex % 10;
    }

    if (direction & BOTTOM) {
        if (currentKeyboardCharIndex < 20)  currentKeyboardCharIndex += 10;
        else                                currentKeyboardCharIndex %= 10;
    }

    if (currentKeyboardCharIndex == 29)   currentKeyboardCharIndex--;

    if (previousKeyboardCharIndex < currentKeyboardCharIndex && currentKeyboardCharIndex == 27) {
        currentKeyboardCharIndex++;
    }

    if (previousKeyboardCharIndex > currentKeyboardCharIndex && currentKeyboardCharIndex == 27) {
        currentKeyboardCharIndex--;
    }

    drawKeyboardSelectedChar(previousKeyboardCharIndex);
}

void TicTacToe::directionChangedOnGame(char direction)
{
    drawXOXItem(xoxCursor, xoxTable[xoxCursor]);

    if (direction & LEFT) {
        if (xoxCursor > 0)         xoxCursor--;
        else                       xoxCursor = 8;
    }

    if (direction & RIGTH) {
       if (xoxCursor < 8)          xoxCursor++;
       else                        xoxCursor = 0;
   }

   if (direction & TOP) {
       if (xoxCursor > 2)          xoxCursor -= 3;
       else                        xoxCursor  = 6 + xoxCursor % 3;
   }

   if (direction & BOTTOM) {
       if (xoxCursor < 6)          xoxCursor += 3;
       else                        xoxCursor %= 3;
   }
}

/*
 * Button Handlers
 */

void TicTacToe::buttonClickedOnMenu()
{
    timerCounter = 0;
    switch (currentMenuItem) {
        case 0: goToPage(PAGE_KEYBOARD); break;
        case 1: goToPage(PAGE_CREDITS);  break;
    }
}

void TicTacToe::buttonClickedOnKeyboard()
{
    if (currentKeyboardCharIndex < 26) {
        if (strlen(ownerUsername) < 7) {
            ownerUsername[strlen(ownerUsername)] = 'A' + currentKeyboardCharIndex;

            lcd.printString(ownerUsername, 3, 4);
        }
    } else if (currentKeyboardCharIndex == 26) {
        if (strlen(ownerUsername) > 0) {
            ownerUsername[strlen(ownerUsername) - 1] = NULL;
            lcd.fillRectangle(0x00, 6 + strlen(ownerUsername), 4, 1, 12);
            lcd.printString(ownerUsername, 3, 4);
        }
    } else if (currentKeyboardCharIndex == 28 && strlen(ownerUsername) > 0) {
        timerCounter = 0;
        goToPage(PAGE_LOADER);
    }
}

void TicTacToe::buttonClickedOnGame()
{
    if (isOwnerTurn) {

        if (xoxTable[xoxCursor] == 0) {
            xoxTable[xoxCursor] = ownerSymbol;
            drawXOXItem(xoxCursor, xoxTable[xoxCursor]);

            wifi.sendByteToServer((xoxCursor + 0x30));
            xoxCursor = 0;

            isOwnerTurn = false;
            lcd.drawRightArrow(0, 4 + (1 - isOwnerTurn) * 48);
            lcd.fillRectangle(0x00, 0, 4 + isOwnerTurn * 48, 2, 8);
        }
    }
}

void TicTacToe::timerTickedOnKeyboard()
{
    if (timerCounter % 80 == 0) {
        if (currentKeyboardCharIndex == 26 || currentKeyboardCharIndex == 28) {
            lcd.fillRectangle(0xF8, 5 + currentKeyboardCharIndex % 10, 33 + 14 * (currentKeyboardCharIndex / 10), 1, 1);
            lcd.fillRectangle(0xFF, 4 + currentKeyboardCharIndex % 10, 33 + 14 * (currentKeyboardCharIndex / 10), 1, 1);
        } else {
            lcd.fillRectangle(0xF8, 4 + currentKeyboardCharIndex % 10, 33 + 14 * (currentKeyboardCharIndex / 10), 1, 1);
        }
    } else if (timerCounter % 80 == 40) {
        if (currentKeyboardCharIndex == 26 || currentKeyboardCharIndex == 28) {
            lcd.fillRectangle(0x00, 5 + currentKeyboardCharIndex % 10, 33 + 14 * (currentKeyboardCharIndex / 10), 2, 1);
        } else {
            lcd.fillRectangle(0x00, 4 + currentKeyboardCharIndex % 10, 33 + 14 * (currentKeyboardCharIndex / 10), 1, 1);
        }
    }
}

void TicTacToe::timerTickedOnLoader()
{
    if (timerCounter % 1000 == 0) {
        lcd.printString("      ", 2, 28);
    } else if (timerCounter % 1000 == 250) {
       lcd.printString(" *    ", 2, 28);
    } else if (timerCounter % 1000 == 500) {
       lcd.printString(" * *  ", 2, 28);
    } else if (timerCounter % 1000 == 750) {
       lcd.printString(" * * *", 2, 28);
    }
}

void TicTacToe::timerTickedOnGame()
{
    if (isOwnerTurn) {
        if (timerCounter % 80 == 0)
            drawXOXItem(xoxCursor, xoxTable[xoxCursor]);
        else if (timerCounter % 80 == 40)
            drawXOXItem(xoxCursor, ~xoxTable[xoxCursor]);
    }
}
