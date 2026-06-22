// HARD_bfs_tester.cpp — "Дороги двох типів"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static long long zeroOneBFS(int n, vector<vector<pair<int,int>>>& adj, int src, int dst) {
    vector<long long> dist(n+1, LLONG_MAX);
    deque<int> dq;
    dist[src] = 0; dq.push_back(src);
    while (!dq.empty()) {
        int u = dq.front(); dq.pop_front();
        for (auto& [v, w] : adj[u]) {
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                if (w == 0) dq.push_front(v); else dq.push_back(v);
            }
        }
    }
    return dist[dst];
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 4 : uniform_int_distribution<int>(1, 3000)(rng);
    vector<array<int,3>> edges; // u v w(0/1)
    set<pair<int,int>> used;
    if (n > 1) {
        vector<int> perm(n);
        for (int i = 0; i < n; i++) perm[i] = i+1;
        shuffle(perm.begin(), perm.end(), rng);
        for (int i = 1; i < n; i++) {
            int u = perm[i], v = perm[uniform_int_distribution<int>(0,i-1)(rng)];
            int a=min(u,v), b=max(u,v);
            if (!used.count({a,b})) {
                used.insert({a,b});
                edges.push_back({a, b, uniform_int_distribution<int>(0,1)(rng)});
            }
        }
    }
    int extra = uniform_int_distribution<int>(0, min(3000, n))(rng);
    for (int i = 0; i < extra; i++) {
        int u = uniform_int_distribution<int>(1, max(n,1))(rng);
        int v = uniform_int_distribution<int>(1, max(n,1))(rng);
        if (u==v) continue;
        int a=min(u,v), b=max(u,v);
        if (used.count({a,b})) continue;
        used.insert({a,b});
        edges.push_back({a, b, uniform_int_distribution<int>(0,1)(rng)});
    }
    int m = (int)edges.size();

    vector<vector<pair<int,int>>> adj(n+1);
    for (auto& e : edges) {
        adj[e[0]].push_back({e[1], e[2]});
        adj[e[1]].push_back({e[0], e[2]});
    }
    long long ans = zeroOneBFS(n, adj, 1, n);

    stringstream in; in << n << " " << m << "\n";
    for (auto& e : edges) in << e[0] << " " << e[1] << " " << e[2] << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = (ans==LLONG_MAX ? "-1" : to_string(ans));
    return tc;
}
#define PROBLEM_NAME "Дороги двох типів (0-1 BFS, складна)"
#include "../../common/tester_main.inc"
