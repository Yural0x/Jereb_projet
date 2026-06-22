// HARD_merge_sort_tester.cpp — "Менші праворуч"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static vector<long long> solveRef(vector<long long> a) {
    int n = (int)a.size();
    vector<long long> result(n, 0);

    // Надійний підхід: Fenwick за стиснутими значеннями, прохід справа наліво.
    vector<long long> sortedVals = a;
    sort(sortedVals.begin(), sortedVals.end());
    sortedVals.erase(unique(sortedVals.begin(), sortedVals.end()), sortedVals.end());
    int m = (int)sortedVals.size();
    vector<long long> bit(m + 1, 0);
    auto bitUpdate = [&](int i) { for (; i <= m; i += i & (-i)) bit[i]++; };
    auto bitQuery = [&](int i) { long long s = 0; for (; i > 0; i -= i & (-i)) s += bit[i]; return s; };

    for (int i = n - 1; i >= 0; i--) {
        int rank = (int)(lower_bound(sortedVals.begin(), sortedVals.end(), a[i]) - sortedVals.begin()) + 1; // 1-indexed
        result[i] = bitQuery(rank - 1); // кількість уже вставлених значень менших за a[i]
        bitUpdate(rank);
    }
    return result;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 1 : uniform_int_distribution<int>(1, 100000)(rng);
    uniform_int_distribution<long long> vd(-1000000000LL, 1000000000LL);
    vector<long long> a(n);
    for (auto& v : a) v = vd(rng);

    vector<long long> res = solveRef(a);
    stringstream in; in << n << "\n";
    for (int i = 0; i < n; i++) in << a[i] << (i+1<n?' ':'\n');
    stringstream out;
    for (int i = 0; i < n; i++) out << res[i] << (i+1<n?' ':' ');
    string e = out.str();
    while (!e.empty() && e.back() == ' ') e.pop_back();
    TestCase tc; tc.input = in.str(); tc.expectedOutput = e;
    return tc;
}
#define PROBLEM_NAME "Менші праворуч (складна)"
#include "../../common/tester_main.inc"
