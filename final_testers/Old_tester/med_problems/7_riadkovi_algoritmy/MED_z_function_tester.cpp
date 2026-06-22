// MED_z_function_tester.cpp — "Найменший період"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static vector<int> zFunction(const string& s) {
    int n = (int)s.size();
    vector<int> z(n, 0);
    int l = 0, r = 0;
    for (int i = 1; i < n; i++) {
        if (i < r) z[i] = min(r - i, z[i - l]);
        while (i + z[i] < n && s[z[i]] == s[i + z[i]]) z[i]++;
        if (i + z[i] > r) { l = i; r = i + z[i]; }
    }
    return z;
}
// Найменший період через Z-функцію: найменше p таке, що z[p] == n - p (тобто
// суфікс з позиції p збігається з префіксом такої ж довжини, і це покриває весь рядок).
static int smallestPeriod(const string& s) {
    int n = (int)s.size();
    if (n == 0) return 0;
    vector<int> z = zFunction(s);
    for (int p = 1; p < n; p++) {
        if (p + z[p] == n) return p;
    }
    return n;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 9 : uniform_int_distribution<int>(1, 100000)(rng);
    string s;
    if (idx == 0) s = "abcabcabc";
    else if (idx == 1) s = "abcd";
    else {
        bool periodic = uniform_int_distribution<int>(0,1)(rng);
        if (periodic && n >= 2) {
            int blockLen = uniform_int_distribution<int>(1, n)(rng);
            string block(blockLen, 'a');
            uniform_int_distribution<int> ld(0, 2);
            for (auto& c : block) c = 'a' + ld(rng);
            s.clear();
            while ((int)s.size() < n) s += block;
            s = s.substr(0, n);
        } else {
            uniform_int_distribution<int> ld(0, 3);
            s.assign(n, 'a');
            for (auto& c : s) c = 'a' + ld(rng);
        }
    }
    n = (int)s.size();

    int ans = smallestPeriod(s);
    stringstream in; in << s << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Найменший період (Z-функція, середня)"
#include "../../common/tester_main.inc"
