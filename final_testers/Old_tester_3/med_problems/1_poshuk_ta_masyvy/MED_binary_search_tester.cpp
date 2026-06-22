// MED_binary_search_tester.cpp — "Швидкість читання"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 1000;

static bool canFinish(const vector<long long>& a, long long v, int h) {
    long long hours = 0;
    for (long long p : a) hours += (p + v - 1) / v;
    return hours <= h;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 1 : uniform_int_distribution<int>(1, 1000)(rng);
    int h = uniform_int_distribution<int>(n, n + 1000)(rng); // h >= n завжди можливо
    uniform_int_distribution<long long> vd(1, 1000000000LL);
    vector<long long> a(n);
    for (auto& v : a) v = vd(rng);

    long long lo = 1, hi = *max_element(a.begin(), a.end());
    while (lo < hi) {
        long long mid = lo + (hi - lo) / 2;
        if (canFinish(a, mid, h)) hi = mid; else lo = mid + 1;
    }

    stringstream in; in << n << " " << h << "\n";
    for (int i = 0; i < n; i++) in << a[i] << (i+1<n?' ':'\n');
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(lo);
    return tc;
}
#define PROBLEM_NAME "Швидкість читання (бінарний пошук, середня)"
#include "../../common/tester_main.inc"
