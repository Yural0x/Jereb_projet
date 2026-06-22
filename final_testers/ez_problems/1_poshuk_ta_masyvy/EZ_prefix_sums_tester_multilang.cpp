// EZ_prefix_sums_tester.cpp — "Щоденник пекаря"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 1 : uniform_int_distribution<int>(1, 2000)(rng);
    int q = idx == 0 ? 1 : uniform_int_distribution<int>(1, 50)(rng);
    uniform_int_distribution<int> vd(0, 1000);
    vector<long long> a(n);
    for (auto& v : a) v = vd(rng);
    vector<long long> pref(n+1, 0);
    for (int i = 0; i < n; i++) pref[i+1] = pref[i] + a[i];

    stringstream in; in << n << " " << q << "\n";
    for (int i = 0; i < n; i++) in << a[i] << (i+1<n?' ':'\n');
    stringstream out;
    for (int j = 0; j < q; j++) {
        int l = uniform_int_distribution<int>(1, n)(rng);
        int r = uniform_int_distribution<int>(l, n)(rng);
        in << l << " " << r << "\n";
        out << (pref[r] - pref[l-1]) << "\n";
    }
    TestCase tc; tc.input = in.str();
    string e = out.str();
    if (!e.empty() && e.back() == '\n') e.pop_back();
    tc.expectedOutput = e;
    return tc;
}
#define PROBLEM_NAME "Щоденник пекаря (префіксні суми)"
#include "../../common/tester_main_multilang.inc"
