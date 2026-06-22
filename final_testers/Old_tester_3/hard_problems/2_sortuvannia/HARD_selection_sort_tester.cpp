// HARD_selection_sort_tester.cpp — "Дешеве сортування обмінами"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static long long solveRef(int n, vector<long long>& a) {
    vector<long long> sorted_a = a;
    sort(sorted_a.begin(), sorted_a.end());
    unordered_map<long long,int> pos;
    for (int i = 0; i < n; i++) pos[sorted_a[i]] = i;
    vector<int> perm(n);
    for (int i = 0; i < n; i++) perm[i] = pos[a[i]];

    long long globalMin = *min_element(a.begin(), a.end());
    vector<bool> visited(n, false);
    long long total = 0;
    for (int i = 0; i < n; i++) {
        if (visited[i] || perm[i] == i) continue;
        vector<int> cycleIdx;
        int j = i;
        while (!visited[j]) { visited[j] = true; cycleIdx.push_back(j); j = perm[j]; }
        int len = (int)cycleIdx.size();
        if (len <= 1) continue;
        long long cycleSum = 0, cycleMin = LLONG_MAX;
        for (int id : cycleIdx) { cycleSum += a[id]; cycleMin = min(cycleMin, a[id]); }
        // Варіант 1: використовувати власний мінімум циклу
        long long cost1 = cycleSum + (long long)(len - 2) * cycleMin;
        // Варіант 2: позичити глобальний мінімум (2 додаткові обміни з ним)
        long long cost2 = cycleSum + cycleMin + (long long)(len + 1) * globalMin;
        total += min(cost1, cost2);
    }
    return total;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 1 : uniform_int_distribution<int>(1, 2000)(rng);
    vector<long long> vals(n);
    set<long long> used;
    uniform_int_distribution<long long> vd(1, 1000000000LL);
    while ((int)used.size() < n) used.insert(vd(rng));
    vals.assign(used.begin(), used.end());
    shuffle(vals.begin(), vals.end(), rng);
    n = (int)vals.size();

    long long ans = solveRef(n, vals);
    stringstream in; in << n << "\n";
    for (int i = 0; i < n; i++) in << vals[i] << (i+1<n?' ':'\n');
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Дешеве сортування обмінами (складна)"
#include "../../common/tester_main.inc"
