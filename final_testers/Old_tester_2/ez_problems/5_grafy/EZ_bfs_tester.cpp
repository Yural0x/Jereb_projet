// EZ_bfs_tester.cpp — "Найкоротший маршрут"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static int bfsDist(int n, vector<vector<int>>& adj, int src, int dst) {
    vector<int> dist(n+1, -1);
    queue<int> q;
    dist[src] = 0; q.push(src);
    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (int v : adj[u]) if (dist[v] == -1) { dist[v] = dist[u] + 1; q.push(v); }
    }
    return dist[dst];
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 4 : uniform_int_distribution<int>(1, 2000)(rng);
    vector<pair<int,int>> edges;
    set<pair<int,int>> usedEdges;
    bool connectPath = true;
    if (connectPath && n > 1) {
        vector<int> perm(n);
        for (int i = 0; i < n; i++) perm[i] = i + 1;
        shuffle(perm.begin(), perm.end(), rng);
        // Гарантуємо зв'язність хоча б випадковим дерева
        for (int i = 1; i < n; i++) {
            int u = perm[i], v = perm[uniform_int_distribution<int>(0, i-1)(rng)];
            if (u > v) swap(u, v);
            if (!usedEdges.count({u,v})) { usedEdges.insert({u,v}); edges.push_back({u,v}); }
        }
    }
    int extra = uniform_int_distribution<int>(0, min(2000, n))(rng);
    uniform_int_distribution<int> nd(1, max(n,1));
    for (int i = 0; i < extra; i++) {
        int u = nd(rng), v = nd(rng);
        if (u == v) continue;
        if (u > v) swap(u, v);
        if (usedEdges.count({u,v})) continue;
        usedEdges.insert({u,v}); edges.push_back({u,v});
    }
    int m = (int)edges.size();

    vector<vector<int>> adj(n+1);
    for (auto& e : edges) { adj[e.first].push_back(e.second); adj[e.second].push_back(e.first); }
    int d = bfsDist(n, adj, 1, n);

    stringstream in; in << n << " " << m << "\n";
    for (auto& e : edges) in << e.first << " " << e.second << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(d);
    return tc;
}
#define PROBLEM_NAME "Найкоротший маршрут (BFS)"
#include "../../common/tester_main.inc"
