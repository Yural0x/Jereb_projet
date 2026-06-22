// MED_dfs_tester.cpp — "Дві кімнати"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static bool isBipartite(int n, vector<vector<int>>& adj) {
    vector<int> color(n+1, -1);
    for (int s = 1; s <= n; s++) {
        if (color[s] != -1) continue;
        color[s] = 0;
        queue<int> q; q.push(s);
        while (!q.empty()) {
            int u = q.front(); q.pop();
            for (int v : adj[u]) {
                if (color[v] == -1) { color[v] = 1 - color[u]; q.push(v); }
                else if (color[v] == color[u]) return false;
            }
        }
    }
    return true;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 3 : uniform_int_distribution<int>(1, 2000)(rng);
    bool wantBipartite = uniform_int_distribution<int>(0,1)(rng);
    if (idx == 0) wantBipartite = false;
    vector<pair<int,int>> edges;
    set<pair<int,int>> used;

    if (wantBipartite && n >= 2) {
        // ділимо вершини на дві групи й беремо ребра лише між групами
        vector<int> side(n+1);
        for (int i = 1; i <= n; i++) side[i] = uniform_int_distribution<int>(0,1)(rng);
        int m = uniform_int_distribution<int>(0, min(2000, n*n))(rng);
        int attempts = 0;
        while ((int)edges.size() < m && attempts < m*5+50) {
            attempts++;
            int u = uniform_int_distribution<int>(1,n)(rng);
            int v = uniform_int_distribution<int>(1,n)(rng);
            if (u == v || side[u] == side[v]) continue;
            int a = min(u,v), b = max(u,v);
            if (used.count({a,b})) continue;
            used.insert({a,b}); edges.push_back({a,b});
        }
    } else if (n >= 3) {
        // гарантований непарний цикл (трикутник) + випадкові додаткові ребра
        edges.push_back({1,2}); edges.push_back({2,3}); edges.push_back({1,3});
        used = {{1,2},{2,3},{1,3}};
        int m = uniform_int_distribution<int>(0, min(2000, n*n))(rng);
        int attempts = 0;
        while ((int)edges.size() < m && attempts < m*5+50) {
            attempts++;
            int u = uniform_int_distribution<int>(1,n)(rng);
            int v = uniform_int_distribution<int>(1,n)(rng);
            if (u == v) continue;
            int a = min(u,v), b = max(u,v);
            if (used.count({a,b})) continue;
            used.insert({a,b}); edges.push_back({a,b});
        }
    }
    int m = (int)edges.size();

    vector<vector<int>> adj(n+1);
    for (auto& e : edges) { adj[e.first].push_back(e.second); adj[e.second].push_back(e.first); }
    bool bip = isBipartite(n, adj);

    stringstream in; in << n << " " << m << "\n";
    for (auto& e : edges) in << e.first << " " << e.second << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = bip ? "YES" : "NO";
    return tc;
}
#define PROBLEM_NAME "Дві кімнати (двочастковість, середня)"
#include "../../common/tester_main.inc"
