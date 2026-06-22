// EZ_sieve_of_eratosthenes_tester.cpp — "Прості до N"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 1000;

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 10 : uniform_int_distribution<int>(2, 1000000)(rng);
    vector<bool> isComposite(n + 1, false);
    vector<int> primes;
    for (int i = 2; i <= n; i++) {
        if (!isComposite[i]) {
            primes.push_back(i);
            for (long long j = (long long)i * i; j <= n; j += i) isComposite[j] = true;
        }
    }

    stringstream in; in << n << "\n";
    stringstream out;
    for (size_t i = 0; i < primes.size(); i++) out << primes[i] << (i+1<primes.size()?' ':' ');
    string e = out.str();
    while (!e.empty() && e.back() == ' ') e.pop_back();
    TestCase tc; tc.input = in.str(); tc.expectedOutput = e;
    return tc;
}
#define PROBLEM_NAME "Прості до N (решето Ератосфена)"
#include "../../common/tester_main_multilang.inc"
