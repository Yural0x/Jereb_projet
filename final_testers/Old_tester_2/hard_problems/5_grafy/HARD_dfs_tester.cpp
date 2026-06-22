// HARD_dfs_tester.cpp — "Критичні з'єднання"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static int countBridges(int n, vector<vector<pair<int,int>>>& adj) {
    // adj[u] = список (сусід, id_ребра); для пропуску зворотного ребра по тому ж id
    vector<int> disc(n+1,-1), low(n+1,-1);
    int timer = 0, bridges = 0;
    function<void(int,int)> dfs = [&](int u, int parentEdge) {
        disc[u] = low[u] = timer++;
        for (auto& [v, eid] : adj[u]) {
            if (eid == parentEdge) continue;
            if (disc[v] == -1) {
                dfs(v, eid);
                low[u] = min(low[u], low[v]);
                if (low[v] > disc[u]) bridges++;
            } else {
                low[u] = min(low[u], disc[v]);
            }
        }
    };
    for (int i = 1; i <= n; i++) if (disc[i] == -1) dfs(i, -1);
    return bridges;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 4 : uniform_int_distribution<int>(1, 1500)(rng);
    vector<pair<int,int>> edges;
    set<pair<int,int>> used;
    if (idx == 0) { edges = {{1,2},{2,3},{3,1},{3,4}}; }
    else if (n >= 2) {
        vector<int> perm(n);
        for (int i = 0; i < n; i++) perm[i] = i+1;
        shuffle(perm.begin(), perm.end(), rng);
        for (int i = 1; i < n; i++) {
            int u = perm[i], v = perm[uniform_int_distribution<int>(0,i-1)(rng)];
            int a = min(u,v), b = max(u,v);
            if (!used.count({a,b})) { used.insert({a,b}); edges.push_back({a,b}); }
        }
        int extra = uniform_int_distribution<int>(0, min(2000, n))(rng);
        for (int i = 0; i < extra; i++) {
            int u = uniform_int_distribution<int>(1,n)(rng);
            int v = uniform_int_distribution<int>(1,n)(rng);
            if (u==v) continue;
            int a=min(u,v), b=max(u,v);
            if (used.count({a,b})) continue;
            used.insert({a,b}); edges.push_back({a,b});
        }
    }
    int m = (int)edges.size();

    vector<vector<pair<int,int>>> adj(n+1);
    for (int i = 0; i < m; i++) {
        adj[edges[i].first].push_back({edges[i].second, i});
        adj[edges[i].second].push_back({edges[i].first, i});
    }
    int ans = countBridges(n, adj);

    stringstream in; in << n << " " << m << "\n";
    for (auto& e : edges) in << e.first << " " << e.second << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Критичні з'єднання (мости, складна)"
#include "../../common/tester_main.inc"
