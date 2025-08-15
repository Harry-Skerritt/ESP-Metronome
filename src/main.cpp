#include <Arduino.h>
#include "Buttons/Buttons.h"
#include "Metronome/Metronome.h"
#include "Settings/Settings.h"
#include "OLED/OLEDManager.h"
#include "SevenSeg/SevenSeg.h"

// --- Setup Vars ---
bool oledInit = false;
bool sevenSegInit = false;
bool buttonsInit = false;
bool metronomeInit = false;

// --- Core Tasks ---
TaskHandle_t metronomeTaskHandle = NULL;
TaskHandle_t displayTaskHandle = NULL;
TaskHandle_t sevenSegRefreshHandle = NULL; // Keep this

void UpdateDisplay() {
    static bool firstEntry = true; // Use static here to retain state across calls

    switch (current_screen) {
        case MODE_SELECT:
            if (firstEntry) {
                OLEDModeSelect();
                firstEntry = false;
            }
            sevseg.setChars(GetCurrentModeText()); // This sets the characters, not refreshes
            break;

        case DEFAULT_SCREEN:
            OLEDNormalDisplay(currentBeat, totalBeats);
            sevseg.setNumber(currentTempo); // This sets the number, not refreshes
            firstEntry = true; // Reset firstEntry when changing from MODE_SELECT
            break;

        case SETTINGS:
            OLEDSettingsScreen();
            sevseg.setChars("----"); // This sets the characters, not refreshes
            firstEntry = true; // Reset firstEntry
            break;

        // Add other display cases here as needed
    }
}

void setup() {
    Serial.begin(9600);

    oledInit = SetupOLED();
    sevenSegInit = SetupSevenSegDisplay();
    buttonsInit = SetupButtons();
    metronomeInit = MetronomeSetup(180, 4);


    if (oledInit && sevenSegInit && buttonsInit && metronomeInit) {
        current_screen = DEFAULT_SCREEN;
        // Set initial 7-seg display content here or in UpdateDisplay on first run
        sevseg.setNumber(currentTempo);
    }

    // Task for the metronome logic
    xTaskCreatePinnedToCore(
        [] (void*) {
            while(true) {
                if (current_screen == DEFAULT_SCREEN || current_screen == PROGRAM_SCREEN || current_screen == TAP_TEMPO) {
                    MetronomeUpdate();
                }
                vTaskDelay(pdMS_TO_TICKS(1)); // Small delay to yield to other tasks
            }
        },
        "MetronomeTask", 2048, NULL, 1, &metronomeTaskHandle, 0 // Core 0, Priority 1
    );

    // Task for handling buttons and updating OLED display
    xTaskCreatePinnedToCore(
        [] (void*) {
            while(true) {
                HandleButtons(); // This handles button logic and sets 7-seg content
                UpdateDisplay(); // This handles OLED updates and sets 7-seg content
                vTaskDelay(pdMS_TO_TICKS(1)); // Add a delay to allow other tasks to run
                // taskYIELD(); // vTaskDelay handles yielding
            }
        },
        "DisplayTask", 4096, NULL, 2, &displayTaskHandle, 0 // Core 1, Priority 1
    );

    // Dedicated task for refreshing the 7-segment display (HIGHER PRIORITY)
    xTaskCreatePinnedToCore(
        [] (void*) {
            while(true) {
                sevseg.refreshDisplay();
                // A very small delay or `taskYIELD()` might be needed depending on SevSeg library's internal timing
                // For SevSeg, it's often designed for frequent calls, so a very small delay might be optimal, or even none if it internally manages timing well.
                // If it's still flickering, experiment with `vTaskDelay(pdMS_TO_TICKS(1))` or `taskYIELD()`
                taskYIELD(); // Yield to allow other tasks on the same core to run
            }
        },
        "SevenSegRefresh", 2048, NULL, 2, &sevenSegRefreshHandle, 1 // Core 1, Priority 2 (Higher than DisplayTask)
    );
}

void loop() {
    // The loop() function can be empty when using FreeRTOS tasks
}