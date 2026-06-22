// MED_dijkstra_tester.cpp — "Скільки найкоротших"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;
static const long long MOD = 1000000007LL;

static long long solveRef(int n, vector<vector<pair<int,long long>>>& adj) {
    vector<long long> dist(n+1, LLONG_MAX);
    vector<long long> ways(n+1, 0);
    priority_queue<pair<long long,int>, vector<pair<long long,int>>, greater<>> pq;
    dist[1] = 0; ways[1] = 1; pq.push({0,1});
    while (!pq.empty()) {
        auto [d,u] = pq.top(); pq.pop();
        if (d > dist[u]) continue;
        for (auto& [v,w] : adj[u]) {
            long long nd = d + w;
            if (nd < dist[v]) { dist[v] = nd; ways[v] = ways[u]; pq.push({nd,v}); }
            else if (nd == dist[v]) { ways[v] = (ways[v] + ways[u]) % MOD; }
        }
    }
    return dist[n] == LLONG_MAX ? 0 : ways[n] % MOD;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 4 : uniform_int_distribution<int>(2, 1500)(rng);
    vector<pair<pair<int,int>, long long>> edges;
    set<pair<int,int>> used;
    uniform_int_distribution<long long> wd(1, 1000000000LL);
    vector<int> perm(n);
    for (int i = 0; i < n; i++) perm[i] = i+1;
    shuffle(perm.begin(), perm.end(), rng);
    for (int i = 1; i < n; i++) {
        int u = perm[i], v = perm[uniform_int_distribution<int>(0,i-1)(rng)];
        if (u > v) swap(u,v);
        if (!used.count({u,v})) { used.insert({u,v}); edges.push_back({{u,v}, wd(rng)}); }
    }
    int extra = uniform_int_distribution<int>(0, min(2000,n))(rng);
    for (int i = 0; i < extra; i++) {
        int u = uniform_int_distribution<int>(1,n)(rng);
        int v = uniform_int_distribution<int>(1,n)(rng);
        if (u==v) continue;
        if (u>v) swap(u,v);
        if (used.count({u,v})) continue;
        used.insert({u,v}); edges.push_back({{u,v}, wd(rng)});
    }
    int m = (int)edges.size();

    vector<vector<pair<int,long long>>> adj(n+1);
    for (auto& e : edges) {
        adj[e.first.first].push_back({e.first.second, e.second});
        adj[e.first.second].push_back({e.first.first, e.second});
    }
    long long ans = solveRef(n, adj);

    stringstream in; in << n << " " << m << "\n";
    for (auto& e : edges) in << e.first.first << " " << e.first.second << " " << e.second << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Скільки найкоротших (Дейкстра, середня)"
#include "../../common/tester_main_multilang.inc"
