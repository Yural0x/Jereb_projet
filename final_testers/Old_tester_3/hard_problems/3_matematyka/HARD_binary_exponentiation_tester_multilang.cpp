// HARD_binary_exponentiation_tester.cpp — "Фібоначчі за матрицею"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 1000;
static const long long MOD = 1000000007LL;

struct Mat { long long a, b, c, d; }; // [[a,b],[c,d]]
static Mat matMul(const Mat& X, const Mat& Y) {
    return {
        (X.a*Y.a + X.b*Y.c) % MOD,
        (X.a*Y.b + X.b*Y.d) % MOD,
        (X.c*Y.a + X.d*Y.c) % MOD,
        (X.c*Y.b + X.d*Y.d) % MOD
    };
}
static Mat matPow(Mat base, long long n) {
    Mat result = {1,0,0,1};
    while (n > 0) {
        if (n & 1) result = matMul(result, base);
        base = matMul(base, base);
        n >>= 1;
    }
    return result;
}
static long long fib(long long n) {
    if (n == 0) return 0;
    Mat base = {1,1,1,0};
    Mat r = matPow(base, n - 1);
    return r.a; // F(n) = (base^(n-1))[0][0] для F(1)=F(2)=1
}

static TestCase genTest(mt19937& rng, int idx) {
    uniform_int_distribution<long long> nd(1, 1000000000000000000LL);
    long long n = idx == 0 ? 10 : (idx == 1 ? 20 : nd(rng));

    long long ans = fib(n);
    stringstream in; in << n << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Фібоначчі за матрицею (складна)"
#include "../../common/tester_main_multilang.inc"
