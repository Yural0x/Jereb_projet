// HARD_heap_sort_tester.cpp — "Медіана потоку"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static vector<long long> solveRef(const vector<long long>& a) {
    // дві купи: maxHeap для меншої половини, minHeap для більшої.
    priority_queue<long long> lo;                                  // max-heap
    priority_queue<long long, vector<long long>, greater<long long>> hi; // min-heap
    vector<long long> medians;
    for (long long x : a) {
        if (lo.empty() || x <= lo.top()) lo.push(x); else hi.push(x);
        // балансування: lo має дорівнювати hi або hi+1 елементів (менше з двох - в lo)
        if (lo.size() > hi.size() + 1) { hi.push(lo.top()); lo.pop(); }
        else if (hi.size() > lo.size()) { lo.push(hi.top()); hi.pop(); }
        medians.push_back(lo.top()); // менше з двох центральних значень при парній кількості
    }
    return medians;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 1 : uniform_int_distribution<int>(1, 200000)(rng);
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
#define PROBLEM_NAME "Медіана потоку (складна)"
#include "../../common/tester_main_multilang.inc"
