// EZ_z_function_tester.cpp — "Усі позиції"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static vector<int> findAll(const string& t, const string& p) {
    int n=(int)t.size(), m=(int)p.size();
    vector<int> res;
    if (m > n) return res;
    for (int i = 0; i + m <= n; i++) if (t.compare(i, m, p) == 0) res.push_back(i + 1);
    return res;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 4 : uniform_int_distribution<int>(1, 3000)(rng);
    int alphabetSize = uniform_int_distribution<int>(1, 3)(rng);
    uniform_int_distribution<int> ad(0, alphabetSize - 1);
    string t(n, 'a');
    for (auto& c : t) c = 'a' + ad(rng);
    int m = uniform_int_distribution<int>(1, n)(rng);
    string p;
    if (idx == 0) { t = "aaaa"; p = "aa"; }
    else {
        int start = uniform_int_distribution<int>(0, n - m)(rng);
        p = t.substr(start, m);
    }

    vector<int> res = findAll(t, p);
    stringstream in; in << t << "\n" << p << "\n";
    stringstream out;
    for (size_t i = 0; i < res.size(); i++) out << res[i] << (i+1<res.size()?' ':' ');
    string e = out.str();
    while (!e.empty() && e.back() == ' ') e.pop_back();
    TestCase tc; tc.input = in.str(); tc.expectedOutput = e;
    return tc;
}
#define PROBLEM_NAME "Усі позиції (Z-функція)"
#include "../../common/tester_main.inc"
