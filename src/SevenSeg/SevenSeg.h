#pragma once
#include <SevSegShift.h>

#define SHIFT_DS 23
#define SHIFT_STCP 5
#define SHIFT_SHCP 18

extern SevSegShift sevseg;

bool SetupSevenSegDisplay();
void UpdateSevenSegText(const char* text);
void UpdateSevenSegNumber(const int num);
void HandleSevenSegInteraction();
void RefreshSevenSeg();
void DimSevenSegIfIdle(unsigned long lastInteractionTime);