// EZ_euclidean_gcd_tester.cpp — "Спільний дільник"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 1000;

static TestCase genTest(mt19937& rng, int idx) {
    uniform_int_distribution<long long> vd(1, (long long)1e18);
    long long a = idx == 0 ? 12 : vd(rng);
    long long b = idx == 0 ? 18 : vd(rng);
    long long g = __gcd(a, b);

    stringstream in; in << a << " " << b << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(g);
    return tc;
}
#define PROBLEM_NAME "Спільний дільник (алгоритм Евкліда)"
#include "../../common/tester_main_multilang.inc"
