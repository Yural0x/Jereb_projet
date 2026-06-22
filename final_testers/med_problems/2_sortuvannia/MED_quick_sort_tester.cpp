// MED_quick_sort_tester.cpp — "K-те найменше"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 1 : uniform_int_distribution<int>(1, 20000)(rng);
    int k = uniform_int_distribution<int>(1, n)(rng);
    uniform_int_distribution<long long> vd(-1000000000LL, 1000000000LL);
    vector<long long> a(n);
    for (auto& v : a) v = vd(rng);

    vector<long long> b = a;
    nth_element(b.begin(), b.begin() + (k - 1), b.end());
    long long ans = b[k - 1];

    stringstream in; in << n << " " << k << "\n";
    for (int i = 0; i < n; i++) in << a[i] << (i+1<n?' ':'\n');
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "K-те найменше (швидке сортування, середня)"
#include "../../common/tester_main.inc"
