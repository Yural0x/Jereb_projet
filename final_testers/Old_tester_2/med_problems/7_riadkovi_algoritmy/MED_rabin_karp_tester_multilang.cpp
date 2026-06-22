// MED_rabin_karp_tester.cpp — "Різні підрядки"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static int countDistinctSubstrings(const string& s, int k) {
    int n = (int)s.size();
    set<string> distinct;
    for (int i = 0; i + k <= n; i++) distinct.insert(s.substr(i, k));
    return (int)distinct.size();
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 5 : uniform_int_distribution<int>(1, 2000)(rng);
    int alphabetSize = uniform_int_distribution<int>(1, 4)(rng);
    uniform_int_distribution<int> ad(0, alphabetSize-1);
    string s(n, 'a');
    for (auto& c : s) c = 'a' + ad(rng);
    int k = uniform_int_distribution<int>(1, n)(rng);
    if (idx == 0) { s = "abcab"; k = 2; }

    int ans = countDistinctSubstrings(s, k);
    stringstream in; in << s << "\n" << k << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Різні підрядки (хешування, середня)"
#include "../../common/tester_main_multilang.inc"
