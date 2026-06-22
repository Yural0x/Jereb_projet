// EZ_segment_tree_tester.cpp — "Мінімум на відрізку"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 5 : uniform_int_distribution<int>(1, 2000)(rng);
    int q = idx == 0 ? 3 : uniform_int_distribution<int>(1, 200)(rng);
    uniform_int_distribution<long long> vd(-1000000000LL, 1000000000LL);
    vector<long long> a(n);
    for (auto& v : a) v = vd(rng);
    if (idx == 0) a = {5,3,8,1,9};

    stringstream in; in << n << " " << q << "\n";
    for (int i = 0; i < n; i++) in << a[i] << (i+1<n?' ':'\n');
    stringstream out;
    uniform_int_distribution<int> nd(1, n);
    for (int t = 0; t < q; t++) {
        bool isUpdate;
        if (idx == 0) isUpdate = (t == 1);
        else isUpdate = uniform_int_distribution<int>(0,1)(rng);
        if (isUpdate) {
            int i = nd(rng);
            long long x = (idx == 0 && t == 1) ? 0 : vd(rng);
            if (idx == 0) i = 3;
            in << "= " << i << " " << x << "\n";
            a[i-1] = x;
        } else {
            int l, r;
            if (idx == 0) { l = 2; r = 4; }
            else { l = nd(rng); r = uniform_int_distribution<int>(l, n)(rng); }
            in << "? " << l << " " << r << "\n";
            long long mn = LLONG_MAX;
            for (int j = l-1; j < r; j++) mn = min(mn, a[j]);
            out << mn << "\n";
        }
    }
    string e = out.str();
    if (!e.empty() && e.back() == '\n') e.pop_back();
    TestCase tc; tc.input = in.str(); tc.expectedOutput = e;
    return tc;
}
#define PROBLEM_NAME "Мінімум на відрізку (дерево відрізків)"
#include "../../common/tester_main.inc"
