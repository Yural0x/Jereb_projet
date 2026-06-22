// EZ_binary_search_tester.cpp — "Список запрошених"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 1000;

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 1 : uniform_int_distribution<int>(1, 2000)(rng);
    int q = idx == 0 ? 1 : uniform_int_distribution<int>(1, 50)(rng);
    uniform_int_distribution<long long> vd(1, 1000000);
    set<long long> s;
    while ((int)s.size() < n) s.insert(vd(rng));
    vector<long long> a(s.begin(), s.end());
    n = (int)a.size();

    stringstream in; in << n << " " << q << "\n";
    for (int i = 0; i < n; i++) in << a[i] << (i+1<n?' ':'\n');
    stringstream out;
    for (int j = 0; j < q; j++) {
        long long x;
        if (uniform_int_distribution<int>(0,1)(rng)) x = a[uniform_int_distribution<int>(0,n-1)(rng)];
        else x = vd(rng);
        in << x << "\n";
        bool present = binary_search(a.begin(), a.end(), x);
        out << (present ? "YES" : "NO") << "\n";
    }
    TestCase tc; tc.input = in.str();
    string e = out.str();
    if (!e.empty() && e.back() == '\n') e.pop_back();
    tc.expectedOutput = e;
    return tc;
}
#define PROBLEM_NAME "Список запрошених (бінарний пошук)"
#include "../../common/tester_main.inc"
