// EZ_dfs_tester.cpp — "Компанії в групі"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static int countComponents(int n, vector<vector<int>>& adj) {
    vector<bool> visited(n+1, false);
    int comps = 0;
    for (int s = 1; s <= n; s++) {
        if (visited[s]) continue;
        comps++;
        vector<int> stack = {s};
        visited[s] = true;
        while (!stack.empty()) {
            int u = stack.back(); stack.pop_back();
            for (int v : adj[u]) if (!visited[v]) { visited[v] = true; stack.push_back(v); }
        }
    }
    return comps;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 5 : uniform_int_distribution<int>(1, 2000)(rng);
    int maxM = min(20000, n * (n - 1) / 2 > 0 ? n * (n - 1) / 2 : 0);
    int m = (n <= 1) ? 0 : uniform_int_distribution<int>(0, min(maxM, 4000))(rng);
    set<pair<int,int>> usedEdges;
    vector<pair<int,int>> edges;
    uniform_int_distribution<int> nd(1, n);
    int attempts = 0;
    while ((int)edges.size() < m && attempts < m * 10 + 100) {
        attempts++;
        int u = nd(rng), v = nd(rng);
        if (u == v) continue;
        if (u > v) swap(u, v);
        if (usedEdges.count({u,v})) continue;
        usedEdges.insert({u,v});
        edges.push_back({u,v});
    }
    m = (int)edges.size();

    vector<vector<int>> adj(n+1);
    for (auto& e : edges) { adj[e.first].push_back(e.second); adj[e.second].push_back(e.first); }
    int comps = countComponents(n, adj);

    stringstream in; in << n << " " << m << "\n";
    for (auto& e : edges) in << e.first << " " << e.second << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(comps);
    return tc;
}
#define PROBLEM_NAME "Компанії в групі (DFS)"
#include "../../common/tester_main.inc"
