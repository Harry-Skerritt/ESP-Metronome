#pragma once
#include <Arduino.h>

enum MetronomeMode {
    NORMAL,
    PROGRAM,
    TAP
};

extern MetronomeMode metronomeMode;
extern int currentTempo;
extern int currentBeat;
extern const int totalBeats;
extern unsigned long beatInterval;
extern const int BEAT_INDICATOR_PIN;
extern const int BEAT_ONE_INDICATOR_PIN;
extern const int MAX_TEMPO ;
extern const int MIN_TEMPO ;

char* GetCurrentModeText();
void cycleModeUp();
void cycleModeDown();

bool MetronomeSetup(int tempo, int beats);
void MetronomeUpdate();
void SetTempo(int tempo);
void ChangeBeat();