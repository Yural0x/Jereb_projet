// HARD_z_function_tester.cpp — "Кількість різних підрядків"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

// Еталон через множину всіх підрядків (O(n^3 log) гірший випадок, але для n<=2000
// з невеликим алфавітом і обмеженою кількістю різних підрядків (через хеш-множину
// рядків) працює прийнятно завдяки коротким рядкам.
static long long countDistinctSubstrings(const string& s) {
    int n = (int)s.size();
    set<string> distinct;
    for (int i = 0; i < n; i++) {
        string cur;
        for (int j = i; j < n; j++) {
            cur.push_back(s[j]);
            distinct.insert(cur);
        }
    }
    return (long long)distinct.size();
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 3 : uniform_int_distribution<int>(1, 700)(rng);
    uniform_int_distribution<int> ld(0, 2); // малий алфавіт - багато повторів підрядків
    string s(n, 'a');
    for (auto& c : s) c = 'a' + ld(rng);
    if (idx == 0) s = "aab";

    long long ans = countDistinctSubstrings(s);
    stringstream in; in << s << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Кількість різних підрядків (складна)"
#include "../../common/tester_main.inc"
