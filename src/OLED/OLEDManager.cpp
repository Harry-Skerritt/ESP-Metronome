//
// Created by Harry Skerritt on 15/08/2025.
//

#include "OLEDManager.h"
#include "Metronome/Metronome.h"
#include <Wire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 oledDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Screens current_screen = DEFAULT_SCREEN;

bool SetupOLED() {
    Wire.begin();

    if (!oledDisplay.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("Failed to initialize display");
        return false;
    }


    OLEDStartupDisplay();
    delay(2000);
    oledDisplay.clearDisplay();
    oledDisplay.setTextColor(SSD1306_WHITE);
    oledDisplay.setTextSize(1);
    oledDisplay.setCursor(0, 0);
    oledDisplay.println("WHY U NO WORK?");
    oledDisplay.display();

    current_screen = INIT;
    Serial.println("OLED Initialized");
    return true;
}

void UpdateScreen() {
    switch (current_screen) {
        case DEFAULT_SCREEN:
            OLEDStartupDisplay();
            break;
        case MODE_SELECT:
            OLEDModeSelect();
            break;
        case SETTINGS:
            OLEDSettingsScreen();
            break;
        default:
            break;
    }
}


// Screens

void OLEDStartupDisplay() {
    oledDisplay.clearDisplay();

    oledDisplay.setTextSize(1);
    oledDisplay.setTextColor(1);
    oledDisplay.setTextWrap(false);
    oledDisplay.setCursor(20, 38);
    oledDisplay.print("Initialising...");

    oledDisplay.setTextSize(2);
    oledDisplay.setCursor(35, 12);
    oledDisplay.print("HELLO");

    oledDisplay.display();
}

void OLEDModeSelect() {
    oledDisplay.clearDisplay();

    oledDisplay.setTextSize(1);

    oledDisplay.setTextColor(1);
    oledDisplay.setTextWrap(false);
    oledDisplay.setCursor(32, 4);
    oledDisplay.print("Select Mode");

    oledDisplay.drawLine(16, 15, 111, 15, 1);

    oledDisplay.setCursor(5, 39);
    oledDisplay.print("Press Tap to confirm");

    oledDisplay.setCursor(14, 29);
    oledDisplay.print("Use +/- to change");

    oledDisplay.display();
}

void OLEDNormalDisplay(int currentBeat, int totalBeats) {
    oledDisplay.clearDisplay();

    oledDisplay.setTextSize(1);

    oledDisplay.setTextColor(1);
    oledDisplay.setTextWrap(false);
    oledDisplay.setCursor(2, 55);
    oledDisplay.print("Mode:");

    oledDisplay.setCursor(32, 55);
    oledDisplay.print(GetCurrentModeText());

    if (currentBeat == 1) {
        oledDisplay.fillCircle(17, 32, 8, 1);
    } else {
        oledDisplay.drawCircle(17, 32, 8, 1);
    }

    if (currentBeat == 2) {
        oledDisplay.fillCircle(48, 32, 8, 1);
    } else {
        oledDisplay.drawCircle(48, 32, 8, 1);
    }

    if (currentBeat == 3) {
        oledDisplay.fillCircle(79, 32, 8, 1);
    } else {
        oledDisplay.drawCircle(79, 32, 8, 1);
    }

    if (currentBeat == 4) {
        oledDisplay.fillCircle(110, 32, 8, 1);
    } else {
        oledDisplay.drawCircle(110, 32, 8, 1);
    }


    oledDisplay.setTextSize(2);
    oledDisplay.setCursor(47, 2);

    // Declare String objects
    String currentBeatString = String(currentBeat);
    String totalBeatString = String(totalBeats);

    // Concatenate
    String outputString = currentBeatString + "/" + totalBeatString;

    // Print to OLED
    oledDisplay.print(outputString);
    oledDisplay.display();

}

void OLEDSettingsScreen() {
    oledDisplay.clearDisplay();

    oledDisplay.setTextSize(1);

    oledDisplay.setTextColor(1);
    oledDisplay.setTextWrap(false);
    oledDisplay.setCursor(47, 3);
    oledDisplay.print("Visit:");

    oledDisplay.setCursor(20, 23);
    oledDisplay.print("ipAddress_text");

    oledDisplay.setCursor(35, 43);
    oledDisplay.print("To change ");

    oledDisplay.setCursor(5, 52);
    oledDisplay.print("the settings & songs");

    oledDisplay.drawRect(17, 18, 96, 17, 1);

    oledDisplay.display();

}