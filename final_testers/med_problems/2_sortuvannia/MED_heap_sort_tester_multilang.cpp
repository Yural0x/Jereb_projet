// MED_heap_sort_tester.cpp — "Найбільші k"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 1 : uniform_int_distribution<int>(1, 20000)(rng);
    int k = uniform_int_distribution<int>(1, n)(rng);
    uniform_int_distribution<long long> vd(-1000000000LL, 1000000000LL);
    vector<long long> a(n);
    for (auto& v : a) v = vd(rng);

    vector<long long> b = a;
    sort(b.rbegin(), b.rend());

    stringstream in; in << n << " " << k << "\n";
    for (int i = 0; i < n; i++) in << a[i] << (i+1<n?' ':'\n');
    stringstream out;
    for (int i = 0; i < k; i++) out << b[i] << (i+1<k?' ':' ');
    string e = out.str();
    while (!e.empty() && e.back() == ' ') e.pop_back();
    TestCase tc; tc.input = in.str(); tc.expectedOutput = e;
    return tc;
}
#define PROBLEM_NAME "Найбільші k (пірамідальне сортування, середня)"
#include "../../common/tester_main_multilang.inc"
