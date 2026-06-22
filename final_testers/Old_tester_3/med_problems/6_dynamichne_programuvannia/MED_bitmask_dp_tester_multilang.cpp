// MED_bitmask_dp_tester.cpp — "Призначення задач"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static long long solveAssignment(int n, vector<vector<long long>>& cost) {
    int FULL = 1 << n;
    vector<long long> dp(FULL, LLONG_MAX/2);
    dp[0] = 0;
    for (int mask = 0; mask < FULL; mask++) {
        if (dp[mask] >= LLONG_MAX/2) continue;
        int worker = __builtin_popcount(mask); // скільки задач уже призначено = індекс наступного працівника
        if (worker >= n) continue;
        for (int task = 0; task < n; task++) {
            if (mask & (1<<task)) continue;
            int nmask = mask | (1<<task);
            long long nd = dp[mask] + cost[worker][task];
            if (nd < dp[nmask]) dp[nmask] = nd;
        }
    }
    return dp[FULL-1];
}

static TestCase genTest(mt19937& rng, int idx) {
    int n;
    vector<vector<long long>> cost;
    if (idx == 0) { n = 2; cost = {{1,2},{3,1}}; }
    else {
        n = uniform_int_distribution<int>(1, 16)(rng);
        cost.assign(n, vector<long long>(n));
        uniform_int_distribution<long long> cd(0, 1000000);
        for (auto& row : cost) for (auto& v : row) v = cd(rng);
    }

    long long ans = solveAssignment(n, cost);
    stringstream in; in << n << "\n";
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) in << cost[i][j] << (j+1<n?' ':'\n');
    }
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Призначення задач (bitmask DP, середня)"
#include "../../common/tester_main_multilang.inc"
