#ifndef ADAFRUIT_KEYPAD_H_MOCK
#define ADAFRUIT_KEYPAD_H_MOCK

#include "Arduino.h"

#define KEY_JUST_PRESSED  1
#define KEY_JUST_RELEASED 2

#define makeKeymap(x) ((byte*)(x))

struct keypadEvent {
  struct { uint8_t ROW; uint8_t COL; uint8_t EVENT; } bit;
};

class Adafruit_Keypad {
public:
  Adafruit_Keypad(byte*, byte*, byte*, int, int) {}
  void begin() {}
  void tick() {}

  bool available() { return _idx < _cnt; }
  keypadEvent read() { return _events[_idx++]; }

  // --- Test helpers: enqueue synthetic events ------------------------------
  void mock_pushEvent(uint8_t row, uint8_t col, uint8_t event) {
    if (_cnt < 32) {
      _events[_cnt].bit.ROW = row;
      _events[_cnt].bit.COL = col;
      _events[_cnt].bit.EVENT = event;
      _cnt++;
    }
  }
  void mock_reset() { _idx = 0; _cnt = 0; }

private:
  keypadEvent _events[32] = {};
  int _idx = 0;
  int _cnt = 0;
};

#endif
