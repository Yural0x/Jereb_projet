// HARD_counting_sort_tester.cpp — "Максимальний розрив"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 1 : uniform_int_distribution<int>(1, 200000)(rng);
    uniform_int_distribution<long long> vd(0, 1000000000LL);
    vector<long long> a(n);
    for (auto& v : a) v = vd(rng);

    long long ans = 0;
    if (n >= 2) {
        vector<long long> b = a;
        sort(b.begin(), b.end());
        for (int i = 1; i < n; i++) ans = max(ans, b[i] - b[i-1]);
    }

    stringstream in; in << n << "\n";
    for (int i = 0; i < n; i++) in << a[i] << (i+1<n?' ':'\n');
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Максимальний розрив (складна)"
#include "../../common/tester_main.inc"
