#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

enum Screens {
    MODE_SELECT,
    DEFAULT_SCREEN,
    PROGRAM_SCREEN,
    TAP_TEMPO,
    SETTINGS,
    INIT
};

extern Screens current_screen;

bool SetupOLED();
void UpdateScreen();

// Screens
void OLEDStartupDisplay();
void OLEDModeSelect();
void OLEDNormalDisplay(int currentBeat, int totalBeats = 4);
void OLEDSettingsScreen();
