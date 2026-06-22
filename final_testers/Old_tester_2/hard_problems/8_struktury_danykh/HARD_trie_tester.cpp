// HARD_trie_tester.cpp — "Максимальний XOR"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static long long maxXorPair(vector<long long>& a) {
    long long best = 0;
    int n = (int)a.size();
    // O(n^2) для n<=2000 цілком прийнятно (4*10^6 операцій)
    for (int i = 0; i < n; i++)
        for (int j = i+1; j < n; j++)
            best = max(best, a[i] ^ a[j]);
    return best;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 6 : uniform_int_distribution<int>(2, 2000)(rng);
    uniform_int_distribution<long long> vd(0, 1000000000LL);
    vector<long long> a(n);
    for (auto& v : a) v = vd(rng);
    if (idx == 0) a = {3,10,5,25,2,8};

    long long ans = maxXorPair(a);
    stringstream in; in << n << "\n";
    for (int i = 0; i < n; i++) in << a[i] << (i+1<n?' ':'\n');
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Максимальний XOR (бор, складна)"
#include "../../common/tester_main.inc"
