//
// Created by Harry Skerritt on 15/08/2025.
//

#include "Buttons.h"
#include <Arduino.h>

#include "OLED/OLEDManager.h"
#include "Metronome/Metronome.h"
#include "SevenSeg/SevenSeg.h"

#define BTN_UP_PIN 27
#define BTN_DOWN_PIN 12
#define BTN_TAP_PIN 13

#define LONG_PRESS_TIME 500
#define DEBOUNCE_TIME 50


ButtonData upBtn ;
ButtonData downBtn;
ButtonData tapBtn;


bool SetupButtons() {

    upBtn.pin = BTN_UP_PIN;
    downBtn.pin = BTN_DOWN_PIN;
    tapBtn.pin = BTN_TAP_PIN;

    pinMode(BTN_UP_PIN, INPUT_PULLUP);
    pinMode(BTN_DOWN_PIN, INPUT_PULLUP);
    pinMode(BTN_TAP_PIN, INPUT_PULLUP);

    Serial.println("Buttons: Done!");

    return true;
}

PressType CheckButtonPress(ButtonData &btn) {
    bool reading = (digitalRead(btn.pin) == LOW); // Active low button

    // Debounce
    if (reading != btn.lastStableReading) {
        btn.lastDebounceTime = millis();
    }
    btn.lastStableReading = reading;

    if ((millis() - btn.lastDebounceTime) > DEBOUNCE_TIME) {
        if (reading != btn.lastStableState) {
            btn.lastStableState = reading;

            if (reading) {
                // Button pressed
                btn.pressStartTime = millis();
                btn.longPressReported = false;
                btn.isLongPressing = false;
                btn.lastRepeatTime = millis();
            } else {
                // Button released
                btn.isLongPressing = false;
                if (!btn.longPressReported && (millis() - btn.pressStartTime) < LONG_PRESS_TIME) {
                    return SHORT_PRESS;
                }
            }
        }

        // Detect long press while holding
        if (btn.lastStableState && !btn.longPressReported) {
            if (millis() - btn.pressStartTime >= LONG_PRESS_TIME) {
                btn.longPressReported = true;
                btn.isLongPressing = true;
                return LONG_PRESS;
            }
        }
    }

    return NONE;
}

void HandleButtons() {

    unsigned long currentTime = millis();

    PressType tapPress = CheckButtonPress(tapBtn);
    PressType btnUpPress = CheckButtonPress(upBtn);
    PressType btnDownPress = CheckButtonPress(downBtn);

    // MAIN HANDLING FROM LOOP
    if (tapPress != NONE || btnUpPress != NONE || btnDownPress != NONE) {
        HandleSevenSegInteraction();
    }

    // Screen Logic
    if (tapPress == LONG_PRESS && current_screen != MODE_SELECT) {
        sevseg.setChars("MODE");
        current_screen = MODE_SELECT;
        // Change to the Mode Select
    }

    if (upBtn.isLongPressing && downBtn.isLongPressing) {
        sevseg.setChars(" IP ");
        current_screen = SETTINGS;
        // Change to the settings page
    }

    if (current_screen == MODE_SELECT) {
        if (btnUpPress == SHORT_PRESS) {
            cycleModeUp();
        }
        if (btnDownPress == SHORT_PRESS) {
            cycleModeDown();
        }

        if (tapPress == SHORT_PRESS) {
            switch (metronomeMode) {
                case NORMAL:
                    current_screen = DEFAULT_SCREEN;
                    OLEDNormalDisplay(3);
                    break;
                case PROGRAM:
                    current_screen = PROGRAM_SCREEN;
                    break;
                case TAP:
                    current_screen = TAP_TEMPO;
                    break;
            }
        }
    }

    if (current_screen == DEFAULT_SCREEN) {
        if (btnUpPress == SHORT_PRESS) {
            if (currentTempo < MAX_TEMPO) currentTempo++;
        }
        if (btnDownPress == SHORT_PRESS) {
            if (currentTempo > MIN_TEMPO) currentTempo--;
        }

        // Long Press Actions
        if (upBtn.isLongPressing && currentTime - upBtn.lastRepeatTime >= 100) { // Every 100ms
            if (currentTempo < MAX_TEMPO) currentTempo++;
            upBtn.lastRepeatTime = currentTime;
        }
        if (downBtn.isLongPressing && currentTime - downBtn.lastRepeatTime >= 100) { // Every 100ms
            if (currentTempo > MIN_TEMPO) currentTempo--;
            downBtn.lastRepeatTime = currentTime;
        }
    }


}