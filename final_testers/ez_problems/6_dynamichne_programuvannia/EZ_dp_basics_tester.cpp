// EZ_dp_basics_tester.cpp — "Число Фібоначчі"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 1000;
static const long long MOD = 1000000007LL;

static long long fibMod(long long n) {
    if (n <= 2) return n == 0 ? 0 : 1;
    vector<long long> dp(n+1);
    dp[1] = 1; dp[2] = 1;
    for (long long i = 3; i <= n; i++) dp[i] = (dp[i-1] + dp[i-2]) % MOD;
    return dp[n];
}

static TestCase genTest(mt19937& rng, int idx) {
    long long n = idx == 0 ? 10 : uniform_int_distribution<long long>(1, 1000000)(rng);
    long long ans = fibMod(n);
    stringstream in; in << n << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Число Фібоначчі (базове ДП)"
#include "../../common/tester_main.inc"
