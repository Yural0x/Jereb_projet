// HARD_binary_search_tester.cpp — "K-те у двох списках"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 1000;

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 1 : uniform_int_distribution<int>(1, 2000)(rng);
    int m = idx == 0 ? 1 : uniform_int_distribution<int>(1, 2000)(rng);
    uniform_int_distribution<long long> vd(-1000000000LL, 1000000000LL);
    vector<long long> a(n), b(m);
    for (auto& v : a) v = vd(rng);
    for (auto& v : b) v = vd(rng);
    sort(a.begin(), a.end());
    sort(b.begin(), b.end());
    int k = uniform_int_distribution<int>(1, n + m)(rng);

    // Еталон: просте злиття (достатньо швидко для тестового n,m<=2000)
    vector<long long> merged;
    merged.reserve(n + m);
    merge(a.begin(), a.end(), b.begin(), b.end(), back_inserter(merged));
    long long ans = merged[k - 1];

    stringstream in; in << n << " " << m << " " << k << "\n";
    for (int i = 0; i < n; i++) in << a[i] << (i+1<n?' ':'\n');
    for (int i = 0; i < m; i++) in << b[i] << (i+1<m?' ':'\n');
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "K-те у двох списках (бінарний пошук, складна)"
#include "../../common/tester_main_multilang.inc"
