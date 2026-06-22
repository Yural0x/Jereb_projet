// MED_linear_search_tester.cpp — "Лідери рейтингу"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 1000;

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 1 : uniform_int_distribution<int>(1, 2000)(rng);
    uniform_int_distribution<long long> vd(0, 1000000000LL);
    vector<long long> a(n);
    for (auto& v : a) v = vd(rng);

    int cnt = 0;
    long long mx = LLONG_MIN;
    for (int i = n - 1; i >= 0; i--) { if (a[i] > mx) { cnt++; mx = a[i]; } }

    stringstream in; in << n << "\n";
    for (int i = 0; i < n; i++) in << a[i] << (i+1<n?' ':'\n');
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(cnt);
    return tc;
}
#define PROBLEM_NAME "Лідери рейтингу (лінійний пошук, середня)"
#include "../../common/tester_main_multilang.inc"
