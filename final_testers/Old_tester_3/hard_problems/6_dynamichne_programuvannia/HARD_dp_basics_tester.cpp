// HARD_dp_basics_tester.cpp — "Способи розміну"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;
static const long long MOD = 1000000007LL;

static long long countWays(int n, int S, vector<int>& coins) {
    vector<long long> dp(S+1, 0);
    dp[0] = 1;
    for (int c : coins) {
        for (int s = c; s <= S; s++) dp[s] = (dp[s] + dp[s-c]) % MOD;
    }
    return dp[S];
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 3 : uniform_int_distribution<int>(1, 100)(rng);
    int S = idx == 0 ? 5 : uniform_int_distribution<int>(1, 100000)(rng);
    vector<int> coins(n);
    set<int> used;
    uniform_int_distribution<int> cd(1, 100000);
    while ((int)used.size() < n) used.insert(cd(rng));
    coins.assign(used.begin(), used.end());
    n = (int)coins.size();
    if (idx == 0) { coins = {1,2,5}; n = 3; S = 5; }

    long long ans = countWays(n, S, coins);
    stringstream in; in << n << " " << S << "\n";
    for (int i = 0; i < n; i++) in << coins[i] << (i+1<n?' ':'\n');
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Способи розміну (DP, складна)"
#include "../../common/tester_main.inc"
