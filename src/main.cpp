#include <Arduino.h>
#include <SevSegShift.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

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

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 oledDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// --- OLED SCREENS ---
void OLEDStartupDisplay() {
    oledDisplay.clearDisplay();

    oledDisplay.setTextSize(1);
    oledDisplay.setTextColor(1);
    oledDisplay.setTextWrap(false);
    oledDisplay.setCursor(20, 38);
    oledDisplay.print("Initialising...");

    oledDisplay.setTextSize(2);
    oledDisplay.setCursor(35, 12);
    oledDisplay.print("HELLO");

    oledDisplay.display();
}

void OLEDModeSelect() {
    oledDisplay.clearDisplay();

    oledDisplay.setTextSize(1);

    oledDisplay.setTextColor(1);
    oledDisplay.setTextWrap(false);
    oledDisplay.setCursor(32, 4);
    oledDisplay.print("Select Mode");

    oledDisplay.drawLine(16, 15, 111, 15, 1);

    oledDisplay.setCursor(5, 39);
    oledDisplay.print("Press Tap to confirm");

    oledDisplay.setCursor(14, 29);
    oledDisplay.print("Use +/- to change");

    oledDisplay.display();
}

void OLEDNormalDisplay(int currentBeat, int totalBeats = 4) {
    oledDisplay.clearDisplay();

    oledDisplay.setTextSize(1);

    oledDisplay.setTextColor(1);
    oledDisplay.setTextWrap(false);
    oledDisplay.setCursor(2, 55);
    oledDisplay.print("Mode:");

    oledDisplay.setCursor(32, 55);
    oledDisplay.print(GetCurrentModeText());

    if (currentBeat == 1) {
        oledDisplay.fillCircle(17, 32, 8, 1);
    } else {
        oledDisplay.drawCircle(17, 32, 8, 1);
    }

    if (currentBeat == 2) {
        oledDisplay.fillCircle(48, 32, 8, 1);
    } else {
        oledDisplay.drawCircle(48, 32, 8, 1);
    }

    if (currentBeat == 3) {
        oledDisplay.fillCircle(79, 32, 8, 1);
    } else {
        oledDisplay.drawCircle(79, 32, 8, 1);
    }

    if (currentBeat == 4) {
        oledDisplay.fillCircle(110, 32, 8, 1);
    } else {
        oledDisplay.drawCircle(110, 32, 8, 1);
    }


    oledDisplay.setTextSize(2);
    oledDisplay.setCursor(47, 2);

    // Declare String objects
    String currentBeatString = String(currentBeat);
    String totalBeatString = String(totalBeats);

    // Concatenate
    String outputString = currentBeatString + "/" + totalBeatString;

    // Print to OLED
    oledDisplay.print(outputString);



    oledDisplay.display();

}

// --- Settings ---
char* settingsName[] = {
    "Sound Type:",
    "LED Blink:",
    "Connect Web?"
    "Reset:"
};

int soundType = 0;
bool ledBlink = true;
bool connectWeb = false;
bool reset = false;

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

const int BTN_UP_PIN = 27;
const int BTN_DOWN_PIN = 12;
const int BTN_TAP_PIN = 13;

ButtonData upBtn;
ButtonData downBtn;
ButtonData tapBtn;

// --- Shift Register (7 Seg) ---
#define SHIFT_DS 23
#define SHIFT_STCP 5
#define SHIFT_SHCP 18

byte numDigits = 4;
byte digit_pins[] = { 33, 32, 19, 4 };
byte segment_pins[] = { 0, 1, 2, 3, 4, 5, 6, 7 };

SevSegShift sevseg(SHIFT_DS, SHIFT_SHCP, SHIFT_STCP, 1, true);

unsigned long lastInteractionTime = 0;
const unsigned long SCREEN_DIM_TIMEOUT = 5000;
const int BRIGHTNESS_NORMAL = 255;
const int BRIGHTNESS_DIM = 10;
static bool isDimmed = false;


// --- Feedback ---
// Todo: DAC
const int BEAT_INIDICATOR_PIN = 14;

// --- Vars ---
int currentTempo = 180;
constexpr int MAX_TEMPO = 300;
constexpr int MIN_TEMPO = 20;

bool oledInit = false;
bool sevenSegInit = false;
bool buttonsInit = false;
bool feedbackInit = false;



// ----- SETUP FUNCTIONS -----
bool SetupOLED() {
    Wire.begin();

    if (!oledDisplay.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("Failed to initialize display");
        return false;
    }


    OLEDStartupDisplay();
    delay(2000);
    oledDisplay.clearDisplay();
    oledDisplay.setTextColor(SSD1306_WHITE);
    oledDisplay.setTextSize(1);
    oledDisplay.setCursor(0, 0);
    oledDisplay.println("WHY U NO WORK?");
    oledDisplay.display();

    current_screen = INIT;
    Serial.println("OLED Initialized");
    return true;
}

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

bool SetupFeedback() {
    // Todo: DAC
    pinMode(BEAT_INIDICATOR_PIN, OUTPUT);
    return true;
}

int availableMemory() {
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void setup() {
    Serial.begin(9600);

    oledInit = SetupOLED();

    delay(200);

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
    isDimmed = false;
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

    if (tapPress == SHORT_PRESS) {
        Serial.println("Short press - Tap");
    }

    if (btnUpPress == SHORT_PRESS) {
        Serial.println("Short press - Button Up");
    }

    if (btnDownPress == SHORT_PRESS) {
        Serial.println("Short press - Button Down");
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
            OLEDModeSelect();
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
                    OLEDNormalDisplay(3);
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
        static bool beatUpdate = true;
        if (beatUpdate) {
            OLEDNormalDisplay(2);
            beatUpdate = false;
        }

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

    // Screen Dim
    if (millis() - lastInteractionTime >= SCREEN_DIM_TIMEOUT && !isDimmed) {
        sevseg.setBrightness(BRIGHTNESS_DIM);
        isDimmed = true;
    }

    sevseg.refreshDisplay();
}