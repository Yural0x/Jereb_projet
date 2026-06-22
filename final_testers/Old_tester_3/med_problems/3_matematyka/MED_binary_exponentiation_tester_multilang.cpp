// MED_binary_exponentiation_tester.cpp — "Ділення за модулем"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 1000;
static const long long MOD = 1000000007LL;

static long long power(long long a, long long n, long long mod) {
    a %= mod; if (a < 0) a += mod;
    long long res = 1;
    while (n > 0) { if (n & 1) res = res * a % mod; a = a * a % mod; n >>= 1; }
    return res;
}

static TestCase genTest(mt19937& rng, int idx) {
    uniform_int_distribution<long long> ad(1, (long long)1e18);
    long long a = idx == 0 ? 1 : ad(rng);
    long long b = idx == 0 ? 2 : ad(rng) % MOD; // b mod MOD != 0 потрібно
    if (b % MOD == 0) b += 1;

    long long binv = power(b % MOD, MOD - 2, MOD);
    long long ans = (a % MOD) * binv % MOD;

    stringstream in; in << a << " " << b << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Ділення за модулем (середня)"
#include "../../common/tester_main_multilang.inc"
