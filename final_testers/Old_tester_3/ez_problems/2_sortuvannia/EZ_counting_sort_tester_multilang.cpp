// EZ_counting_sort_tester.cpp — "Бали ЗНО"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 1000;

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 1 : uniform_int_distribution<int>(1, 5000)(rng);
    uniform_int_distribution<int> vd(0, 100);
    vector<int> a(n);
    for (auto& v : a) v = vd(rng);
    vector<int> sorted_a = a;
    sort(sorted_a.begin(), sorted_a.end());

    stringstream in; in << n << "\n";
    for (int i = 0; i < n; i++) in << a[i] << (i+1<n?' ':'\n');
    stringstream out;
    for (int i = 0; i < n; i++) out << sorted_a[i] << (i+1<n?' ':' ');
    string e = out.str();
    while (!e.empty() && e.back() == ' ') e.pop_back();
    TestCase tc; tc.input = in.str(); tc.expectedOutput = e;
    return tc;
}
#define PROBLEM_NAME "Бали ЗНО (сортування підрахунком)"
#include "../../common/tester_main_multilang.inc"
