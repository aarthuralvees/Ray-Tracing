# Test Runner Design

**Date:** 2026-04-15
**Topic:** Dedicated test structure for Vetor/Ponto classes

---

## Goal

Add a lightweight, zero-dependency test structure to the ray tracer project that:
- Lives in its own files, separate from production code
- Is stripped out of normal builds entirely (compile-time flag)
- Prints named, non-aborting failure messages with file and line info
- Scales naturally as new classes (Camera, Ray, Sphere, etc.) are added

---

## Constraints

- Pure C++17, no external libraries
- No build system (single `g++` invocation)
- Tests compile into the same binary, gated by `-DRUN_TESTS`
- Normal render workflow is unchanged

---

## File Structure

```
tests/
  TestRunner.h     ← ~30 lines: CHECK macro, run_test(), report_tests()
  test_vetor.h     ← tests for Vetor ops + dot, cross, unit_vector
  test_ponto.h     ← tests for Ponto arithmetic + ray.at(t)
main.cpp           ← #ifdef RUN_TESTS block at top of main()
```

---

## TestRunner.h

Global state in `namespace tests`:

```cpp
namespace tests {
    inline int _passed = 0, _failed = 0;
    inline const char* _current = "";
}
```

`CHECK` macro — never aborts, prints on failure:

```cpp
#define CHECK(expr) do { \
    if (!(expr)) { \
        std::cerr << "  FAIL [" << tests::_current << "]: " #expr \
                  << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
        tests::_failed++; \
    } else { tests::_passed++; } \
} while(0)
```

Two free functions:

```cpp
inline void run_test(const char* name, void(*fn)()) {
    tests::_current = name;
    fn();
}

inline void report_tests() {
    std::cerr << "\n--- " << tests::_passed + tests::_failed << " checks: "
              << tests::_passed << " passed, " << tests::_failed << " failed ---\n";
}
```

---

## Test Files

Each test file is a plain header with static void functions. No registration macros — functions are explicitly passed to `run_test()` in `main.cpp`.

**`tests/test_vetor.h`** covers:
- Arithmetic: `+`, `-`, `*`, `/`, unary `-`
- `length()`, `length_squared()`
- `dot()` — including perpendicular vectors (expected 0)
- `cross()` — right-hand rule verification
- `unit_vector()` — result length within epsilon of 1.0

**`tests/test_ponto.h`** covers:
- `Ponto + Vetor → Ponto`
- `Ponto - Ponto → Vetor`
- `ray::at(t)` — parametric position

---

## main.cpp Hookup

```cpp
#ifdef RUN_TESTS
#include "tests/test_vetor.h"
#include "tests/test_ponto.h"

static void run_all_tests() {
    run_test("vetor_arithmetic",  vetor_arithmetic);
    run_test("vetor_length",      vetor_length);
    run_test("vetor_dot_cross",   vetor_dot_cross);
    run_test("vetor_unit_vector", vetor_unit_vector);
    run_test("ponto_arithmetic",  ponto_arithmetic);
    run_test("ray_at",            ray_at);
    report_tests();
}
#endif

int main() {
#ifdef RUN_TESTS
    run_all_tests();
    return tests::_failed > 0 ? 1 : 0;
#endif
    // ... renderer ...
}
```

---

## How to Use

**Run tests:**
```bash
g++ -std=c++17 -DRUN_TESTS main.cpp -o raytracer && ./raytracer
```

**Normal render (tests stripped out):**
```bash
g++ -std=c++17 main.cpp -o raytracer && ./raytracer > image.ppm
```

**Adding tests for a new class (e.g., Camera):**
1. Create `tests/test_camera.h` with static test functions
2. `#include "tests/test_camera.h"` in the `#ifdef RUN_TESTS` block in `main.cpp`
3. Add `run_test("camera_...", fn)` calls inside `run_all_tests()`
