#ifndef VIDEOGEM_TYPES_H
#define VIDEOGEM_TYPES_H

// =====================================================================
// VIDEO GEM — Shared types, defines, and pin assignments
// =====================================================================

#include <PicoDVI.h>
#include <math.h>
#include "Adafruit_Keypad.h"
#include <Arduino.h>

// ─── Display ─────────────────────────────────────────────────────────
#define W 320
#define H 240
#define HALFW 160
#define HALFH 120

// ─── Pins: Potentiometer ADC + Multiplexer ──────────────────────────
#define POT_A_PIN A0
#define POT_B_PIN A1
#define POT_C_PIN A2
#define POT_D_PIN A3
#define MUX_CTRL_0 24
#define MUX_CTRL_1 25

// ─── Pins: Keypad matrix ────────────────────────────────────────────
#define KP_ROWS 4
#define KP_COLS 4

// ─── Key roles ──────────────────────────────────────────────────────
#define KEY_MOD_A    12
#define KEY_MOD_B    13
#define KEY_CONTROL  14
#define KEY_SHIFT    15

// ─── Math ───────────────────────────────────────────────────────────
#define SIN_TAB 256

// ─── LFO (low-frequency oscillator) ─────────────────────────────
#define NUM_LFOS 4

struct LFO {
  uint32_t phase;   // 16.16 fixed-point, wraps at 0x10000
  uint16_t freq;    // 8.8 fixed-point Hz (0x0100 = 1 Hz)
};

// ─── Palette ────────────────────────────────────────────────────────
#define PAL_ENTRIES 254
#define NUM_PALETTES 6

struct RGB { uint8_t r, g, b; };

// ─── Max programs and presets ───────────────────────────────────────
#define MAX_PROGRAMS 12
#define MAX_PRESETS  24

// ─── Program interface ──────────────────────────────────────────────
// Each program provides draw, renderHint, name, character, presetName, potLabel.
// The router uses the program registry (prog_registry.ino) to dispatch.
// Programs read globals (pots[], keysPressed[], globalTime, etc.) directly.

// ─── Program registry (populated by prog_registry.ino) ───────────────
typedef struct {
  int slot;
  void (*draw)(int preset);
  uint8_t (*renderHint)(int preset);
  const char* (*name)(void);
  const char* (*character)(void);
  const char* (*presetName)(int preset);
  const char* (*potLabel)(int preset, int pot);
  void (*init)(void);    // optional: called on program switch (NULL = no-op)
  uint8_t flags;
} ProgEntry;

#define PROG_FLAG_OWNS_GLOBALS 1  // program manages p0–p3 itself (skips palette/speed/trail)

extern ProgEntry g_programs[];
extern int g_numPrograms;
ProgEntry* progForSlot(int slot);

// ─── Render hint flags (returned by programs to guide the router) ───
#define RENDER_CLEAR    0   // fill screen with black before draw
#define RENDER_TRAIL    1   // fade previous frame (trail amount from p2)
#define RENDER_PERPIXEL 2   // no clear — program fills every pixel

// ─── Globals defined in core_input.ino (declared here for all .ino files) ──
extern Adafruit_Keypad keypad;
extern int pots[];
extern bool keysPressed[];
extern int potIntervalMS;
extern unsigned long nextPotUpdateTime;
extern int currentPalette;
extern int paletteOffset;
extern float globalTime;
extern int activeProgram;
extern int activePreset;
extern bool shiftHeld;
extern bool k13Holding;
extern bool hintsEnabled;

// ─── LFO globals (defined in core_time.ino) ─────────────────────
extern LFO g_lfos[NUM_LFOS];

// ─── Global FX layer (defined in core_fx.ino) ───────────────────
// FX identifiers — grouped by mechanism
// Canvas FX (draw primitives)
#define FX_CUTFADE    0
#define FX_DOTFADE    1
// Framebuffer FX — pre-draw (index shift)
#define FX_DESCEND    2
#define FX_ASCEND     3
// Framebuffer FX — post-draw (spatial transform + temporal blend)
#define FX_MIRROR_H   4
#define FX_MIRROR_V   5
#define FX_MIRROR_QUAD 6
#define FX_ECHO       7
// Palette FX (color LUT transform)
#define FX_INVERT     8
#define FX_REVERSE    9
#define FX_WIPE       10
#define FX_DROPOUTS   11
#define FX_COUNT      12

extern bool     g_fxMode;
extern uint16_t g_fxActive;

#endif
