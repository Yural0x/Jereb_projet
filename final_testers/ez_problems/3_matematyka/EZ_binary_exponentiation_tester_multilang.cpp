// EZ_binary_exponentiation_tester.cpp — "Степінь за модулем"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 1000;
static const long long MOD = 1000000007LL;

static long long power(long long a, long long n, long long mod) {
    a %= mod; if (a < 0) a += mod;
    long long res = 1;
    while (n > 0) {
        if (n & 1) res = (res * a) % mod;
        a = (a * a) % mod;
        n >>= 1;
    }
    return res;
}

static TestCase genTest(mt19937& rng, int idx) {
    uniform_int_distribution<long long> ad(0, 1000000000LL);
    uniform_int_distribution<long long> nd(0, (long long)1e18);
    long long a = idx == 0 ? 2 : ad(rng);
    long long n = idx == 0 ? 10 : nd(rng);
    if (idx == 1) { a = 3; n = 0; }

    long long ans = power(a, n, MOD);
    stringstream in; in << a << " " << n << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Степінь за модулем (швидке піднесення до степеня)"
#include "../../common/tester_main_multilang.inc"
