#pragma once

struct ButtonData {
    int pin;
    bool lastStableState = false;
    bool lastStableReading = false;
    bool longPressReported = false;
    bool isLongPressing = false;
    unsigned long lastDebounceTime = 0;
    unsigned long pressStartTime = 0;
    unsigned long lastRepeatTime = 0;
};


enum PressType {
    NONE,
    SHORT_PRESS,
    LONG_PRESS
};

extern ButtonData upBtn;
extern ButtonData downBtn;
extern ButtonData tapBtn;

bool SetupButtons();
PressType CheckButtonPress(ButtonData &btn);
void HandleButtons();