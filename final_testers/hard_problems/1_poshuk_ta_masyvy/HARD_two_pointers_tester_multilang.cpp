// HARD_two_pointers_tester.cpp — "Трійки під лімітом"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 3 : uniform_int_distribution<int>(3, 400)(rng);
    uniform_int_distribution<long long> vd(-1000000000LL, 1000000000LL);
    vector<long long> a(n);
    for (auto& v : a) v = vd(rng);
    long long S = uniform_int_distribution<long long>(-2000000000LL, 2000000000LL)(rng);

    vector<long long> b = a;
    sort(b.begin(), b.end());
    long long count = 0;
    for (int i = 0; i < n - 2; i++) {
        int lo = i + 1, hi = n - 1;
        while (lo < hi) {
            long long sum = b[i] + b[lo] + b[hi];
            if (sum < S) { count += (hi - lo); lo++; }
            else hi--;
        }
    }

    stringstream in; in << n << " " << S << "\n";
    for (int i = 0; i < n; i++) in << a[i] << (i+1<n?' ':'\n');
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(count);
    return tc;
}
#define PROBLEM_NAME "Трійки під лімітом (два вказівники, складна)"
#include "../../common/tester_main_multilang.inc"
