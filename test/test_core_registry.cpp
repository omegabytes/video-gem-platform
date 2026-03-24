#include "test_framework.h"
#include "PicoDVI.h"
#include "Adafruit_Keypad.h"
#include "types.h"

DVIGFX8 display(DVI_RES_320x240p60, true, adafruit_feather_dvi_cfg);

// Mock program functions for testing
static void mock_draw(int) {}
static uint8_t mock_renderHint(int) { return RENDER_CLEAR; }
static const char* mock_name()  { return "MOCK"; }
static const char* mock_char()  { return "test"; }
static const char* mock_preset(int) { return "p0"; }
static const char* mock_pot(int, int) { return "pot"; }

static int mock_init_called = 0;
static void mock_init() { mock_init_called++; }

ProgEntry g_programs[] = {
  {0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0},                                        // slot 0: null stub
  {3, mock_draw, mock_renderHint, mock_name, mock_char, mock_preset, mock_pot, NULL, 0},   // slot 3: normal
  {5, mock_draw, mock_renderHint, mock_name, mock_char, mock_preset, mock_pot, mock_init, PROG_FLAG_OWNS_GLOBALS}, // slot 5: with init + flags
};
int g_numPrograms = sizeof(g_programs) / sizeof(g_programs[0]);

#include "core_registry.ino"

// ─── g_numPrograms ──────────────────────────────────────────────────

void test_numPrograms_correct() {
  TEST_ASSERT_EQUAL_INT(3, g_numPrograms);
}

// ─── progForSlot: valid slot ────────────────────────────────────────

void test_progForSlot_valid() {
  ProgEntry* p = progForSlot(3);
  TEST_ASSERT(p != NULL);
  TEST_ASSERT_EQUAL_INT(3, p->slot);
  TEST_ASSERT_STR_EQUAL("MOCK", p->name());
}

// ─── progForSlot: slot with null draw (stub) is skipped ─────────────

void test_progForSlot_null_draw_skipped() {
  ProgEntry* p = progForSlot(0);
  TEST_ASSERT(p == NULL);
}

// ─── progForSlot: nonexistent slot returns NULL ─────────────────────

void test_progForSlot_missing_slot() {
  TEST_ASSERT(progForSlot(99) == NULL);
  TEST_ASSERT(progForSlot(-1) == NULL);
}

// ─── flags: PROG_FLAG_OWNS_GLOBALS ─────────────────────────────────

void test_flags_owns_globals() {
  ProgEntry* p = progForSlot(5);
  TEST_ASSERT(p != NULL);
  TEST_ASSERT(p->flags & PROG_FLAG_OWNS_GLOBALS);

  ProgEntry* p2 = progForSlot(3);
  TEST_ASSERT(p2 != NULL);
  TEST_ASSERT(!(p2->flags & PROG_FLAG_OWNS_GLOBALS));
}

// ─── init callback ──────────────────────────────────────────────────

void test_init_callback() {
  mock_init_called = 0;
  ProgEntry* p = progForSlot(5);
  TEST_ASSERT(p != NULL);
  TEST_ASSERT(p->init != NULL);
  p->init();
  TEST_ASSERT_EQUAL_INT(1, mock_init_called);
}

void test_init_null_when_not_set() {
  ProgEntry* p = progForSlot(3);
  TEST_ASSERT(p != NULL);
  TEST_ASSERT(p->init == NULL);
}

// ─── PROG_ENTRY macro ──────────────────────────────────────────────

static void macro_test_draw(int) {}
static uint8_t macro_test_renderHint(int) { return RENDER_TRAIL; }
static const char* macro_test_name()  { return "MACRO"; }
static const char* macro_test_character()  { return "macro test"; }
static const char* macro_test_presetName(int) { return "m0"; }
static const char* macro_test_potLabel(int, int) { return "mpot"; }

void test_prog_entry_macro() {
  ProgEntry e = PROG_ENTRY(7, macro_test);
  TEST_ASSERT_EQUAL_INT(7, e.slot);
  TEST_ASSERT(e.draw == macro_test_draw);
  TEST_ASSERT_STR_EQUAL("MACRO", e.name());
  TEST_ASSERT(e.init == NULL);
  TEST_ASSERT_EQUAL_INT(0, e.flags);
}

void test_prog_entry_ex_macro() {
  ProgEntry e = PROG_ENTRY_EX(9, macro_test, mock_init, PROG_FLAG_OWNS_GLOBALS);
  TEST_ASSERT_EQUAL_INT(9, e.slot);
  TEST_ASSERT(e.draw == macro_test_draw);
  TEST_ASSERT(e.init == mock_init);
  TEST_ASSERT(e.flags & PROG_FLAG_OWNS_GLOBALS);
}

// ===========================================================================

TEST_SUITE_BEGIN("core_registry")
  RUN_TEST(test_numPrograms_correct);
  RUN_TEST(test_progForSlot_valid);
  RUN_TEST(test_progForSlot_null_draw_skipped);
  RUN_TEST(test_progForSlot_missing_slot);
  RUN_TEST(test_flags_owns_globals);
  RUN_TEST(test_init_callback);
  RUN_TEST(test_init_null_when_not_set);
  RUN_TEST(test_prog_entry_macro);
  RUN_TEST(test_prog_entry_ex_macro);
TEST_SUITE_END
