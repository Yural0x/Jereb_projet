// EZ_bitmask_dp_tester.cpp — "Маршрут кур'єра"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static long long tsp(int n, vector<vector<long long>>& d) {
    int FULL = 1 << n;
    vector<vector<long long>> dp(FULL, vector<long long>(n, LLONG_MAX/2));
    dp[1][0] = 0;
    for (int mask = 1; mask < FULL; mask++) {
        for (int u = 0; u < n; u++) {
            if (!(mask & (1<<u))) continue;
            if (dp[mask][u] >= LLONG_MAX/2) continue;
            for (int v = 0; v < n; v++) {
                if (mask & (1<<v)) continue;
                int nmask = mask | (1<<v);
                long long nd = dp[mask][u] + d[u][v];
                if (nd < dp[nmask][v]) dp[nmask][v] = nd;
            }
        }
    }
    long long best = LLONG_MAX/2;
    for (int u = 0; u < n; u++) {
        if (d[u][0] == 0 && u != 0 && n > 1) continue; // нічого спеціального, просто продовжити
        best = min(best, dp[FULL-1][u] + d[u][0]);
    }
    return best;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n;
    vector<vector<long long>> d;
    if (idx == 0) {
        n = 4;
        d = {{0,10,15,20},{10,0,35,25},{15,35,0,30},{20,25,30,0}};
    } else {
        n = uniform_int_distribution<int>(1, 11)(rng);
        d.assign(n, vector<long long>(n, 0));
        uniform_int_distribution<long long> wd(1, 1000);
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                if (i != j) d[i][j] = wd(rng);
    }

    long long ans = (n == 1) ? 0 : tsp(n, d);
    stringstream in; in << n << "\n";
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) in << d[i][j] << (j+1<n?' ':'\n');
    }
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Маршрут кур'єра (bitmask DP / TSP)"
#include "../../common/tester_main_multilang.inc"
