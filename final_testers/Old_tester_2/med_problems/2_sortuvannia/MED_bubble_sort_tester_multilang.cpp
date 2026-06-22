// MED_bubble_sort_tester.cpp — "Кількість обмінів"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 1000;

static long long countInversions(vector<long long> a) {
    // merge sort підрахунок інверсій, O(n log n), щоб генератор встигав на n до 5000
    int n = (int)a.size();
    vector<long long> tmp(n);
    long long inv = 0;
    function<void(int,int)> ms = [&](int l, int r) {
        if (r - l <= 1) return;
        int mid = (l + r) / 2;
        ms(l, mid); ms(mid, r);
        int i = l, j = mid, k = l;
        while (i < mid && j < r) {
            if (a[i] <= a[j]) tmp[k++] = a[i++];
            else { inv += (mid - i); tmp[k++] = a[j++]; }
        }
        while (i < mid) tmp[k++] = a[i++];
        while (j < r) tmp[k++] = a[j++];
        for (int t = l; t < r; t++) a[t] = tmp[t];
    };
    ms(0, n);
    return inv;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 1 : uniform_int_distribution<int>(1, 5000)(rng);
    uniform_int_distribution<long long> vd(-1000000000LL, 1000000000LL);
    vector<long long> a(n);
    for (auto& v : a) v = vd(rng);

    long long inv = countInversions(a);
    stringstream in; in << n << "\n";
    for (int i = 0; i < n; i++) in << a[i] << (i+1<n?' ':'\n');
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(inv);
    return tc;
}
#define PROBLEM_NAME "Кількість обмінів (бульбашка, середня)"
#include "../../common/tester_main_multilang.inc"
