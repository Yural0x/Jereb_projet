// EZ_dijkstra_tester.cpp — "Найдешевша дорога"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static long long dijkstra(int n, vector<vector<pair<int,long long>>>& adj, int src, int dst) {
    vector<long long> dist(n+1, LLONG_MAX);
    priority_queue<pair<long long,int>, vector<pair<long long,int>>, greater<>> pq;
    dist[src] = 0; pq.push({0, src});
    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > dist[u]) continue;
        for (auto& [v, w] : adj[u]) {
            if (dist[u] + w < dist[v]) { dist[v] = dist[u] + w; pq.push({dist[v], v}); }
        }
    }
    return dist[dst] == LLONG_MAX ? -1 : dist[dst];
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 3 : uniform_int_distribution<int>(1, 1500)(rng);
    vector<pair<pair<int,int>, long long>> edges;
    set<pair<int,int>> used;
    uniform_int_distribution<long long> wd(1, 1000000000LL);
    if (n > 1) {
        vector<int> perm(n);
        for (int i = 0; i < n; i++) perm[i] = i + 1;
        shuffle(perm.begin(), perm.end(), rng);
        for (int i = 1; i < n; i++) {
            int u = perm[i], v = perm[uniform_int_distribution<int>(0, i-1)(rng)];
            if (u > v) swap(u, v);
            if (!used.count({u,v})) { used.insert({u,v}); edges.push_back({{u,v}, wd(rng)}); }
        }
    }
    int extra = uniform_int_distribution<int>(0, min(2000, n))(rng);
    uniform_int_distribution<int> nd(1, max(n,1));
    for (int i = 0; i < extra; i++) {
        int u = nd(rng), v = nd(rng);
        if (u == v) continue;
        if (u > v) swap(u, v);
        if (used.count({u,v})) continue;
        used.insert({u,v}); edges.push_back({{u,v}, wd(rng)});
    }
    int m = (int)edges.size();

    vector<vector<pair<int,long long>>> adj(n+1);
    for (auto& e : edges) {
        adj[e.first.first].push_back({e.first.second, e.second});
        adj[e.first.second].push_back({e.first.first, e.second});
    }
    long long ans = dijkstra(n, adj, 1, n);

    stringstream in; in << n << " " << m << "\n";
    for (auto& e : edges) in << e.first.first << " " << e.first.second << " " << e.second << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Найдешевша дорога (Дейкстра)"
#include "../../common/tester_main_multilang.inc"
