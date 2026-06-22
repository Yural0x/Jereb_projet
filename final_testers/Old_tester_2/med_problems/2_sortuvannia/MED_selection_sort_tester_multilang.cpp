// MED_selection_sort_tester.cpp — "Мінімум перестановок"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 1000;

static int solveRef(int n, vector<long long>& a) {
    vector<long long> sorted_a = a;
    sort(sorted_a.begin(), sorted_a.end());
    unordered_map<long long, int> pos;
    for (int i = 0; i < n; i++) pos[sorted_a[i]] = i;

    vector<int> perm(n);
    for (int i = 0; i < n; i++) perm[i] = pos[a[i]];

    vector<bool> visited(n, false);
    int swaps = 0;
    for (int i = 0; i < n; i++) {
        if (visited[i] || perm[i] == i) continue;
        int cycleLen = 0;
        int j = i;
        while (!visited[j]) { visited[j] = true; j = perm[j]; cycleLen++; }
        swaps += cycleLen - 1;
    }
    return swaps;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 1 : uniform_int_distribution<int>(1, 5000)(rng);
    vector<long long> a(n);
    for (int i = 0; i < n; i++) a[i] = i + 1; // різні числа гарантовано
    shuffle(a.begin(), a.end(), rng);

    int ans = solveRef(n, a);
    stringstream in; in << n << "\n";
    for (int i = 0; i < n; i++) in << a[i] << (i+1<n?' ':'\n');
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Мінімум перестановок (сортування вибором, середня)"
#include "../../common/tester_main_multilang.inc"
