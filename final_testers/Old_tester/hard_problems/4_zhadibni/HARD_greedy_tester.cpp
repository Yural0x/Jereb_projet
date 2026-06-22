// HARD_greedy_tester.cpp — "Розклад робіт із дедлайнами"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

// Жадібний: сортуємо за прибутком (спад), для кожної роботи шукаємо найпізніший
// вільний слот <= дедлайну (DSU "знайти найбільший вільний слот, не більший за x").
static long long solveRef(int n, vector<pair<int,long long>>& jobs) {
    vector<int> idx(n);
    for (int i = 0; i < n; i++) idx[i] = i;
    sort(idx.begin(), idx.end(), [&](int a, int b){ return jobs[a].second > jobs[b].second; });
    int maxDeadline = 0;
    for (auto& j : jobs) maxDeadline = max(maxDeadline, j.first);
    vector<int> parent(maxDeadline+1);
    for (int i = 0; i <= maxDeadline; i++) parent[i] = i;
    function<int(int)> find = [&](int x){ while (parent[x]!=x) x=parent[x]=parent[parent[x]]; return x; };
    long long total = 0;
    for (int id : idx) {
        int d = jobs[id].first;
        int slot = find(d);
        if (slot == 0) continue; // немає вільного слота <= d
        total += jobs[id].second;
        parent[slot] = find(slot - 1);
    }
    return total;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 5 : uniform_int_distribution<int>(1, 5000)(rng);
    vector<pair<int,long long>> jobs(n);
    if (idx == 0) jobs = {{2,100},{1,19},{2,27},{1,25},{3,15}};
    else {
        uniform_int_distribution<int> dd(1, n);
        uniform_int_distribution<long long> pd(1, 1000000000LL);
        for (auto& j : jobs) j = {dd(rng), pd(rng)};
    }

    long long ans = solveRef(n, jobs);
    stringstream in; in << n << "\n";
    for (auto& j : jobs) in << j.first << " " << j.second << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Розклад робіт із дедлайнами (жадібний, складна)"
#include "../../common/tester_main.inc"
