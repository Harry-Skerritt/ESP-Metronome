//
// Created by Harry Skerritt on 15/08/2025.
//

#include "SevenSeg.h"
#include <Arduino.h>

SevSegShift sevseg(SHIFT_DS, SHIFT_SHCP, SHIFT_STCP, 1, true);

byte numDigits = 4;
byte digit_pins[] = { 33, 32, 19, 4 };
byte segment_pins[] = { 0, 1, 2, 3, 4, 5, 6, 7 };

const unsigned long SCREEN_DIM_TIMEOUT = 5000;
const int BRIGHTNESS_NORMAL = 255;
const int BRIGHTNESS_DIM = 10;
static bool isDimmed = false;

bool SetupSevenSegDisplay() {
    pinMode(SHIFT_DS, OUTPUT);
    pinMode(SHIFT_STCP, OUTPUT);
    pinMode(SHIFT_SHCP, OUTPUT);

    bool resistorsOnSegments = true; // 'false' means resistors are on digit pins
    byte hardwareConfig = COMMON_CATHODE; // See README.md for options
    bool updateWithDelays = false; // Default 'false' is Recommended
    bool leadingZeros = false; // Use 'true' if you'd like to keep the leading zeros
    bool disableDecPoint = false; // Use 'true' if your decimal point doesn't exist or isn't connected

    sevseg.begin(hardwareConfig, numDigits, digit_pins, segment_pins, resistorsOnSegments,
                 updateWithDelays, leadingZeros, disableDecPoint);
    sevseg.setBrightness(90);
    sevseg.setChars("Test");

    Serial.println("7Seg: Done!");

    return true;
}

void UpdateSevenSegText(const char* text) {
    sevseg.setChars(text);
}

void UpdateSevenSegNumber(const int num) {
    sevseg.setNumber(num);
}

void HandleSevenSegInteraction() {
    sevseg.setBrightness(BRIGHTNESS_NORMAL);
    isDimmed = false;
}

void RefreshSevenSeg() {
    sevseg.refreshDisplay();
}

void DimSevenSegIfIdle(unsigned long lastInteractionTime) {
    if (millis() - lastInteractionTime >= SCREEN_DIM_TIMEOUT && !isDimmed) {
        sevseg.setBrightness(BRIGHTNESS_DIM);
        isDimmed = true;
    }
}


