// MED_sieve_of_eratosthenes_tester.cpp — "Прості на відрізку"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static TestCase genTest(mt19937& rng, int idx) {
    int N = idx == 0 ? 10 : uniform_int_distribution<int>(2, 1000000)(rng);
    int q = idx == 0 ? 1 : uniform_int_distribution<int>(1, 30)(rng);

    vector<bool> isComposite(N + 1, false);
    for (int i = 2; i <= N; i++) {
        if (!isComposite[i]) for (long long j = (long long)i*i; j <= N; j += i) isComposite[j] = true;
    }
    vector<int> prefix(N + 1, 0);
    for (int i = 2; i <= N; i++) prefix[i] = prefix[i-1] + (!isComposite[i] ? 1 : 0);
    if (N >= 1) prefix[1] = 0;

    stringstream in; in << N << " " << q << "\n";
    stringstream out;
    for (int j = 0; j < q; j++) {
        int L = uniform_int_distribution<int>(2, N)(rng);
        int R = uniform_int_distribution<int>(L, N)(rng);
        in << L << " " << R << "\n";
        out << (prefix[R] - prefix[L-1]) << "\n";
    }
    string e = out.str();
    if (!e.empty() && e.back() == '\n') e.pop_back();
    TestCase tc; tc.input = in.str(); tc.expectedOutput = e;
    return tc;
}
#define PROBLEM_NAME "Прості на відрізку (середня)"
#include "../../common/tester_main.inc"
