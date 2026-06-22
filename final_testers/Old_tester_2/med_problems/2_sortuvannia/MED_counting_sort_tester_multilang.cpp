// MED_counting_sort_tester.cpp — "Сортування за частотою"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 1000;

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 1 : uniform_int_distribution<int>(1, 3000)(rng);
    uniform_int_distribution<int> vd(0, 100);
    vector<int> a(n);
    for (auto& v : a) v = vd(rng);

    vector<int> freq(101, 0);
    for (int v : a) freq[v]++;
    vector<int> values;
    for (int v = 0; v <= 100; v++) if (freq[v] > 0) values.push_back(v);
    stable_sort(values.begin(), values.end(), [&](int x, int y){
        if (freq[x] != freq[y]) return freq[x] > freq[y];
        return x < y;
    });

    stringstream out;
    bool first = true;
    for (int v : values) {
        for (int c = 0; c < freq[v]; c++) {
            if (!first) out << ' ';
            out << v;
            first = false;
        }
    }

    stringstream in; in << n << "\n";
    for (int i = 0; i < n; i++) in << a[i] << (i+1<n?' ':'\n');
    TestCase tc; tc.input = in.str(); tc.expectedOutput = out.str();
    return tc;
}
#define PROBLEM_NAME "Сортування за частотою (сортування підрахунком, середня)"
#include "../../common/tester_main_multilang.inc"
