// MED_lis_tester.cpp — "Зростання результатів"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static int lisLen(vector<long long>& a) {
    vector<long long> tails;
    for (long long x : a) {
        auto it = lower_bound(tails.begin(), tails.end(), x);
        if (it == tails.end()) tails.push_back(x); else *it = x;
    }
    return (int)tails.size();
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 8 : uniform_int_distribution<int>(1, 200000)(rng);
    uniform_int_distribution<long long> vd(-1000000000LL, 1000000000LL);
    vector<long long> a(n);
    for (auto& v : a) v = vd(rng);
    if (idx == 0) a = {10,9,2,5,3,7,101,18};

    int ans = lisLen(a);
    stringstream in; in << n << "\n";
    for (int i = 0; i < n; i++) in << a[i] << (i+1<n?' ':'\n');
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Зростання результатів (LIS O(n log n), середня)"
#include "../../common/tester_main_multilang.inc"
