// HARD_lis_tester.cpp — "Кількість найдовших"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;
static const long long MOD = 1000000007LL;

// O(n^2) рахуємо довжину найдовшої LIS, що закінчується на i, та кількість таких.
static long long solveRef(int n, vector<long long>& a) {
    vector<int> length(n, 1);
    vector<long long> count(n, 1);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < i; j++) {
            if (a[j] < a[i]) {
                if (length[j] + 1 > length[i]) { length[i] = length[j] + 1; count[i] = count[j]; }
                else if (length[j] + 1 == length[i]) { count[i] = (count[i] + count[j]) % MOD; }
            }
        }
    }
    int maxLen = *max_element(length.begin(), length.end());
    long long total = 0;
    for (int i = 0; i < n; i++) if (length[i] == maxLen) total = (total + count[i]) % MOD;
    return total;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 5 : uniform_int_distribution<int>(1, 2000)(rng);
    uniform_int_distribution<long long> vd(-1000000000LL, 1000000000LL);
    vector<long long> a(n);
    for (auto& v : a) v = vd(rng);
    if (idx == 0) a = {1,3,5,4,7};

    long long ans = solveRef(n, a);
    stringstream in; in << n << "\n";
    for (int i = 0; i < n; i++) in << a[i] << (i+1<n?' ':'\n');
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Кількість найдовших (LIS, складна)"
#include "../../common/tester_main_multilang.inc"
