// EZ_sliding_window_tester.cpp — "Найкасовіший тиждень"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 1 : uniform_int_distribution<int>(1, 2000)(rng);
    int k = uniform_int_distribution<int>(1, n)(rng);
    uniform_int_distribution<int> vd(0, 1000);
    vector<long long> a(n);
    for (auto& v : a) v = vd(rng);

    long long cur = 0;
    for (int i = 0; i < k; i++) cur += a[i];
    long long best = cur;
    for (int i = k; i < n; i++) { cur += a[i] - a[i-k]; best = max(best, cur); }

    stringstream in; in << n << " " << k << "\n";
    for (int i = 0; i < n; i++) in << a[i] << (i+1<n?' ':'\n');
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(best);
    return tc;
}
#define PROBLEM_NAME "Найкасовіший тиждень (ковзне вікно)"
#include "../../common/tester_main_multilang.inc"
