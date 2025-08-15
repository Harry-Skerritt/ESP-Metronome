//
// Created by Harry Skerritt on 15/08/2025.
//

#include "Settings.h"

char* settingsName[] = {
    "Sound Type:",
    "LED Blink:",
    "Time Signature",
    "Connect Web?"
    "Reset:"
};

int soundType = 0;
bool ledBlink = true;
bool connectWeb = false;
bool reset = false;
char timeSignature = '44';