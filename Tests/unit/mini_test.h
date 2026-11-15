// Minimal, dependency-free unit-test harness (no network / package install needed,
// identical on Windows, macOS and Linux).
#pragma once

#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

namespace mt {

struct TestCase {
    const char* name;
    void (*fn)();
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> r;
    return r;
}
inline int& failed_checks() { static int n = 0; return n; }
inline long& total_checks() { static long n = 0; return n; }

struct Registrar {
    Registrar(const char* name, void (*fn)()) { registry().push_back({name, fn}); }
};

inline int run_all(int argc, char** argv) {
    const char* filter = argc > 1 ? argv[1] : nullptr;
    int ran = 0, failed = 0;
    for (const TestCase& t : registry()) {
        if (filter && std::strstr(t.name, filter) == nullptr) continue;
        const int before = failed_checks();
        t.fn();
        ++ran;
        const bool ok = failed_checks() == before;
        if (!ok) ++failed;
        std::printf("[%s] %s\n", ok ? " OK " : "FAIL", t.name);
    }
    std::printf("\n%d test(s) run, %d failed, %ld check(s) evaluated\n", ran, failed, total_checks());
    return failed == 0 && ran > 0 ? 0 : 1;
}

}  // namespace mt

#define TEST(name)                                         \
    static void name();                                    \
    static mt::Registrar mt_registrar_##name(#name, name); \
    static void name()

#define CHECK(cond)                                                                   \
    do {                                                                              \
        ++mt::total_checks();                                                         \
        if (!(cond)) {                                                                \
            ++mt::failed_checks();                                                    \
            std::printf("    CHECK FAILED %s:%d: %s\n", __FILE__, __LINE__, #cond);   \
        }                                                                             \
    } while (0)

#define CHECK_EQ(a, b)                                                                          \
    do {                                                                                        \
        ++mt::total_checks();                                                                   \
        if (!((a) == (b))) {                                                                    \
            ++mt::failed_checks();                                                              \
            std::printf("    CHECK_EQ FAILED %s:%d: %s == %s\n", __FILE__, __LINE__, #a, #b);   \
        }                                                                                       \
    } while (0)

#define CHECK_NEAR(a, b, tol)                                                                       \
    do {                                                                                            \
        ++mt::total_checks();                                                                       \
        const double mt_a = static_cast<double>(a), mt_b = static_cast<double>(b);                  \
        if (!(std::fabs(mt_a - mt_b) <= (tol))) {                                                   \
            ++mt::failed_checks();                                                                  \
            std::printf("    CHECK_NEAR FAILED %s:%d: %s=%.9g vs %s=%.9g (tol %g)\n", __FILE__,     \
                        __LINE__, #a, mt_a, #b, mt_b, static_cast<double>(tol));                    \
        }                                                                                           \
    } while (0)
