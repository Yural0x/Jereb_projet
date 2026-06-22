// MED_segment_tree_tester.cpp — "Максимум на відрізку"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 5 : uniform_int_distribution<int>(1, 3000)(rng);
    int q = idx == 0 ? 3 : uniform_int_distribution<int>(1, 300)(rng);
    uniform_int_distribution<long long> vd(-1000000000LL, 1000000000LL);
    vector<long long> a(n);
    for (auto& v : a) v = vd(rng);
    if (idx == 0) a = {1,5,2,8,3};

    stringstream in; in << n << " " << q << "\n";
    for (int i = 0; i < n; i++) in << a[i] << (i+1<n?' ':'\n');
    stringstream out;
    uniform_int_distribution<int> nd(1, n);
    for (int t = 0; t < q; t++) {
        bool isUpdate;
        if (idx == 0) isUpdate = (t == 1);
        else isUpdate = uniform_int_distribution<int>(0,1)(rng);
        if (isUpdate) {
            int i = idx==0 ? 2 : nd(rng);
            long long x = idx==0 ? 9 : vd(rng);
            in << "= " << i << " " << x << "\n";
            a[i-1] = x;
        } else {
            int l, r;
            if (idx == 0) { l=1; r=3; }
            else { l = nd(rng); r = uniform_int_distribution<int>(l, n)(rng); }
            in << "? " << l << " " << r << "\n";
            long long mx = LLONG_MIN;
            for (int j = l-1; j < r; j++) mx = max(mx, a[j]);
            out << mx << "\n";
        }
    }
    string e = out.str();
    if (!e.empty() && e.back() == '\n') e.pop_back();
    TestCase tc; tc.input = in.str(); tc.expectedOutput = e;
    return tc;
}
#define PROBLEM_NAME "Максимум на відрізку (Segment Tree, середня)"
#include "../../common/tester_main_multilang.inc"
