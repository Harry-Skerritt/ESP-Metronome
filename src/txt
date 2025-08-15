#include <Arduino.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SevSegShift.h>

// ---- SETUP -----
// --- General ---
enum MetronomeMode {
    NORMAL,
    PROGRAM,
    TAP
};

MetronomeMode metronomeMode = NORMAL;

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

// --- OLED Screen ---
enum Screens {
    MODE_SELECT,
    DEFAULT_SCREEN,
    PROGRAM_SCREEN,
    TAP_TEMPO,
    SETTINGS,
    INIT
};

Screens current_screen = INIT;

const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- Buttons ---

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

const unsigned long LONG_PRESS_TIME = 500;
const unsigned long DEBOUNCE_TIME = 50;

enum PressType {
    NONE,
    SHORT_PRESS,
    LONG_PRESS
};

const int BTN_UP_PIN = 8;
const int BTN_DOWN_PIN = 7;
const int BTN_TAP_PIN = 13;

ButtonData upBtn;
ButtonData downBtn;
ButtonData tapBtn;

// --- Shift Register (7 Seg) ---
const int SHIFT_DS = 4;
const int SHIFT_STCP = 5;
const int SHIFT_SHCP = 6;

SevSegShift sevseg(SHIFT_DS, SHIFT_SHCP, SHIFT_STCP, 1, true);

unsigned long lastInteractionTime = 0;
const unsigned long SCREEN_DIM_TIMEOUT = 5000;
const int BRIGHTNESS_NORMAL = 100;
const int BRIGHTNESS_DIM = 10;

// --- Feedback ---
// Todo: DAC
const int BEAT_INIDICATOR_PIN = 2;

// --- Vars ---
int currentTempo = 180;
constexpr int MAX_TEMPO = 300;
constexpr int MIN_TEMPO = 20;

bool oledInit = false;
bool sevenSegInit = false;
bool buttonsInit = false;
bool feedbackInit = false;

// --- OLED SCREENS ---


// ----- SETUP FUNCTIONS -----
bool SetupSevenSegDisplay(char* initStr = "") {
    byte num_digits = 4;
    byte digit_pins[] = {12, 11, 10, 9};

    byte segment_pins[] = { 0, 1, 2, 3, 4, 5, 6, 7 };

    bool resistors_on_segments = true;

    byte hardware_config = COMMON_CATHODE;

    bool update_with_delays = false;
    bool leading_zeros = false;

    sevseg.begin(hardware_config, num_digits, digit_pins, segment_pins, resistors_on_segments, update_with_delays, leading_zeros, false);

    sevseg.setBrightness(90);

    sevseg.setChars(initStr);

    Serial.println("7Seg: Done!");

    return true;
}

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

bool SetupOLED() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("Failed to initialize display");
        return false;
    }

    //OLEDStartupDisplay(display);
    delay(2000);
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);

    current_screen = INIT;
    Serial.println("OLED Initialized");
    return true;
}

bool SetupFeedback() {
    // Todo: DAC
    pinMode(BEAT_INIDICATOR_PIN, OUTPUT);
    return true;
}

void setup() {

    Serial.begin(9600);

    oledInit = SetupOLED();
    sevenSegInit = SetupSevenSegDisplay();
    buttonsInit = SetupButtons();
    feedbackInit = SetupFeedback();

    if (oledInit && sevenSegInit && buttonsInit && feedbackInit) {
        // All setup correctly
        current_screen = DEFAULT_SCREEN;
        sevseg.setChars("Done");

    }
    else {
        sevseg.setChars("Err!");
    }
}


// ----- MAIN -----
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

void HandleInteraction() {
    lastInteractionTime = millis();
    sevseg.setBrightness(BRIGHTNESS_NORMAL);
}

// Enum Cycling Helper
void cycleModeUp() {
    metronomeMode = static_cast<MetronomeMode>((metronomeMode + 1) % 3);
}

void cycleModeDown() {
    metronomeMode = static_cast<MetronomeMode>((metronomeMode + 2) % 3);
}

void loop() {

    // --- Handle Button Presses ---
    PressType tapPress = CheckButtonPress(tapBtn);
    PressType btnUpPress = CheckButtonPress(upBtn);
    PressType btnDownPress = CheckButtonPress(downBtn);

    if (tapPress != NONE || btnUpPress != NONE || btnDownPress != NONE) {
        HandleInteraction();
    }

    // Screen Logic
    if (tapPress == LONG_PRESS && current_screen != MODE_SELECT) {
        sevseg.setChars("MODE");
        current_screen = MODE_SELECT;
        // Change to the Mode Select
    }

    if (upBtn.isLongPressing && downBtn.isLongPressing) {
        sevseg.setChars("Sett");
        current_screen = SETTINGS;
        // Change to the settings page
    }

    // Handle Mode Select
    if (current_screen == MODE_SELECT) {
        sevseg.setChars("----");
        static bool firstEntry = true;
        if (firstEntry) {
            //OLEDModeSelect(display);
            firstEntry = false;
        }

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
                    break;
                case PROGRAM:
                    current_screen = PROGRAM_SCREEN;
                    break;
                case TAP:
                    current_screen = TAP_TEMPO;
                    break;
            }
            firstEntry = true;
        }

        sevseg.setChars(GetCurrentModeText());
    }

    // Handle Normal Mode (DEFAULT_SCREEN)
    else if (current_screen == DEFAULT_SCREEN) {
        // Short Press Actions
        if (btnUpPress == SHORT_PRESS) {
            if (currentTempo < MAX_TEMPO) currentTempo++;
        }
        if (btnDownPress == SHORT_PRESS) {
            if (currentTempo > MIN_TEMPO) currentTempo--;
        }

        // Long Press Actions
        unsigned long currentTime = millis();
        if (upBtn.isLongPressing && currentTime - upBtn.lastRepeatTime >= 100) { // Every 100ms
            if (currentTempo < MAX_TEMPO) currentTempo++;
            upBtn.lastRepeatTime = currentTime;
        }
        if (downBtn.isLongPressing && currentTime - downBtn.lastRepeatTime >= 100) { // Every 100ms
            if (currentTempo > MIN_TEMPO) currentTempo--;
            downBtn.lastRepeatTime = currentTime;
        }

        sevseg.setNumber(currentTempo);
    }

    sevseg.refreshDisplay();

    // Screen Dim
    if (millis() - lastInteractionTime >= SCREEN_DIM_TIMEOUT) {
        sevseg.setBrightness(BRIGHTNESS_DIM);
    }

}