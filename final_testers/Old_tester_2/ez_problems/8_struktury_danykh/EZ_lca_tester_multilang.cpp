// EZ_lca_tester.cpp — "Спільний предок"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static int findLCA(int n, vector<vector<int>>& adj, int u, int v) {
    vector<int> parent(n+1, 0), depth(n+1, 0);
    vector<bool> visited(n+1, false);
    queue<int> q; q.push(1); visited[1] = true;
    while (!q.empty()) {
        int x = q.front(); q.pop();
        for (int y : adj[x]) if (!visited[y]) { visited[y]=true; parent[y]=x; depth[y]=depth[x]+1; q.push(y); }
    }
    while (depth[u] > depth[v]) u = parent[u];
    while (depth[v] > depth[u]) v = parent[v];
    while (u != v) { u = parent[u]; v = parent[v]; }
    return u;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 5 : uniform_int_distribution<int>(2, 2000)(rng);
    vector<pair<int,int>> edges;
    if (idx == 0) { edges = {{1,2},{1,3},{2,4},{2,5}}; }
    else {
        for (int i = 2; i <= n; i++) {
            int parent = uniform_int_distribution<int>(1, i-1)(rng);
            edges.push_back({parent, i});
        }
    }

    vector<vector<int>> adj(n+1);
    for (auto& e : edges) { adj[e.first].push_back(e.second); adj[e.second].push_back(e.first); }

    int q = idx == 0 ? 2 : uniform_int_distribution<int>(1, 200)(rng);
    stringstream in; in << n << "\n";
    for (auto& e : edges) in << e.first << " " << e.second << "\n";
    in << q << "\n";
    stringstream out;
    uniform_int_distribution<int> nd(1, n);
    for (int t = 0; t < q; t++) {
        int u, v;
        if (idx == 0) { u = (t==0) ? 4 : 4; v = (t==0) ? 5 : 3; }
        else { u = nd(rng); v = nd(rng); }
        in << u << " " << v << "\n";
        int ans = findLCA(n, adj, u, v);
        out << ans << "\n";
    }
    string e = out.str();
    if (!e.empty() && e.back() == '\n') e.pop_back();
    TestCase tc; tc.input = in.str(); tc.expectedOutput = e;
    return tc;
}
#define PROBLEM_NAME "Спільний предок (LCA)"
#include "../../common/tester_main_multilang.inc"
