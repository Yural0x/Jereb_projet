// HARD_prefix_sums_tester.cpp — "Кратні відрізки"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 1 : uniform_int_distribution<int>(1, 3000)(rng);
    int k = uniform_int_distribution<int>(1, 100)(rng);
    uniform_int_distribution<int> vd(-1000, 1000);
    vector<long long> a(n);
    for (auto& v : a) v = vd(rng);

    vector<long long> cnt(k, 0);
    long long pref = 0, ans = 0;
    cnt[0] = 1; // префікс довжини 0
    for (int i = 0; i < n; i++) {
        pref += a[i];
        long long r = ((pref % k) + k) % k;
        ans += cnt[r];
        cnt[r]++;
    }

    stringstream in; in << n << " " << k << "\n";
    for (int i = 0; i < n; i++) in << a[i] << (i+1<n?' ':'\n');
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Кратні відрізки (префіксні суми, складна)"
#include "../../common/tester_main_multilang.inc"
