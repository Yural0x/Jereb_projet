// HARD_sieve_of_eratosthenes_tester.cpp — "Прості множники"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;
static const int MAXV = 10000000;

static TestCase genTest(mt19937& rng, int idx) {
    static vector<int> spf; // найменший простий дільник
    static bool built = false;
    if (!built) {
        spf.assign(MAXV + 1, 0);
        for (int i = 2; i <= MAXV; i++) {
            if (spf[i] == 0) {
                for (long long j = i; j <= MAXV; j += i) if (spf[j] == 0) spf[j] = i;
            }
        }
        built = true;
    }

    int q = idx == 0 ? 1 : uniform_int_distribution<int>(1, 200)(rng);
    stringstream in; in << q << "\n";
    stringstream out;
    uniform_int_distribution<int> vd(2, MAXV);
    for (int j = 0; j < q; j++) {
        int x = idx == 0 ? 12 : vd(rng);
        in << x << "\n";
        int cur = x;
        int distinctCount = 0;
        int lastP = -1;
        while (cur > 1) {
            int p = spf[cur];
            if (p != lastP) { distinctCount++; lastP = p; }
            cur /= p;
        }
        out << distinctCount << "\n";
    }
    string e = out.str();
    if (!e.empty() && e.back() == '\n') e.pop_back();
    TestCase tc; tc.input = in.str(); tc.expectedOutput = e;
    return tc;
}
#define PROBLEM_NAME "Прості множники (складна)"
#include "../../common/tester_main_multilang.inc"
