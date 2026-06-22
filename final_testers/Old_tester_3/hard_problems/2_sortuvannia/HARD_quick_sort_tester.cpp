// HARD_quick_sort_tester.cpp — "Сума k найменших"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 1 : uniform_int_distribution<int>(1, 300000)(rng);
    int k = uniform_int_distribution<int>(1, n)(rng);
    uniform_int_distribution<long long> vd(-1000000, 1000000);
    vector<long long> a(n);
    for (auto& v : a) v = vd(rng);

    vector<long long> b = a;
    nth_element(b.begin(), b.begin() + k, b.end());
    long long sum = 0;
    for (int i = 0; i < k; i++) sum += b[i];

    stringstream in; in << n << " " << k << "\n";
    for (int i = 0; i < n; i++) in << a[i] << (i+1<n?' ':'\n');
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(sum);
    return tc;
}
#define PROBLEM_NAME "Сума k найменших (складна)"
#include "../../common/tester_main.inc"
