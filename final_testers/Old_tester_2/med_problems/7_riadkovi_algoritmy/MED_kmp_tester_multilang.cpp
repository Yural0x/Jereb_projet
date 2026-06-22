// MED_kmp_tester.cpp — "Кількість входжень"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static long long countOccurrences(const string& t, const string& p) {
    int n=(int)t.size(), m=(int)p.size();
    if (m > n) return 0;
    long long cnt = 0;
    for (int i = 0; i + m <= n; i++) if (t.compare(i, m, p) == 0) cnt++;
    return cnt;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 4 : uniform_int_distribution<int>(1, 1000000)(rng);
    int alphabetSize = uniform_int_distribution<int>(1, 3)(rng);
    uniform_int_distribution<int> ad(0, alphabetSize-1);
    string t;
    string p;
    if (idx == 0) { t = "aaaa"; p = "aa"; }
    else {
        t.assign(n, 'a');
        for (auto& c : t) c = 'a' + ad(rng);
        int m = uniform_int_distribution<int>(1, min(n, 50))(rng);
        int start = uniform_int_distribution<int>(0, n - m)(rng);
        p = t.substr(start, m);
    }

    long long ans = countOccurrences(t, p);
    stringstream in; in << t << "\n" << p << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Кількість входжень (KMP, середня)"
#include "../../common/tester_main_multilang.inc"
