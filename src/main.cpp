#include <Arduino.h>
#include <SevSegShift.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


// --- Mode ---
enum MetronomeMode {
  NORMAL_MODE,
  PROGRAM_MODE,
  TAP_MODE,
  COUNT
};

MetronomeMode current_mode = MetronomeMode::NORMAL_MODE;


char* getCurrentModeText() {
  switch (current_mode) {
    case NORMAL_MODE:
      return "Normal";

    case PROGRAM_MODE:
      return "Program";

    case TAP_MODE:
      return "Tap";

    default:
      return "NaN";
  }
}


// --- OLED ---
enum OLED_State {
  INIT,
  MODE,
  TAP,
  NORM,
  PROG
};

OLED_State oled_state = INIT;

const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT =  32; //64
const int OLED_RESET = -1;

Adafruit_SSD1306 oled_display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- Btn ---
struct ButtonState {
  int pin;
  unsigned long long_press_time = 1000;
  int button_state = HIGH;
  int last_button_state = HIGH;
  unsigned long button_press_time = 0;
  unsigned long last_repeat_time = 0;
  bool is_held = false;
};

enum PressType {
  SHORT,
  LONG,
  NONE
};

// --- Shift Register Control Pins ---
const int SHIFT_PIN_DS = 4;    // DS (Data) of 74HC595
const int SHIFT_PIN_STCP = 5;  // STCP (Latch) of 74HC595
const int SHIFT_PIN_SHCP = 6;  // SHCP (Clock) of 74HC595

// --- Buttons ---
const int BTN_UP_PIN = 8;
const int BTN_DN_PIN = 7;
const int BTN_TAP_PIN = 13;
ButtonState btn_up;
ButtonState btn_down;
ButtonState btn_tap;

const unsigned long debounce_ms = 25;
const unsigned long initial_delay_ms = 500;
const unsigned long repeat_rate_slow = 150;
const unsigned long repeat_rate_fast = 50;

// --- Feedback ---
const int BEAT_LED_PIN = 2;

// --- SevSegShift ---
SevSegShift sev_seg(SHIFT_PIN_DS, SHIFT_PIN_SHCP, SHIFT_PIN_STCP, 1, true);

// --- Vars ---
int current_tempo = 180; // Start Value
constexpr int MAX_TEMPO = 350;
constexpr int MIN_TEMPO = 20;

bool oled_setup = false;
bool seven_seg_setup = false;
bool buttons_setup = false;
bool feedback_setup = false;


void SetupSevenSegment() {
  byte num_digits = 4;
  byte digit_pins[] = {12, 11, 10, 9};

  byte segment_pins[] = { 0, 1, 2, 3, 4, 5, 6, 7 };

  bool resistors_on_segments = true;

  byte hardware_config = COMMON_CATHODE;

  bool update_with_delays = false;
  bool leading_zeros = false;

  sev_seg.begin(hardware_config, num_digits, digit_pins, segment_pins, resistors_on_segments, update_with_delays, leading_zeros, false);

  sev_seg.setBrightness(90);

  sev_seg.setChars("----");
  seven_seg_setup = true;
}

// --- Buttons ---
void SetupButtons() {
  pinMode(BTN_UP_PIN, INPUT_PULLUP);
  pinMode(BTN_DN_PIN, INPUT_PULLUP);
  pinMode(BTN_TAP_PIN, INPUT_PULLUP);

  btn_up.pin = BTN_UP_PIN;
  btn_down.pin = BTN_DN_PIN;
  btn_tap.pin = BTN_TAP_PIN;

  buttons_setup = true;
}

PressType CheckButton(ButtonState &btn)
{
  btn.button_state = digitalRead(btn.pin);

  PressType result = NONE;

  // Button pressed
  if (btn.button_state == LOW && btn.last_button_state == HIGH) {
    btn.button_press_time = millis();
  }

  // Button released
  if (btn.button_state == HIGH && btn.last_button_state == LOW) {
    unsigned long press_duration = millis() - btn.button_press_time;

    if (press_duration >= btn.long_press_time) {
      result = LONG;
    } else {
      result = SHORT;
    }
  }

  btn.last_button_state = btn.button_state;  // Update last state!
  return result;
}

bool CheckRepeat(ButtonState &btn, int &delta) {
  PressType press = CheckButton(btn);

  if (press == SHORT) {
    delta = 1;
    return true;
  }
  if (press == LONG) {
    btn.is_held = true;
    btn.last_repeat_time = millis();
    delta = 1;
    return true;
  }

  if (btn.is_held && btn.button_state == LOW) {
    unsigned long now = millis();
    unsigned long held_for = now - btn.button_press_time;

    unsigned long repeat_rate = (held_for > 2000) ? repeat_rate_fast : repeat_rate_slow;
    if (now - btn.last_repeat_time >= repeat_rate) {
      btn.last_repeat_time = now;
      delta = 1;
      return true;
    }
  }

  if (btn.button_state == HIGH) {
    btn.is_held = false;
  }

  return false;

}


void SetupFeedback() {
  // LED
  pinMode(BEAT_LED_PIN, OUTPUT);

  feedback_setup = true;
}

// --- DISPLAY ---
void StartupDisplay() {
  oled_display.clearDisplay();

  oled_display.setTextColor(1);
  oled_display.setTextWrap(false);
  oled_display.setCursor(20, 38);
  oled_display.print("Initialising...");

  oled_display.setTextSize(2);
  oled_display.setCursor(35, 12);
  oled_display.print("HELLO!");

  oled_display.display();
}

void SetupOLED() {
  if (!oled_display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Failed to initialize display");
    for (;;);
  }

  StartupDisplay();
  delay(2000);
  oled_display.clearDisplay();
  oled_display.setTextColor(SSD1306_WHITE);
  oled_display.setTextSize(1);
  oled_display.setCursor(0, 0);

  oled_state = INIT;
  oled_setup = true;

}

void OLEDModeDisplay() {
  if (oled_state != MODE) {
    oled_display.clearDisplay();

    oled_display.setTextColor(1);
    oled_display.setTextWrap(false);
    oled_display.setCursor(32, 4);
    oled_display.print("Select Mode");

    oled_display.drawLine(16, 15, 111, 15, 1);

    oled_display.setCursor(5, 39);
    oled_display.print("Press Tap to confirm");

    oled_display.setCursor(14, 29);
    oled_display.print("Use +/- to change");

    oled_display.display();
    oled_state = MODE;
  }
}

void OLEDNormDisplay() {
  oled_state = OLED_State::NORM;

  oled_display.clearDisplay();

  oled_display.setTextColor(1);
  oled_display.setTextWrap(false);
  oled_display.setCursor(2, 55);
  oled_display.print("Mode:");

  oled_display.setCursor(32, 55);
  oled_display.print(getCurrentModeText());

  oled_display.fillCircle(17, 32, 8, 1);

  oled_display.drawCircle(79, 32, 8, 1);

  oled_display.drawCircle(110, 32, 8, 1);

  oled_display.drawCircle(48, 32, 8, 1);

  oled_display.display();
}

void OLEDProgramDisplay() {

}

void OLEDTapDisplay() {

}




// --- MAIN ---

void setup() {
  SetupOLED();
  SetupSevenSegment();
  SetupButtons();
  SetupFeedback();

  Serial.begin(9600);
}

void loop() {

  if (oled_setup && seven_seg_setup && buttons_setup && feedback_setup && oled_state == INIT) {

    OLEDModeDisplay();
  }

  if (CheckButton(btn_tap) == LONG) {
    sev_seg.setChars("----");
    OLEDModeDisplay();
  }

  if (oled_state == MODE) {
    if (CheckButton(btn_up) == SHORT)
    {
      current_mode = static_cast<MetronomeMode>((static_cast<int>(current_mode) + 1) % static_cast<int>(MetronomeMode::COUNT));
      sev_seg.setChars(getCurrentModeText());
    }
    else if (CheckButton(btn_down) == SHORT){
      current_mode = static_cast<MetronomeMode>((static_cast<int>(current_mode) - 1 + static_cast<int>(MetronomeMode::COUNT)) % static_cast<int>(MetronomeMode::COUNT));
      sev_seg.setChars(getCurrentModeText());
    }
    else if (CheckButton(btn_tap) == SHORT) {
      sev_seg.setChars("----");

      if (current_mode == MetronomeMode::NORMAL_MODE) {
        OLEDNormDisplay();
      }
      else if (current_mode == MetronomeMode::PROGRAM_MODE) {
        OLEDProgramDisplay();
      }
      else if (current_mode == MetronomeMode::TAP_MODE) {
        OLEDTapDisplay();
      } else {
        sev_seg.setChars("Err!");
      }

    }

  }

  if (oled_state == NORM) {
    int delta = 0;

    if (CheckRepeat(btn_up, delta)) {
      current_tempo = min(current_tempo + delta, MAX_TEMPO);
    }
    if (CheckRepeat(btn_down, delta)) {
      current_tempo = max(current_tempo - delta, MIN_TEMPO);
    }

    sev_seg.setChars("180");

    Serial.println(current_tempo);
  }

  sev_seg.refreshDisplay();
}


