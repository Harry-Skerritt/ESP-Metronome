#include <Arduino.h>
#include <SevSegShift.h>

struct ButtonState {
  int pin;
  int stable_state;
  int last_reading;
  unsigned long last_change;
  unsigned long hold_start;
  unsigned long last_repeat;
  bool held;
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



void SetupSevenSegment() {
  byte num_digits = 4;
  byte digit_pins[] = {12, 11, 10, 9};

  byte segment_pins[] = { 0, 1, 2, 3, 4, 5, 6, 7 };

  bool resistors_on_segments = true;

  byte hardware_config = COMMON_CATHODE;

  bool update_with_delays = false;
  bool leading_zeros = false;

  sev_seg.begin(hardware_config, num_digits, digit_pins, segment_pins, resistors_on_segments, update_with_delays, leading_zeros, true);

  sev_seg.setBrightness(90);
}

// --- Buttons ---
void SetupButtons() {
  pinMode(BTN_UP_PIN, INPUT_PULLUP);
  pinMode(BTN_DN_PIN, INPUT_PULLUP);
  pinMode(BTN_TAP_PIN, INPUT_PULLUP);

  btn_up = {BTN_UP_PIN, HIGH, HIGH, 0, 0, 0, false};
  btn_down = {BTN_DN_PIN, HIGH, HIGH, 0, 0, 0, false};
  btn_tap = {BTN_TAP_PIN, HIGH, HIGH, 0, 0, 0, false};
}

void HandleButton(ButtonState &btn, int direction)
{
  int reading = digitalRead(btn.pin);
  if (reading != btn.last_reading) {
    btn.last_change = millis();
  }

  if (millis() - btn.last_change > debounce_ms && reading != btn.stable_state) {
    btn.stable_state = reading;
    if (btn.stable_state == LOW) { // pressed
      if ((direction > 0 && current_tempo < MAX_TEMPO) ||
          (direction < 0 && current_tempo > MIN_TEMPO))
        {
          current_tempo += direction;
        }
      btn.held = false;
      btn.hold_start = millis();
      btn.last_repeat = millis();
    }
  }

  if (btn.stable_state == LOW) {
    if (!btn.held && millis() - btn.hold_start > initial_delay_ms) {
      btn.held = true;
      btn.last_repeat = millis();
    }
    if (btn.held) {
      unsigned long rate = (millis() - btn.hold_start > 2000) ? repeat_rate_fast: repeat_rate_slow;
      if (millis() - btn.last_repeat > rate) {
        if ((direction > 0 && current_tempo < MAX_TEMPO) ||
            (direction < 0 && current_tempo > MIN_TEMPO))
        {
          current_tempo += direction;
        }
        btn.last_repeat = millis();
      }
    }
  }

  btn.last_reading = reading;
}

void CheckButtons() {
  HandleButton(btn_up, +1);
  HandleButton(btn_down, -1);
}


void SetupFeedback() {
  // LED
  pinMode(BEAT_LED_PIN, OUTPUT);
}

void setup() {
  SetupSevenSegment();
  SetupButtons();
  SetupFeedback();
}

void loop() {

  CheckButtons();

  sev_seg.setNumber(current_tempo, 0);
  sev_seg.refreshDisplay();
}