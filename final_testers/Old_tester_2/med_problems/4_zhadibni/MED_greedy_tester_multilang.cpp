// MED_greedy_tester.cpp — "Розклад аудиторії"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 1000;

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 4 : uniform_int_distribution<int>(1, 5000)(rng);
    uniform_int_distribution<long long> sd(0, 1000000000LL);
    vector<pair<long long,long long>> jobs(n);
    for (auto& j : jobs) {
        long long s = sd(rng);
        long long f = s + uniform_int_distribution<long long>(1, 1000000)(rng);
        j = {s, f};
    }

    vector<pair<long long,long long>> sorted_j = jobs;
    sort(sorted_j.begin(), sorted_j.end(), [](const pair<long long,long long>& a, const pair<long long,long long>& b){
        return a.second < b.second;
    });
    long long lastEnd = LLONG_MIN;
    int count = 0;
    for (auto& j : sorted_j) {
        if (j.first >= lastEnd) { count++; lastEnd = j.second; }
    }

    stringstream in; in << n << "\n";
    for (auto& j : jobs) in << j.first << " " << j.second << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(count);
    return tc;
}
#define PROBLEM_NAME "Розклад аудиторії (жадібний, середня)"
#include "../../common/tester_main_multilang.inc"
