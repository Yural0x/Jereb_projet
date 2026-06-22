// HARD_kadane_tester.cpp — "Підматриця з максимальною сумою"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static long long kadane1D(const vector<long long>& arr) {
    long long cur = arr[0], best = arr[0];
    for (size_t i = 1; i < arr.size(); i++) {
        cur = max(arr[i], cur + arr[i]);
        best = max(best, cur);
    }
    return best;
}

static long long solveRef(int n, int m, vector<vector<long long>>& g) {
    long long best = LLONG_MIN;
    for (int top = 0; top < n; top++) {
        vector<long long> colSum(m, 0);
        for (int bottom = top; bottom < n; bottom++) {
            for (int c = 0; c < m; c++) colSum[c] += g[bottom][c];
            best = max(best, kadane1D(colSum));
        }
    }
    return best;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n, m;
    if (idx == 0) { n = 1; m = 1; }
    else { n = uniform_int_distribution<int>(1, 40)(rng); m = uniform_int_distribution<int>(1, 40)(rng); }
    uniform_int_distribution<int> vd(-100, 100);
    vector<vector<long long>> g(n, vector<long long>(m));
    for (auto& row : g) for (auto& v : row) v = vd(rng);

    long long ans = solveRef(n, m, g);
    stringstream in; in << n << " " << m << "\n";
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) in << g[i][j] << (j+1<m?' ':'\n');
    }
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Підматриця з максимальною сумою (Кадане 2D, складна)"
#include "../../common/tester_main.inc"
