// MED_sliding_window_tester.cpp — "Унікальні символи"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 1 : uniform_int_distribution<int>(1, 3000)(rng);
    // використовуємо невеликий алфавіт, щоб гарантовано були повтори
    uniform_int_distribution<int> ad(0, 5);
    string s(n, 'a');
    for (auto& c : s) c = 'a' + ad(rng);

    int best = 0;
    int last[256]; fill(begin(last), end(last), -1);
    int left = 0;
    for (int right = 0; right < n; right++) {
        unsigned char c = s[right];
        if (last[c] >= left) left = last[c] + 1;
        last[c] = right;
        best = max(best, right - left + 1);
    }

    stringstream in; in << s << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(best);
    return tc;
}
#define PROBLEM_NAME "Унікальні символи (ковзне вікно, середня)"
#include "../../common/tester_main.inc"
