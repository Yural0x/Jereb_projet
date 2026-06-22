// HARD_kmp_tester.cpp — "Усі межі рядка"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static vector<int> prefixFunction(const string& s) {
    int n = (int)s.size();
    vector<int> pi(n, 0);
    for (int i = 1; i < n; i++) {
        int j = pi[i-1];
        while (j > 0 && s[i] != s[j]) j = pi[j-1];
        if (s[i] == s[j]) j++;
        pi[i] = j;
    }
    return pi;
}
// Усі межі - це ланцюжок pi[n-1], pi[pi[n-1]-1], ... до 0.
static vector<int> allBorders(const string& s) {
    int n = (int)s.size();
    vector<int> pi = prefixFunction(s);
    vector<int> borders;
    int cur = pi[n-1];
    while (cur > 0) { borders.push_back(cur); cur = pi[cur-1]; }
    sort(borders.begin(), borders.end());
    return borders;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 7 : uniform_int_distribution<int>(1, 1000000)(rng);
    string s;
    if (idx == 0) s = "abacaba";
    else {
        bool periodic = uniform_int_distribution<int>(0,1)(rng);
        uniform_int_distribution<int> ld(0, 2);
        if (periodic && n >= 2) {
            int blockLen = uniform_int_distribution<int>(1, max(1,n/3+1))(rng);
            string block(blockLen, 'a');
            for (auto& c : block) c = 'a' + ld(rng);
            s.clear();
            while ((int)s.size() < n) s += block;
            s = s.substr(0, n);
        } else {
            s.assign(n, 'a');
            for (auto& c : s) c = 'a' + ld(rng);
        }
    }
    n = (int)s.size();

    vector<int> borders = allBorders(s);
    stringstream in; in << s << "\n";
    stringstream out;
    for (size_t i = 0; i < borders.size(); i++) out << borders[i] << (i+1<borders.size()?' ':' ');
    string e = out.str();
    while (!e.empty() && e.back() == ' ') e.pop_back();
    TestCase tc; tc.input = in.str(); tc.expectedOutput = e;
    return tc;
}
#define PROBLEM_NAME "Усі межі рядка (префікс-функція, складна)"
#include "../../common/tester_main.inc"
