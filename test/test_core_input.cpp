#include "test_framework.h"
#include "PicoDVI.h"
#include "Adafruit_Keypad.h"
#include "types.h"

DVIGFX8 display(DVI_RES_320x240p60, true, adafruit_feather_dvi_cfg);

#include "core_math.ino"
#include "core_time.ino"
#include "core_fx.ino"
#include "core_input.ino"

static void resetInputState() {
  for (int i = 0; i < 16; i++) { pots[i] = 0; keysPressed[i] = false; }
  activeProgram = 0;
  activePreset = 0;
  shiftHeld = false;
  controlHeld = false;
  modAHeld = false;
  k13Holding = false;
  hintsEnabled = false;
  g_fxMode = false;
  g_fxActive = 0;
  keypad.mock_reset();
}

// === potMap ================================================================

void test_potMap_min() {
  resetInputState();
  pots[0] = 0;
  TEST_ASSERT_EQUAL_INT(10, potMap(0, 10, 100));
}

void test_potMap_max() {
  resetInputState();
  pots[0] = 1023;
  TEST_ASSERT_EQUAL_INT(100, potMap(0, 10, 100));
}

void test_potMap_mid() {
  resetInputState();
  pots[5] = 512;
  int expected = (int)map(512L, 0L, 1023L, 10L, 100L);
  TEST_ASSERT_EQUAL_INT(expected, potMap(5, 10, 100));
}

void test_potMap_full_range() {
  resetInputState();
  pots[2] = 0;
  TEST_ASSERT_EQUAL_INT(0, potMap(2, 0, 255));
  pots[2] = 1023;
  TEST_ASSERT_EQUAL_INT(255, potMap(2, 0, 255));
}

// === handleKeypad — preset selection =======================================

void test_keypress_selects_preset() {
  resetInputState();
  // Simulate key 3 pressed: row 0, col 3 → keynum 3
  keypad.mock_pushEvent(0, 3, KEY_JUST_PRESSED);
  handleKeypad();
  TEST_ASSERT_EQUAL_INT(3, activePreset);
  TEST_ASSERT(keysPressed[3]);
}

void test_keyrelease_clears_pressed() {
  resetInputState();
  keysPressed[3] = true;
  keypad.mock_pushEvent(0, 3, KEY_JUST_RELEASED);
  handleKeypad();
  TEST_ASSERT(!keysPressed[3]);
}

void test_shift_selects_secondary_preset() {
  resetInputState();
  keysPressed[KEY_SHIFT] = true;
  shiftHeld = true;
  // Key 2 with shift → preset 14
  keypad.mock_pushEvent(0, 2, KEY_JUST_PRESSED);
  handleKeypad();
  TEST_ASSERT_EQUAL_INT(14, activePreset);
}

void test_control_shift_switches_program() {
  resetInputState();
  keysPressed[KEY_CONTROL] = true;
  keysPressed[KEY_SHIFT] = true;
  controlHeld = true;
  shiftHeld = true;
  // Key 5 with ctrl+shift → program 5, preset reset to 0
  keypad.mock_pushEvent(1, 1, KEY_JUST_PRESSED);  // row 1, col 1 → keynum 5
  handleKeypad();
  TEST_ASSERT_EQUAL_INT(5, activeProgram);
  TEST_ASSERT_EQUAL_INT(0, activePreset);
}

void test_modifier_keys_tracked() {
  resetInputState();
  // Press Shift (key 15): row 3, col 3
  keypad.mock_pushEvent(3, 3, KEY_JUST_PRESSED);
  handleKeypad();
  TEST_ASSERT(shiftHeld);
  TEST_ASSERT(!controlHeld);

  // Release Shift
  keypad.mock_reset();
  keypad.mock_pushEvent(3, 3, KEY_JUST_RELEASED);
  handleKeypad();
  TEST_ASSERT(!shiftHeld);
}

void test_keys_above_11_dont_change_preset() {
  resetInputState();
  activePreset = 7;
  // Key 12 (mod A): row 3, col 0
  keypad.mock_pushEvent(3, 0, KEY_JUST_PRESSED);
  handleKeypad();
  TEST_ASSERT_EQUAL_INT(7, activePreset);
  TEST_ASSERT(keysPressed[12]);
}

// === k13 gesture (hold / double-tap) =======================================

void test_k13_hold_sets_holding() {
  resetInputState();
  mock_set_millis(100);
  // k13 = row 3, col 1
  keypad.mock_pushEvent(3, 1, KEY_JUST_PRESSED);
  handleKeypad();
  updateK13Gesture();
  mock_set_millis(450);  // >300ms later
  updateK13Gesture();
  TEST_ASSERT(k13Holding);
}

void test_k13_release_clears_holding() {
  resetInputState();
  mock_set_millis(100);
  keypad.mock_pushEvent(3, 1, KEY_JUST_PRESSED);
  handleKeypad();
  updateK13Gesture();
  mock_set_millis(450);
  updateK13Gesture();
  TEST_ASSERT(k13Holding);
  keypad.mock_pushEvent(3, 1, KEY_JUST_RELEASED);
  handleKeypad();
  updateK13Gesture();
  TEST_ASSERT(!k13Holding);
}

void test_k13_double_tap_toggles_hints() {
  resetInputState();
  mock_set_millis(100);
  // First tap: press and release (k13 = row 3, col 1)
  keypad.mock_pushEvent(3, 1, KEY_JUST_PRESSED);
  handleKeypad();
  updateK13Gesture();
  mock_set_millis(150);
  keypad.mock_pushEvent(3, 1, KEY_JUST_RELEASED);
  handleKeypad();
  updateK13Gesture();
  TEST_ASSERT(!hintsEnabled);
  // Second tap within 400ms window
  mock_set_millis(250);
  keypad.mock_pushEvent(3, 1, KEY_JUST_PRESSED);
  handleKeypad();
  updateK13Gesture();
  mock_set_millis(300);
  keypad.mock_pushEvent(3, 1, KEY_JUST_RELEASED);
  handleKeypad();
  updateK13Gesture();
  TEST_ASSERT(hintsEnabled);
}

void test_k13_single_tap_no_toggle() {
  resetInputState();
  mock_set_millis(100);
  keypad.mock_pushEvent(3, 1, KEY_JUST_PRESSED);
  handleKeypad();
  updateK13Gesture();
  mock_set_millis(200);
  keypad.mock_pushEvent(3, 1, KEY_JUST_RELEASED);
  handleKeypad();
  updateK13Gesture();
  bool wasOff = !hintsEnabled;
  mock_set_millis(700);  // >400ms later, tap expires
  updateK13Gesture();
  TEST_ASSERT(wasOff && !hintsEnabled);
}

// === FX mode =================================================================

void test_ctrl_k12_toggles_fx_mode() {
  resetInputState();
  keysPressed[KEY_CONTROL] = true;
  controlHeld = true;
  // k12 = row 3, col 0
  keypad.mock_pushEvent(3, 0, KEY_JUST_PRESSED);
  handleKeypad();
  TEST_ASSERT(g_fxMode);

  // Toggle off
  keypad.mock_pushEvent(3, 0, KEY_JUST_RELEASED);
  handleKeypad();
  keypad.mock_pushEvent(3, 0, KEY_JUST_PRESSED);
  handleKeypad();
  TEST_ASSERT(!g_fxMode);
}

void test_fx_mode_keys_toggle_fx() {
  resetInputState();
  g_fxMode = true;
  // Press key 4 (row 1, col 0) → should toggle FX 4 (FX_MIRROR_H)
  keypad.mock_pushEvent(1, 0, KEY_JUST_PRESSED);
  handleKeypad();
  TEST_ASSERT(fxIsActive(4));
  TEST_ASSERT_EQUAL_INT(0, activePreset);  // preset should NOT change
}

void test_fx_mode_doesnt_change_preset() {
  resetInputState();
  g_fxMode = true;
  activePreset = 7;
  // Press key 3 in FX mode
  keypad.mock_pushEvent(0, 3, KEY_JUST_PRESSED);
  handleKeypad();
  TEST_ASSERT_EQUAL_INT(7, activePreset);
  TEST_ASSERT(fxIsActive(3));
}

void test_shift_k12_clears_fx() {
  resetInputState();
  g_fxMode = true;
  g_fxActive = 0x0FFF;  // all 12 FX active
  keysPressed[KEY_SHIFT] = true;
  shiftHeld = true;
  // k12 = row 3, col 0
  keypad.mock_pushEvent(3, 0, KEY_JUST_PRESSED);
  handleKeypad();
  TEST_ASSERT_EQUAL_INT(0, (int)g_fxActive);
}

void test_normal_mode_keys_select_preset() {
  resetInputState();
  g_fxMode = false;
  keypad.mock_pushEvent(0, 2, KEY_JUST_PRESSED);
  handleKeypad();
  TEST_ASSERT_EQUAL_INT(2, activePreset);
  TEST_ASSERT(!fxIsActive(2));
}

// ===========================================================================

TEST_SUITE_BEGIN("core_input")
  RUN_TEST(test_potMap_min);
  RUN_TEST(test_potMap_max);
  RUN_TEST(test_potMap_mid);
  RUN_TEST(test_potMap_full_range);
  RUN_TEST(test_keypress_selects_preset);
  RUN_TEST(test_keyrelease_clears_pressed);
  RUN_TEST(test_shift_selects_secondary_preset);
  RUN_TEST(test_control_shift_switches_program);
  RUN_TEST(test_modifier_keys_tracked);
  RUN_TEST(test_keys_above_11_dont_change_preset);
  RUN_TEST(test_k13_hold_sets_holding);
  RUN_TEST(test_k13_release_clears_holding);
  RUN_TEST(test_k13_double_tap_toggles_hints);
  RUN_TEST(test_k13_single_tap_no_toggle);
  RUN_TEST(test_ctrl_k12_toggles_fx_mode);
  RUN_TEST(test_fx_mode_keys_toggle_fx);
  RUN_TEST(test_fx_mode_doesnt_change_preset);
  RUN_TEST(test_shift_k12_clears_fx);
  RUN_TEST(test_normal_mode_keys_select_preset);
TEST_SUITE_END
