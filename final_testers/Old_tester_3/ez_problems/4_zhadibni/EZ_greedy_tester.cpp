// EZ_greedy_tester.cpp — "Решта монетами"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 1000;

static TestCase genTest(mt19937& rng, int idx) {
    long long S = idx == 0 ? 27 : uniform_int_distribution<long long>(1, 1000000000LL)(rng);
    vector<int> coins = {10, 5, 2, 1};
    int cnt = 0;
    long long rem = S;
    for (int c : coins) { cnt += rem / c; rem %= c; }

    stringstream in; in << S << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(cnt);
    return tc;
}
#define PROBLEM_NAME "Решта монетами (жадібний алгоритм)"
#include "../../common/tester_main.inc"
