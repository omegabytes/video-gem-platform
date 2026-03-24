#ifndef ARDUINO_H_MOCK
#define ARDUINO_H_MOCK

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef uint8_t byte;
typedef bool boolean;

#define PI 3.14159265358979323846f
#define TWO_PI (2.0f * PI)
#define HALF_PI (0.5f * PI)
#define DEG_TO_RAD (PI / 180.0f)
#define RAD_TO_DEG (180.0f / PI)

#define OUTPUT 1
#define INPUT  0
#define INPUT_PULLUP 2
#define HIGH 1
#define LOW  0
#define LED_BUILTIN 13

#define A0 26
#define A1 27
#define A2 28
#define A3 29

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define constrain(x,lo,hi) ((x)<(lo)?(lo):((x)>(hi)?(hi):(x)))

// --- Controllable millis / micros ------------------------------------------
static unsigned long _mock_millis = 0;
static unsigned long _mock_micros = 0;
inline unsigned long millis() { return _mock_millis; }
inline unsigned long micros() { return _mock_micros; }
inline void mock_set_millis(unsigned long ms) { _mock_millis = ms; }
inline void mock_set_micros(unsigned long us) { _mock_micros = us; }

// --- Arduino map -----------------------------------------------------------
inline long map(long x, long in_min, long in_max, long out_min, long out_max) {
  if (in_max == in_min) return out_min;
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// --- I/O stubs with controllable analog reads ------------------------------
static int _mock_analog[32] = {};
inline void  pinMode(int, int) {}
inline void  digitalWrite(int, int) {}
inline int   digitalRead(int) { return LOW; }
inline int   analogRead(int pin) { return _mock_analog[pin & 31]; }
inline void  mock_set_analog(int pin, int val) { _mock_analog[pin & 31] = val; }
inline void  delayMicroseconds(unsigned int) {}
inline void  delay(unsigned long) {}

// --- Random ----------------------------------------------------------------
inline void randomSeed(unsigned long s) { srand((unsigned)s); }
inline long random(long hi) { return (hi <= 0) ? 0 : (rand() % hi); }
inline long random(long lo, long hi) { return (hi <= lo) ? lo : lo + (rand() % (hi - lo)); }

#endif
