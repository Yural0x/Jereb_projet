// EZ_lcs_tester.cpp — "Спільні літери"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static int lcsLen(const string& a, const string& b) {
    int n = (int)a.size(), m = (int)b.size();
    vector<vector<int>> dp(n+1, vector<int>(m+1, 0));
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= m; j++)
            dp[i][j] = (a[i-1]==b[j-1]) ? dp[i-1][j-1]+1 : max(dp[i-1][j], dp[i][j-1]);
    return dp[n][m];
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 5 : uniform_int_distribution<int>(1, 700)(rng);
    int m = idx == 0 ? 3 : uniform_int_distribution<int>(1, 700)(rng);
    uniform_int_distribution<int> ld(0, 4); // невеликий алфавіт для гарантованих збігів
    string a(n,'a'), b(m,'a');
    for (auto& c : a) c = 'a' + ld(rng);
    for (auto& c : b) c = 'a' + ld(rng);
    if (idx == 0) { a = "abcde"; b = "ace"; }

    int ans = lcsLen(a, b);
    stringstream in; in << a << "\n" << b << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Спільні літери (LCS)"
#include "../../common/tester_main_multilang.inc"
