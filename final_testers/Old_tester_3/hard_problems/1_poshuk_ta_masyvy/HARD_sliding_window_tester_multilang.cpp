// HARD_sliding_window_tester.cpp — "Мінімальне вікно"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static int solveRef(const string& t, const string& p) {
    int need[26] = {0};
    for (char c : p) need[c - 'A']++;
    int required = 0;
    for (int i = 0; i < 26; i++) if (need[i] > 0) required++;

    int have[26] = {0};
    int formed = 0;
    int n = (int)t.size();
    int best = INT_MAX;
    int left = 0;
    for (int right = 0; right < n; right++) {
        int c = t[right] - 'A';
        if (c < 0 || c >= 26) continue; // тільки великі літери очікуються
        have[c]++;
        if (need[c] > 0 && have[c] == need[c]) formed++;
        while (formed == required) {
            best = min(best, right - left + 1);
            int lc = t[left] - 'A';
            have[lc]--;
            if (need[lc] > 0 && have[lc] < need[lc]) formed--;
            left++;
        }
    }
    return best == INT_MAX ? -1 : best;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 1 : uniform_int_distribution<int>(1, 2000)(rng);
    int alphabetSize = uniform_int_distribution<int>(2, 6)(rng);
    uniform_int_distribution<int> ad(0, alphabetSize - 1);
    string t(n, 'A');
    for (auto& c : t) c = 'A' + ad(rng);

    int plen = uniform_int_distribution<int>(1, min(5, n))(rng);
    string p(plen, 'A');
    for (auto& c : p) c = 'A' + ad(rng);

    int ans = solveRef(t, p);
    stringstream in; in << t << "\n" << p << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Мінімальне вікно (ковзне вікно, складна)"
#include "../../common/tester_main_multilang.inc"
