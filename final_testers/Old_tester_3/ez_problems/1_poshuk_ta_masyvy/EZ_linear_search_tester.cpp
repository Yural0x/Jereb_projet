// EZ_linear_search_tester.cpp — "Загублений рюкзак" (лінійний пошук)
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 1000;

static string solveRef(int n, long long x, vector<long long>& a) {
    for (int i = 0; i < n; i++) if (a[i] == x) return to_string(i + 1);
    return "-1";
}
static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 1 : uniform_int_distribution<int>(1, 1000)(rng);
    uniform_int_distribution<long long> vd(1, 1000);
    vector<long long> a(n);
    for (auto& v : a) v = vd(rng);
    long long x;
    if (uniform_int_distribution<int>(0,1)(rng)) x = a[uniform_int_distribution<int>(0,n-1)(rng)];
    else x = vd(rng);
    stringstream in; in << n << " " << x << "\n";
    for (int i = 0; i < n; i++) in << a[i] << (i+1<n?' ':'\n');
    TestCase tc; tc.input = in.str(); tc.expectedOutput = solveRef(n, x, a);
    return tc;
}
#define PROBLEM_NAME "Загублений рюкзак (лінійний пошук)"
#include "../../common/tester_main.inc"
