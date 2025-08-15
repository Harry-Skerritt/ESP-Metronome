//
// Created by Harry Skerritt on 15/08/2025.
//

#include "Metronome.h"

MetronomeMode metronomeMode = NORMAL;
int currentTempo = 180;
int currentBeat = 1;
const int totalBeats = 4;
unsigned long beatInterval = 60000UL / currentTempo;
constexpr int MAX_TEMPO = 300;
constexpr int MIN_TEMPO = 20;
const int BEAT_INDICATOR_PIN = 14;
const int BEAT_ONE_INDICATOR_PIN = 2;
unsigned long lastBeatTime = 0;

char* GetCurrentModeText() {
    switch (metronomeMode) {
        case NORMAL:
            return "Normal";
        case PROGRAM:
            return "Program";
        case TAP:
            return "Tap";
        default:
            return "nan ";
    };
}

void cycleModeUp() {
    metronomeMode = static_cast<MetronomeMode>((metronomeMode + 1) % 3);
}

void cycleModeDown() {
    metronomeMode = static_cast<MetronomeMode>((metronomeMode + 2) % 3);
}


bool MetronomeSetup(int tempo, int beats) {
    currentTempo = tempo;
    currentBeat = beats;
    pinMode(BEAT_INDICATOR_PIN, OUTPUT);
    pinMode(BEAT_ONE_INDICATOR_PIN, OUTPUT);
    lastBeatTime = millis();
    return true;
}

void SetTempo(int tempo) {
    currentTempo = tempo;
}

void ChangeBeat() {
    currentBeat++;
    if (currentBeat > totalBeats) currentBeat = 1;

    if (currentBeat == 1) {
        digitalWrite(BEAT_ONE_INDICATOR_PIN, HIGH);
        delay(50);
        digitalWrite(BEAT_ONE_INDICATOR_PIN, LOW);
    } else {
        digitalWrite(BEAT_INDICATOR_PIN, HIGH);
        delay(50);
        digitalWrite(BEAT_INDICATOR_PIN, LOW);
    }


}


void MetronomeUpdate() {
    unsigned long now = millis();
    unsigned long interval = 60000UL / currentTempo;

    if (now - lastBeatTime >= interval) {
        lastBeatTime = now;
        ChangeBeat();
    }
}

