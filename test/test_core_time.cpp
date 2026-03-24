#include "test_framework.h"
#include "types.h"

#include "core_math.ino"
#include "core_time.ino"

// === normalizedTime ========================================================

void test_normalizedTime_start() {
  mock_set_millis(0);
  float t = normalizedTime(1000, 0);
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, t);
}

void test_normalizedTime_mid() {
  mock_set_millis(500);
  float t = normalizedTime(1000, 0);
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.5f, t);
}

void test_normalizedTime_wraps() {
  mock_set_millis(1500);
  float t = normalizedTime(1000, 0);
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.5f, t);
}

void test_normalizedTime_phase_offset() {
  mock_set_millis(0);
  // phase 0.25 shifts by 250ms on a 1000ms period
  float t = normalizedTime(1000, 0.25f);
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.25f, t);
}

void test_normalizedTime_range() {
  for (unsigned long ms = 0; ms < 2000; ms += 37) {
    mock_set_millis(ms);
    float t = normalizedTime(1000, 0);
    TEST_ASSERT(t >= 0.0f && t < 1.0f);
  }
}

// === pingPongTime ==========================================================

void test_pingPong_at_zero() {
  mock_set_millis(0);
  float t = pingPongTime(1000, 0);
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, t);
}

void test_pingPong_at_quarter() {
  mock_set_millis(250);
  float t = pingPongTime(1000, 0);
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.5f, t);
}

void test_pingPong_at_half() {
  mock_set_millis(500);
  float t = pingPongTime(1000, 0);
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.0f, t);
}

void test_pingPong_at_three_quarter() {
  mock_set_millis(750);
  float t = pingPongTime(1000, 0);
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.5f, t);
}

void test_pingPong_symmetric() {
  // ping-pong should be symmetric: pp(t) == pp(period-t)
  for (int ms = 1; ms < 500; ms += 47) {
    mock_set_millis(ms);
    float a = pingPongTime(1000, 0);
    mock_set_millis(1000 - ms);
    float b = pingPongTime(1000, 0);
    TEST_ASSERT_FLOAT_WITHIN(0.02f, a, b);
  }
}

void test_pingPong_range() {
  for (unsigned long ms = 0; ms < 2000; ms += 13) {
    mock_set_millis(ms);
    float t = pingPongTime(1000, 0);
    TEST_ASSERT(t >= 0.0f && t <= 1.01f);
  }
}

// === LFO ===================================================================

static void lfo_reset() {
  initSinTable();
  for (int i = 0; i < NUM_LFOS; i++) {
    g_lfos[i].phase = 0;
    g_lfos[i].freq = 0;
  }
  _lfoLastMicros = 0;
  mock_set_micros(0);
}

void test_lfo_initial_zero() {
  lfo_reset();
  TEST_ASSERT_EQUAL_INT(128, lfoSine(0, 0));      // sin(0) = 0 → midpoint 128
  TEST_ASSERT_EQUAL_INT(0, lfoTriangle(0, 0));     // triangle starts at 0
  TEST_ASSERT_EQUAL_INT(255, lfoSquare(0, 0));     // square starts high
  TEST_ASSERT_EQUAL_INT(0, lfoSaw(0, 0));          // saw starts at 0
  TEST_ASSERT_EQUAL_INT(255, lfoRamp(0, 0));       // ramp starts at 255
}

void test_lfo_bipolar_initial() {
  lfo_reset();
  TEST_ASSERT_EQUAL_INT(0, lfoSineBi(0, 0));
  TEST_ASSERT_EQUAL_INT(-128, lfoTriangleBi(0, 0));
  TEST_ASSERT_EQUAL_INT(127, lfoSquareBi(0, 0));
  TEST_ASSERT_EQUAL_INT(-128, lfoSawBi(0, 0));
  TEST_ASSERT_EQUAL_INT(127, lfoRampBi(0, 0));
}

void test_lfo_phase_advances() {
  lfo_reset();
  g_lfos[0].freq = 0x0100;   // 1 Hz
  mock_set_micros(0);
  updateLFOs();               // initialize lastMicros

  mock_set_micros(500000);    // advance 0.5 seconds → phase should be ~0x8000 (half cycle)
  updateLFOs();

  uint8_t p = (uint8_t)(g_lfos[0].phase >> 8);
  TEST_ASSERT(p >= 120 && p <= 136); // roughly 128 (halfway through 256)
}

void test_lfo_phase_wraps() {
  lfo_reset();
  g_lfos[0].freq = 0x0100;   // 1 Hz
  mock_set_micros(0);
  updateLFOs();

  mock_set_micros(1000000);   // 1 full second → should wrap back near 0
  updateLFOs();

  uint8_t p = (uint8_t)(g_lfos[0].phase >> 8);
  TEST_ASSERT(p < 16 || p > 240); // near 0 (with fixed-point rounding tolerance)
}

void test_lfo_phase_offset() {
  lfo_reset();
  // At phase 0, sine with offset 64 (quarter cycle) should be near peak (255)
  uint8_t val = lfoSine(0, 64);
  TEST_ASSERT(val > 240);
}

void test_lfo_independent_frequencies() {
  lfo_reset();
  g_lfos[0].freq = 0x0100;   // 1 Hz
  g_lfos[1].freq = 0x0200;   // 2 Hz
  mock_set_micros(0);
  updateLFOs();

  mock_set_micros(250000);    // 0.25 seconds
  updateLFOs();

  // LFO 0 at 1 Hz: 0.25 of cycle → phase ~64
  // LFO 1 at 2 Hz: 0.50 of cycle → phase ~128
  uint8_t p0 = (uint8_t)(g_lfos[0].phase >> 8);
  uint8_t p1 = (uint8_t)(g_lfos[1].phase >> 8);
  TEST_ASSERT(p0 >= 56 && p0 <= 72);
  TEST_ASSERT(p1 >= 120 && p1 <= 136);
}

void test_lfo_sine_range() {
  lfo_reset();
  for (int p = 0; p < 256; p++) {
    g_lfos[0].phase = (uint32_t)p << 8;
    uint8_t val = lfoSine(0, 0);
    TEST_ASSERT(val <= 255); // always valid uint8_t
    int8_t bval = lfoSineBi(0, 0);
    TEST_ASSERT(bval >= -128 && bval <= 127);
  }
}

void test_lfo_triangle_symmetry() {
  lfo_reset();
  // triangle(64) should equal triangle(192-1) approximately (symmetric around peak)
  g_lfos[0].phase = 64 << 8;
  uint8_t a = lfoTriangle(0, 0);
  g_lfos[0].phase = (256 - 64) << 8;
  uint8_t b = lfoTriangle(0, 0);
  // Should be roughly equal (within fixed-point tolerance)
  int diff = (int)a - (int)b;
  TEST_ASSERT(abs(diff) <= 4);
}

void test_lfo_square_half() {
  lfo_reset();
  // First half: high, second half: low
  g_lfos[0].phase = 0;
  TEST_ASSERT_EQUAL_INT(255, lfoSquare(0, 0));
  g_lfos[0].phase = 64 << 8;
  TEST_ASSERT_EQUAL_INT(255, lfoSquare(0, 0));
  g_lfos[0].phase = 128 << 8;
  TEST_ASSERT_EQUAL_INT(0, lfoSquare(0, 0));
  g_lfos[0].phase = 200 << 8;
  TEST_ASSERT_EQUAL_INT(0, lfoSquare(0, 0));
}

// ===========================================================================

TEST_SUITE_BEGIN("core_time")
  RUN_TEST(test_normalizedTime_start);
  RUN_TEST(test_normalizedTime_mid);
  RUN_TEST(test_normalizedTime_wraps);
  RUN_TEST(test_normalizedTime_phase_offset);
  RUN_TEST(test_normalizedTime_range);
  RUN_TEST(test_pingPong_at_zero);
  RUN_TEST(test_pingPong_at_quarter);
  RUN_TEST(test_pingPong_at_half);
  RUN_TEST(test_pingPong_at_three_quarter);
  RUN_TEST(test_pingPong_symmetric);
  RUN_TEST(test_pingPong_range);
  RUN_TEST(test_lfo_initial_zero);
  RUN_TEST(test_lfo_bipolar_initial);
  RUN_TEST(test_lfo_phase_advances);
  RUN_TEST(test_lfo_phase_wraps);
  RUN_TEST(test_lfo_phase_offset);
  RUN_TEST(test_lfo_independent_frequencies);
  RUN_TEST(test_lfo_sine_range);
  RUN_TEST(test_lfo_triangle_symmetry);
  RUN_TEST(test_lfo_square_half);
TEST_SUITE_END
