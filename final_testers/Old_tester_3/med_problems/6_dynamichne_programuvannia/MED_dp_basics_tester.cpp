// MED_dp_basics_tester.cpp — "Рюкзак"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static long long solveKnapsack(int n, int W, vector<int>& w, vector<long long>& v) {
    vector<long long> dp(W+1, 0);
    for (int i = 0; i < n; i++) {
        for (int cap = W; cap >= w[i]; cap--) {
            dp[cap] = max(dp[cap], dp[cap - w[i]] + v[i]);
        }
    }
    return dp[W];
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 4 : uniform_int_distribution<int>(1, 500)(rng);
    int W = idx == 0 ? 7 : uniform_int_distribution<int>(1, 5000)(rng);
    uniform_int_distribution<int> wd(1, max(W,1));
    uniform_int_distribution<long long> vd(1, 1000000);
    vector<int> w(n);
    vector<long long> v(n);
    for (int i = 0; i < n; i++) { w[i] = uniform_int_distribution<int>(1, max(W,1))(rng); v[i] = vd(rng); }
    if (idx == 0) { w = {1,3,4,5}; v = {1,4,5,7}; W = 7; n = 4; }

    long long ans = solveKnapsack(n, W, w, v);
    stringstream in; in << n << " " << W << "\n";
    for (int i = 0; i < n; i++) in << w[i] << " " << v[i] << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Рюкзак (DP 0/1, середня)"
#include "../../common/tester_main.inc"
