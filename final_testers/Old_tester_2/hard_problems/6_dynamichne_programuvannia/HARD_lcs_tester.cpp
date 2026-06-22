// HARD_lcs_tester.cpp — "Спільна для трьох"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static int lcs3(const string& a, const string& b, const string& c) {
    int n=(int)a.size(), m=(int)b.size(), p=(int)c.size();
    vector<vector<vector<int>>> dp(n+1, vector<vector<int>>(m+1, vector<int>(p+1, 0)));
    for (int i=1;i<=n;i++)
        for (int j=1;j<=m;j++)
            for (int k=1;k<=p;k++) {
                if (a[i-1]==b[j-1] && b[j-1]==c[k-1]) dp[i][j][k] = dp[i-1][j-1][k-1] + 1;
                else dp[i][j][k] = max({dp[i-1][j][k], dp[i][j-1][k], dp[i][j][k-1]});
            }
    return dp[n][m][p];
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 5 : uniform_int_distribution<int>(1, 100)(rng);
    int m = idx == 0 ? 3 : uniform_int_distribution<int>(1, 100)(rng);
    int p = idx == 0 ? 3 : uniform_int_distribution<int>(1, 100)(rng);
    uniform_int_distribution<int> ld(0, 3); // малий алфавіт для гарантованих збігів у трьох
    string a(n,'a'), b(m,'a'), c(p,'a');
    for (auto& ch : a) ch = 'a' + ld(rng);
    for (auto& ch : b) ch = 'a' + ld(rng);
    for (auto& ch : c) ch = 'a' + ld(rng);
    if (idx == 0) { a = "abcde"; b = "ace"; c = "abe"; }

    int ans = lcs3(a, b, c);
    stringstream in; in << a << "\n" << b << "\n" << c << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Спільна для трьох (LCS 3D, складна)"
#include "../../common/tester_main.inc"
