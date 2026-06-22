// MED_trie_tester.cpp — "Скільки слів з префіксом"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static string randWord(mt19937& rng, int len, int alphabetSize) {
    uniform_int_distribution<int> ld(0, alphabetSize-1);
    string w(len, 'a');
    for (auto& c : w) c = 'a' + ld(rng);
    return w;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 3 : uniform_int_distribution<int>(1, 1000)(rng);
    int q = idx == 0 ? 3 : uniform_int_distribution<int>(1, 200)(rng);
    int alphabetSize = uniform_int_distribution<int>(1, 4)(rng);
    vector<string> words(n);
    if (idx == 0) words = {"apple","app","apricot"};
    else for (auto& w : words) w = randWord(rng, uniform_int_distribution<int>(1,10)(rng), alphabetSize);

    stringstream in; in << n << "\n";
    for (auto& w : words) in << w << "\n";
    in << q << "\n";
    stringstream out;
    for (int i = 0; i < q; i++) {
        string prefix;
        if (idx == 0) prefix = (i==0) ? "ap" : (i==1 ? "app" : "b");
        else {
            bool useExisting = uniform_int_distribution<int>(0,1)(rng);
            if (useExisting) {
                const string& w = words[uniform_int_distribution<int>(0,n-1)(rng)];
                prefix = w.substr(0, uniform_int_distribution<int>(1,(int)w.size())(rng));
            } else prefix = randWord(rng, uniform_int_distribution<int>(1,10)(rng), alphabetSize);
        }
        in << prefix << "\n";
        int cnt = 0;
        for (auto& w : words) if (w.size() >= prefix.size() && w.compare(0, prefix.size(), prefix) == 0) cnt++;
        out << cnt << "\n";
    }
    string e = out.str();
    if (!e.empty() && e.back() == '\n') e.pop_back();
    TestCase tc; tc.input = in.str(); tc.expectedOutput = e;
    return tc;
}
#define PROBLEM_NAME "Скільки слів з префіксом (Trie, середня)"
#include "../../common/tester_main_multilang.inc"
