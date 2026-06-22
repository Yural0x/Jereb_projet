// HARD_dijkstra_tester.cpp — "Безкоштовні ділянки"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static long long solveRef(int n, int K, vector<vector<pair<int,long long>>>& adj) {
    // стан (вершина, кількість використаних безкоштовних доріг)
    vector<vector<long long>> dist(n+1, vector<long long>(K+1, LLONG_MAX));
    priority_queue<array<long long,3>, vector<array<long long,3>>, greater<>> pq; // {dist, vertex, usedFree}
    dist[1][0] = 0;
    pq.push({0, 1, 0});
    while (!pq.empty()) {
        auto [d, u, used] = pq.top(); pq.pop();
        if (d > dist[u][used]) continue;
        for (auto& [v, w] : adj[u]) {
            // не використати безкоштовну
            if (d + w < dist[v][used]) { dist[v][used] = d + w; pq.push({dist[v][used], v, used}); }
            // використати безкоштовну (якщо лишились)
            if (used < K && d < dist[v][used+1]) { dist[v][used+1] = d; pq.push({dist[v][used+1], v, used+1}); }
        }
    }
    long long best = LLONG_MAX;
    for (int k = 0; k <= K; k++) best = min(best, dist[n][k]);
    return best;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 4 : uniform_int_distribution<int>(2, 800)(rng);
    int K = idx == 0 ? 1 : uniform_int_distribution<int>(0, 10)(rng);
    vector<array<long long,3>> edges;
    set<pair<int,int>> used;
    uniform_int_distribution<long long> wd(1, 1000000000LL);
    vector<int> perm(n);
    for (int i = 0; i < n; i++) perm[i] = i+1;
    shuffle(perm.begin(), perm.end(), rng);
    for (int i = 1; i < n; i++) {
        int u = perm[i], v = perm[uniform_int_distribution<int>(0,i-1)(rng)];
        int a=min(u,v), b=max(u,v);
        if (!used.count({a,b})) { used.insert({a,b}); edges.push_back({a,b,wd(rng)}); }
    }
    int extra = uniform_int_distribution<int>(0, min(1500,n))(rng);
    for (int i = 0; i < extra; i++) {
        int u = uniform_int_distribution<int>(1,n)(rng);
        int v = uniform_int_distribution<int>(1,n)(rng);
        if (u==v) continue;
        int a=min(u,v), b=max(u,v);
        if (used.count({a,b})) continue;
        used.insert({a,b}); edges.push_back({a,b,wd(rng)});
    }
    int m = (int)edges.size();

    vector<vector<pair<int,long long>>> adj(n+1);
    for (auto& e : edges) {
        adj[e[0]].push_back({(int)e[1], e[2]});
        adj[e[1]].push_back({(int)e[0], e[2]});
    }
    long long ans = solveRef(n, K, adj);

    stringstream in; in << n << " " << m << " " << K << "\n";
    for (auto& e : edges) in << e[0] << " " << e[1] << " " << e[2] << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Безкоштовні ділянки (Дейкстра зі станом, складна)"
#include "../../common/tester_main.inc"
