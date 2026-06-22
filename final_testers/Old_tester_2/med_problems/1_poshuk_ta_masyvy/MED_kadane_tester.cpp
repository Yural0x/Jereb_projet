// MED_kadane_tester.cpp — "Максимальний добуток"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 1 : uniform_int_distribution<int>(1, 500)(rng);
    uniform_int_distribution<int> vd(-10, 10);
    vector<long long> a(n);
    for (auto& v : a) v = vd(rng);

    long long maxP = a[0], minP = a[0], best = a[0];
    for (int i = 1; i < n; i++) {
        long long x = a[i];
        long long candMax = max({x, maxP * x, minP * x});
        long long candMin = min({x, maxP * x, minP * x});
        maxP = candMax; minP = candMin;
        best = max(best, maxP);
    }

    stringstream in; in << n << "\n";
    for (int i = 0; i < n; i++) in << a[i] << (i+1<n?' ':'\n');
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(best);
    return tc;
}
#define PROBLEM_NAME "Максимальний добуток (Кадане, середня)"
#include "../../common/tester_main.inc"
