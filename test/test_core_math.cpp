#include "test_framework.h"
#include "types.h"

#include "core_math.ino"

// === initSinTable ==========================================================

void test_sinTable_midpoint_at_zero() {
  initSinTable();
  // sin(0) = 0 → 128 + 127*0 = 128
  TEST_ASSERT_EQUAL_INT(128, sinTab[0]);
}

void test_sinTable_peak_at_quarter() {
  initSinTable();
  // sin(π/2) = 1 → 128 + 127 = 255
  TEST_ASSERT_EQUAL_INT(255, sinTab[SIN_TAB / 4]);
}

void test_sinTable_midpoint_at_half() {
  initSinTable();
  // sin(π) = 0 → ~128
  int val = sinTab[SIN_TAB / 2];
  TEST_ASSERT(val >= 127 && val <= 129);
}

void test_sinTable_trough_at_three_quarter() {
  initSinTable();
  // sin(3π/2) = -1 → 128 - 127 = 1
  TEST_ASSERT_EQUAL_INT(1, sinTab[3 * SIN_TAB / 4]);
}

void test_sinTable_all_in_range() {
  initSinTable();
  for (int i = 0; i < SIN_TAB; i++) {
    TEST_ASSERT(sinTab[i] >= 1 && sinTab[i] <= 255);
  }
}

void test_sinTable_symmetry() {
  initSinTable();
  // sin(x) symmetric about quarter: tab[i] + tab[i+128] ≈ 256
  for (int i = 0; i < SIN_TAB / 2; i++) {
    int sum = (int)sinTab[i] + (int)sinTab[i + SIN_TAB / 2];
    TEST_ASSERT(sum >= 254 && sum <= 258);
  }
}

// === fastAngle =============================================================

void test_fastAngle_origin() {
  TEST_ASSERT_EQUAL_INT(0, fastAngle(0, 0));
}

void test_fastAngle_east() {
  // Positive x, zero y → 0°
  uint8_t a = fastAngle(100, 0);
  TEST_ASSERT(a < 4);
}

void test_fastAngle_north() {
  // Zero x, positive y → 64 (90°)
  uint8_t a = fastAngle(0, 100);
  TEST_ASSERT(a >= 60 && a <= 68);
}

void test_fastAngle_west() {
  // Negative x, zero y → 128 (180°)
  uint8_t a = fastAngle(-100, 0);
  TEST_ASSERT(a >= 124 && a <= 132);
}

void test_fastAngle_south() {
  // Zero x, negative y → 192 (270°)
  uint8_t a = fastAngle(0, -100);
  TEST_ASSERT(a >= 188 && a <= 196);
}

void test_fastAngle_diagonal_ne() {
  // 45° → ~32
  uint8_t a = fastAngle(100, 100);
  TEST_ASSERT(a >= 28 && a <= 36);
}

void test_fastAngle_diagonal_sw() {
  // 225° → ~160
  uint8_t a = fastAngle(-100, -100);
  TEST_ASSERT(a >= 156 && a <= 164);
}

void test_fastAngle_monotonic_first_quadrant() {
  // As angle increases from 0° to 90°, fastAngle should increase
  uint8_t prev = fastAngle(100, 0);
  for (int step = 1; step <= 10; step++) {
    int dx = (int)(100 * cosf(step * HALF_PI / 10));
    int dy = (int)(100 * sinf(step * HALF_PI / 10));
    uint8_t cur = fastAngle(dx, dy);
    TEST_ASSERT(cur >= prev);
    prev = cur;
  }
}

// ===========================================================================

TEST_SUITE_BEGIN("core_math")
  RUN_TEST(test_sinTable_midpoint_at_zero);
  RUN_TEST(test_sinTable_peak_at_quarter);
  RUN_TEST(test_sinTable_midpoint_at_half);
  RUN_TEST(test_sinTable_trough_at_three_quarter);
  RUN_TEST(test_sinTable_all_in_range);
  RUN_TEST(test_sinTable_symmetry);
  RUN_TEST(test_fastAngle_origin);
  RUN_TEST(test_fastAngle_east);
  RUN_TEST(test_fastAngle_north);
  RUN_TEST(test_fastAngle_west);
  RUN_TEST(test_fastAngle_south);
  RUN_TEST(test_fastAngle_diagonal_ne);
  RUN_TEST(test_fastAngle_diagonal_sw);
  RUN_TEST(test_fastAngle_monotonic_first_quadrant);
TEST_SUITE_END
