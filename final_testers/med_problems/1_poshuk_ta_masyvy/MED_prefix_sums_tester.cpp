// MED_prefix_sums_tester.cpp — "Точка рівноваги"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 1 : uniform_int_distribution<int>(1, 2000)(rng);
    uniform_int_distribution<int> vd(-1000, 1000);
    vector<long long> a(n);
    for (auto& v : a) v = vd(rng);

    vector<long long> pref(n+1, 0);
    for (int i = 0; i < n; i++) pref[i+1] = pref[i] + a[i];
    long long total = pref[n];

    int ans = -1;
    for (int p = 1; p <= n; p++) {
        long long left = pref[p-1];          // сума елементів до p (1-індекс), не враховуючи a[p]
        long long right = total - pref[p];   // сума елементів після p
        if (left == right) { ans = p; break; }
    }

    stringstream in; in << n << "\n";
    for (int i = 0; i < n; i++) in << a[i] << (i+1<n?' ':'\n');
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Точка рівноваги (префіксні суми, середня)"
#include "../../common/tester_main.inc"
