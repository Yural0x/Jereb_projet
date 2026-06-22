// MED_euclidean_gcd_tester.cpp — "Найменше спільне кратне"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 1000;

static TestCase genTest(mt19937& rng, int idx) {
    uniform_int_distribution<long long> vd(1, 1000000000LL);
    long long a = idx == 0 ? 4 : vd(rng);
    long long b = idx == 0 ? 6 : vd(rng);
    long long g = __gcd(a, b);
    long long lcm = (a / g) * b;

    stringstream in; in << a << " " << b << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(lcm);
    return tc;
}
#define PROBLEM_NAME "Найменше спільне кратне (середня)"
#include "../../common/tester_main_multilang.inc"
