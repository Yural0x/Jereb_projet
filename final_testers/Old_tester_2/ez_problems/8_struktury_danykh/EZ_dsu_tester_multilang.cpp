// EZ_dsu_tester.cpp — "Знайомства в гуртожитку"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static int solveRef(int n, vector<pair<int,int>>& edges) {
    vector<int> parent(n+1);
    for (int i = 0; i <= n; i++) parent[i] = i;
    function<int(int)> find = [&](int x){ while (parent[x]!=x) x = parent[x] = parent[parent[x]]; return x; };
    int comps = n;
    for (auto& e : edges) {
        int ra = find(e.first), rb = find(e.second);
        if (ra != rb) { parent[ra] = rb; comps--; }
    }
    return comps;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 5 : uniform_int_distribution<int>(1, 2000)(rng);
    int m = idx == 0 ? 3 : uniform_int_distribution<int>(0, min(2000, n))(rng);
    vector<pair<int,int>> edges;
    uniform_int_distribution<int> nd(1, max(n,1));
    if (idx == 0) { edges = {{1,2},{2,3},{4,5}}; }
    else {
        for (int i = 0; i < m; i++) {
            int a = nd(rng), b = nd(rng);
            if (a == b) continue;
            edges.push_back({a,b});
        }
    }
    m = (int)edges.size();

    int ans = solveRef(n, edges);
    stringstream in; in << n << " " << m << "\n";
    for (auto& e : edges) in << e.first << " " << e.second << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Знайомства в гуртожитку (DSU)"
#include "../../common/tester_main_multilang.inc"
