// EZ_kmp_tester.cpp — "Перше входження"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static int findFirst(const string& t, const string& p) {
    size_t pos = t.find(p);
    return pos == string::npos ? -1 : (int)pos + 1;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 7 : uniform_int_distribution<int>(1, 3000)(rng);
    int alphabetSize = uniform_int_distribution<int>(1, 4)(rng);
    uniform_int_distribution<int> ad(0, alphabetSize - 1);
    string t(n, 'a');
    for (auto& c : t) c = 'a' + ad(rng);
    int m = uniform_int_distribution<int>(1, n)(rng);
    string p;
    bool present = uniform_int_distribution<int>(0,1)(rng);
    if (idx == 0) { t = "abcabcd"; p = "abcd"; }
    else if (idx == 1) { t = "aaa"; p = "b"; }
    else if (present) {
        int start = uniform_int_distribution<int>(0, n - m)(rng);
        p = t.substr(start, m);
    } else {
        // зразок з символу поза алфавітом тексту - гарантовано відсутній
        p = string(m, 'z');
        if (alphabetSize >= 26) p = string(m, 'a'); // запобіжник, малоймовірно
    }

    int ans = findFirst(t, p);
    stringstream in; in << t << "\n" << p << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Перше входження (KMP)"
#include "../../common/tester_main.inc"
