// HARD_euclidean_gcd_tester.cpp — "Обернений за модулем"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 1000;

// Розширений алгоритм Евкліда: повертає (g, x, y) такі що a*x + b*y = g
static long long extgcd(long long a, long long b, long long& x, long long& y) {
    if (b == 0) { x = 1; y = 0; return a; }
    long long x1, y1;
    long long g = extgcd(b, a % b, x1, y1);
    x = y1;
    y = x1 - (a / b) * y1;
    return g;
}

static TestCase genTest(mt19937& rng, int idx) {
    uniform_int_distribution<long long> md(2, 1000000000000000000LL);
    long long m = idx == 0 ? 7 : md(rng);
    uniform_int_distribution<long long> ad(1, m - 1 > 0 ? m - 1 : 1);
    long long a = idx == 0 ? 3 : ad(rng);
    if (idx == 1) { a = 2; m = 4; } // спеціальний випадок: оберненого немає

    long long x, y;
    long long g = extgcd(a, m, x, y);
    string ans;
    if (g != 1) ans = "-1";
    else {
        long long inv = ((x % m) + m) % m;
        ans = to_string(inv);
    }

    stringstream in; in << a << " " << m << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = ans;
    return tc;
}
#define PROBLEM_NAME "Обернений за модулем (складна)"
#include "../../common/tester_main_multilang.inc"
