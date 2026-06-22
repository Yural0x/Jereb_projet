// MED_insertion_sort_tester.cpp — "Турнірна таблиця"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 1000;

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 1 : uniform_int_distribution<int>(1, 2000)(rng);
    uniform_int_distribution<long long> sd(0, 1000000000LL);
    uniform_int_distribution<int> letDist(0, 25);
    int nameLen = 5;
    vector<pair<string,long long>> students(n);
    set<string> usedNames;
    for (int i = 0; i < n; i++) {
        string name;
        do {
            name.clear();
            for (int k = 0; k < nameLen; k++) name.push_back('a' + letDist(rng));
            name += to_string(i); // гарантуємо унікальність простим способом
        } while (usedNames.count(name));
        usedNames.insert(name);
        students[i] = {name, sd(rng)};
    }

    vector<pair<string,long long>> sortedS = students;
    stable_sort(sortedS.begin(), sortedS.end(), [](const pair<string,long long>& A, const pair<string,long long>& B){
        if (A.second != B.second) return A.second > B.second; // спадання балів
        return A.first < B.first; // за алфавітом
    });

    stringstream in; in << n << "\n";
    for (int i = 0; i < n; i++) in << students[i].first << " " << students[i].second << "\n";
    stringstream out;
    for (int i = 0; i < n; i++) out << sortedS[i].first << "\n";
    string e = out.str();
    if (!e.empty() && e.back() == '\n') e.pop_back();
    TestCase tc; tc.input = in.str(); tc.expectedOutput = e;
    return tc;
}
#define PROBLEM_NAME "Турнірна таблиця (сортування вставками, середня)"
#include "../../common/tester_main.inc"
