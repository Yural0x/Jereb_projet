// EZ_kadane_tester.cpp — "Смуга везіння"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 1 : uniform_int_distribution<int>(1, 2000)(rng);
    uniform_int_distribution<int> vd(-1000, 1000);
    vector<long long> a(n);
    for (auto& v : a) v = vd(rng);

    long long cur = a[0], best = a[0];
    for (int i = 1; i < n; i++) { cur = max((long long)a[i], cur + a[i]); best = max(best, cur); }

    stringstream in; in << n << "\n";
    for (int i = 0; i < n; i++) in << a[i] << (i+1<n?' ':'\n');
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(best);
    return tc;
}
#define PROBLEM_NAME "Смуга везіння (Кадане)"
#include "../../common/tester_main_multilang.inc"
