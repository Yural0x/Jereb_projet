// HARD_rabin_karp_tester.cpp — "Найдовший спільний підрядок"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

// Еталон через динамічне програмування O(n*m) - для тестових обмежень (n,m <= 1500) прийнятно.
static int lcSubstring(const string& a, const string& b) {
    int n = (int)a.size(), m = (int)b.size();
    vector<vector<int>> dp(n+1, vector<int>(m+1, 0));
    int best = 0;
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= m; j++) {
            if (a[i-1] == b[j-1]) { dp[i][j] = dp[i-1][j-1] + 1; best = max(best, dp[i][j]); }
        }
    return best;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 5 : uniform_int_distribution<int>(1, 1500)(rng);
    int m = idx == 0 ? 5 : uniform_int_distribution<int>(1, 1500)(rng);
    uniform_int_distribution<int> ld(0, 3); // малий алфавіт - гарантовані збіги
    string a(n, 'a'), b(m, 'a');
    for (auto& c : a) c = 'a' + ld(rng);
    for (auto& c : b) c = 'a' + ld(rng);
    if (idx == 0) { a = "abcde"; b = "cdefg"; }

    int ans = lcSubstring(a, b);
    stringstream in; in << a << "\n" << b << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Найдовший спільний підрядок (складна)"
#include "../../common/tester_main_multilang.inc"
