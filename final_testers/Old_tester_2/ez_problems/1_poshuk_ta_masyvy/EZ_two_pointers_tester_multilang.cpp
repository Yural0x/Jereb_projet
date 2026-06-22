// EZ_two_pointers_tester.cpp — "Два сувеніри"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 1000;

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 2 : uniform_int_distribution<int>(2, 2000)(rng);
    uniform_int_distribution<int> vd(1, 1000);
    vector<long long> a(n);
    for (auto& v : a) v = vd(rng);
    sort(a.begin(), a.end());

    long long S;
    bool wantYes = uniform_int_distribution<int>(0,1)(rng);
    if (wantYes) {
        int i = uniform_int_distribution<int>(0, n-1)(rng);
        int j = uniform_int_distribution<int>(0, n-1)(rng);
        while (j == i) j = uniform_int_distribution<int>(0, n-1)(rng);
        S = a[i] + a[j];
    } else {
        S = uniform_int_distribution<long long>(1, 4000)(rng);
    }

    bool found = false;
    for (int i = 0, j = n-1; i < j; ) {
        long long s = a[i] + a[j];
        if (s == S) { found = true; break; }
        else if (s < S) i++;
        else j--;
    }

    stringstream in; in << n << " " << S << "\n";
    for (int i = 0; i < n; i++) in << a[i] << (i+1<n?' ':'\n');
    TestCase tc; tc.input = in.str(); tc.expectedOutput = found ? "YES" : "NO";
    return tc;
}
#define PROBLEM_NAME "Два сувеніри (два вказівники)"
#include "../../common/tester_main_multilang.inc"
