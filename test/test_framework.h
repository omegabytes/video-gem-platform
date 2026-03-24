#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static int _tf_total   = 0;
static int _tf_passed  = 0;
static int _tf_failed  = 0;
static int _tf_snap    = 0;

#define RUN_TEST(fn) do {                                     \
  _tf_total++;                                                \
  _tf_snap = _tf_failed;                                      \
  fn();                                                       \
  if (_tf_failed == _tf_snap) {                               \
    printf("  \033[32m[PASS]\033[0m %s\n", #fn);              \
    _tf_passed++;                                             \
  }                                                           \
} while(0)

#define _TF_FAIL_AT(file, line) do {                          \
  _tf_failed++; return;                                       \
} while(0)

#define TEST_ASSERT(cond) do {                                \
  if (!(cond)) {                                              \
    printf("  \033[31m[FAIL]\033[0m %s:%d: %s\n",             \
           __FILE__, __LINE__, #cond);                         \
    _TF_FAIL_AT(__FILE__, __LINE__);                           \
  }                                                           \
} while(0)

#define TEST_ASSERT_EQUAL_INT(exp, act) do {                  \
  long _e = (long)(exp), _a = (long)(act);                    \
  if (_e != _a) {                                             \
    printf("  \033[31m[FAIL]\033[0m %s:%d: "                  \
           "expected %ld, got %ld\n",                          \
           __FILE__, __LINE__, _e, _a);                        \
    _TF_FAIL_AT(__FILE__, __LINE__);                           \
  }                                                           \
} while(0)

#define TEST_ASSERT_FLOAT_WITHIN(tol, exp, act) do {          \
  double _e=(double)(exp), _a=(double)(act), _t=(double)(tol);\
  if (fabs(_e - _a) > _t) {                                  \
    printf("  \033[31m[FAIL]\033[0m %s:%d: "                  \
           "expected %.6f +/- %.6f, got %.6f\n",               \
           __FILE__, __LINE__, _e, _t, _a);                    \
    _TF_FAIL_AT(__FILE__, __LINE__);                           \
  }                                                           \
} while(0)

#define TEST_ASSERT_STR_EQUAL(exp, act) do {                  \
  const char* _e = (exp);                                     \
  const char* _a = (act);                                     \
  if (_e == NULL || _a == NULL) {                             \
    if (_e != _a) {                                           \
      printf("  \033[31m[FAIL]\033[0m %s:%d: expected %s, got %s\n", \
             __FILE__, __LINE__, _e ? _e : "NULL", _a ? _a : "NULL"); \
      _TF_FAIL_AT(__FILE__, __LINE__);                         \
    }                                                         \
  } else if (strcmp(_e, _a) != 0) {                           \
    printf("  \033[31m[FAIL]\033[0m %s:%d: expected \"%s\", got \"%s\"\n", \
           __FILE__, __LINE__, _e, _a);                        \
    _TF_FAIL_AT(__FILE__, __LINE__);                           \
  }                                                           \
} while(0)

#define TEST_SUITE_BEGIN(name) \
  int main() { printf("\n\033[1m═══ %s ═══\033[0m\n", name);

#define TEST_SUITE_END                                        \
  printf("\n── %d/%d passed ──\n\n", _tf_passed, _tf_total);  \
  return (_tf_failed > 0) ? 1 : 0; }

#endif
