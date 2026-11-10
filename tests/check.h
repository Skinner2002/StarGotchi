#pragma once
// Minimal test harness: each test file defines TEST(name) functions and
// ends with RUN_TESTS(); main() returns non-zero on any failure.
#include <cmath>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

namespace sgtest {
struct Case { const char *name; void (*fn)(); };
inline std::vector<Case> &cases() { static std::vector<Case> c; return c; }
inline int &failures() { static int f = 0; return f; }
inline const char *&current() { static const char *n = ""; return n; }
struct Reg { Reg(const char *n, void (*f)()) { cases().push_back({n, f}); } };
inline int runAll(const char *file) {
  for (auto &c : cases()) { current() = c.name; c.fn(); }
  std::printf("%s: %zu tests, %d failed checks\n", file, cases().size(), failures());
  return failures() ? 1 : 0;
}
}  // namespace sgtest

#define TEST(name) \
  static void name(); \
  static sgtest::Reg reg_##name(#name, name); \
  static void name()

#define CHECK(cond) \
  do { if (!(cond)) { sgtest::failures()++; \
    std::printf("FAIL %s (%s:%d): %s\n", sgtest::current(), __FILE__, __LINE__, #cond); } } while (0)

#define CHECK_NEAR(a, b, eps) \
  do { double _a = (a), _b = (b); if (std::fabs(_a - _b) > (eps)) { sgtest::failures()++; \
    std::printf("FAIL %s (%s:%d): %s = %f, expected %f\n", sgtest::current(), __FILE__, __LINE__, #a, _a, _b); } } while (0)

#define RUN_TESTS() int main() { return sgtest::runAll(__FILE__); }
