// MED_two_pointers_tester.cpp — "Найбільший резервуар"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 1000;

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 2 : uniform_int_distribution<int>(2, 2000)(rng);
    uniform_int_distribution<long long> vd(0, 1000000000LL);
    vector<long long> a(n);
    for (auto& v : a) v = vd(rng);

    long long best = 0;
    int i = 0, j = n - 1;
    while (i < j) {
        long long h = min(a[i], a[j]);
        best = max(best, h * (j - i));
        if (a[i] < a[j]) i++; else j--;
    }

    stringstream in; in << n << "\n";
    for (int k = 0; k < n; k++) in << a[k] << (k+1<n?' ':'\n');
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(best);
    return tc;
}
#define PROBLEM_NAME "Найбільший резервуар (два вказівники, середня)"
#include "../../common/tester_main.inc"
