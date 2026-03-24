// =====================================================================
// CORE: Input — pot scanning, keypad handling, program switching
// =====================================================================

// ─── Globals ─────────────────────────────────────────────────────────
int pots[16] = {0};
int potIntervalMS = 16;
unsigned long nextPotUpdateTime = 0;

int keys_map[KP_ROWS][KP_COLS] = {
  {0, 1, 2, 3},
  {4, 5, 6, 7},
  {8, 9, 10, 11},
  {12, 13, 14, 15}
};
uint8_t rowPins[KP_ROWS] = {13, 12, 11, 0};
uint8_t colPins[KP_COLS] = {10, 9, 6, 5};
bool keysPressed[16] = {false};

Adafruit_Keypad keypad = Adafruit_Keypad(makeKeymap(keys_map), rowPins, colPins, KP_ROWS, KP_COLS);

// ─── State ───────────────────────────────────────────────────────────
int activeProgram = 0;
int activePreset = 0;
bool shiftHeld = false;
bool controlHeld = false;
bool modAHeld = false;
int currentPalette = 0;
int paletteOffset = 0;
float globalTime = 0;

// ─── k13 gesture detection ──────────────────────────────────────────
// k13 is a system-level "Info" key:
//   Hold >300ms  → k13Holding = true (full info overlay)
//   Double-tap   → toggle hintsEnabled (contextual tooltips)
bool k13Holding = false;
bool hintsEnabled = false;

static uint8_t k13GState = 0;  // 0=idle, 1=tap_pending, 2=holding
static unsigned long k13DownTime = 0;
static unsigned long k13LastTapTime = 0;

void updateK13Gesture() {
  bool pressed = keysPressed[KEY_MOD_B];
  unsigned long now = millis();

  if (pressed && k13DownTime == 0) {
    k13DownTime = now;
  }

  if (pressed && k13DownTime > 0 && now - k13DownTime > 300) {
    k13GState = 2;
    k13Holding = true;
  }

  if (!pressed && k13DownTime > 0) {
    if (k13GState == 2) {
      k13Holding = false;
      k13GState = 0;
    } else if (now - k13DownTime < 300) {
      if (k13GState == 1 && now - k13LastTapTime < 400) {
        hintsEnabled = !hintsEnabled;
        k13GState = 0;
      } else {
        k13GState = 1;
        k13LastTapTime = now;
      }
    }
    k13DownTime = 0;
  }

  if (k13GState == 1 && now - k13LastTapTime > 400) {
    k13GState = 0;
  }
}

// ─── Pot reading ─────────────────────────────────────────────────────
static inline int potEma(int prev, int raw) {
  return (prev * 3 + raw + 2) >> 2;
}

void updatePots() {
  int sum, samples = 4;
  for (int i = 0; i < 4; i++) {
    digitalWrite(MUX_CTRL_0, i & 1);
    digitalWrite(MUX_CTRL_1, i & 2);
    delayMicroseconds(50);

    sum = 0;
    for (int s = 0; s < samples; s++) sum += analogRead(POT_A_PIN);
    pots[i] = potEma(pots[i], 1023 - sum / samples);

    sum = 0;
    for (int s = 0; s < samples; s++) sum += analogRead(POT_B_PIN);
    pots[i + 4] = potEma(pots[i + 4], 1023 - sum / samples);

    sum = 0;
    for (int s = 0; s < samples; s++) sum += analogRead(POT_C_PIN);
    pots[i + 8] = potEma(pots[i + 8], 1023 - sum / samples);

    sum = 0;
    for (int s = 0; s < samples; s++) sum += analogRead(POT_D_PIN);
    pots[i + 12] = potEma(pots[i + 12], 1023 - sum / samples);
  }
}

int potMap(int idx, int lo, int hi) {
  return map(pots[idx], 0, 1023, lo, hi);
}

// ─── Keypad handling with program/preset/shift/control logic ────────
void handleKeypad() {
  while (keypad.available()) {
    keypadEvent e = keypad.read();
    int keynum = e.bit.ROW * KP_COLS + e.bit.COL;

    if (e.bit.EVENT == KEY_JUST_PRESSED) {
      keysPressed[keynum] = true;

      // Ctrl + k12: toggle FX mode
      if (keynum == KEY_MOD_A && controlHeld) {
        g_fxMode = !g_fxMode;
      }
      // Shift + k12 in FX mode: clear all FX
      else if (keynum == KEY_MOD_A && shiftHeld && g_fxMode) {
        fxClearAll();
      }
      else if (keynum <= 11) {
        if (controlHeld && shiftHeld) {
          // k14+k15+k0..k11 = switch program
          activeProgram = keynum;
          activePreset = 0;
        } else if (g_fxMode) {
          // FX mode: k0..k11 = toggle FX
          fxToggle(keynum);
        } else if (shiftHeld) {
          // k15+k0..k11 = secondary presets 12..23
          activePreset = keynum + 12;
        } else {
          // k0..k11 = primary presets 0..11
          activePreset = keynum;
        }
      }
    } else if (e.bit.EVENT == KEY_JUST_RELEASED) {
      keysPressed[keynum] = false;
    }
  }

  shiftHeld   = keysPressed[KEY_SHIFT];
  controlHeld = keysPressed[KEY_CONTROL];
  modAHeld    = keysPressed[KEY_MOD_A];
}
