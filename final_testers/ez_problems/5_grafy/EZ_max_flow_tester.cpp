// EZ_max_flow_tester.cpp — "Пропускна здатність труб"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

struct Edge { int to; long long cap; int rev; };
static vector<vector<Edge>> graph;
static void addEdge(int u, int v, long long cap) {
    graph[u].push_back({v, cap, (int)graph[v].size()});
    graph[v].push_back({u, 0, (int)graph[u].size() - 1});
}
static long long bfsAugment(int n, int s, int t, vector<int>& prevV, vector<int>& prevE) {
    vector<bool> visited(n+1, false);
    queue<int> q; q.push(s); visited[s] = true;
    prevV.assign(n+1, -1); prevE.assign(n+1, -1);
    while (!q.empty()) {
        int u = q.front(); q.pop();
        if (u == t) break;
        for (int i = 0; i < (int)graph[u].size(); i++) {
            auto& e = graph[u][i];
            if (!visited[e.to] && e.cap > 0) {
                visited[e.to] = true;
                prevV[e.to] = u; prevE[e.to] = i;
                q.push(e.to);
            }
        }
    }
    if (!visited[t]) return 0;
    long long flow = LLONG_MAX;
    for (int v = t; v != s; v = prevV[v]) flow = min(flow, graph[prevV[v]][prevE[v]].cap);
    for (int v = t; v != s; v = prevV[v]) {
        graph[prevV[v]][prevE[v]].cap -= flow;
        int rev = graph[prevV[v]][prevE[v]].rev;
        graph[v][rev].cap += flow;
    }
    return flow;
}
static long long maxFlow(int n, int s, int t) {
    long long total = 0;
    vector<int> pv, pe;
    while (true) {
        long long f = bfsAugment(n, s, t, pv, pe);
        if (f == 0) break;
        total += f;
    }
    return total;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 4 : uniform_int_distribution<int>(2, 150)(rng);
    vector<array<long long,3>> edges;
    uniform_int_distribution<long long> cd(1, 1000000);
    int m = uniform_int_distribution<int>(1, min(800, n*(n-1)))(rng);
    set<pair<int,int>> used;
    int attempts = 0;
    while ((int)edges.size() < m && attempts < m * 5 + 50) {
        attempts++;
        int u = uniform_int_distribution<int>(1, n)(rng);
        int v = uniform_int_distribution<int>(1, n)(rng);
        if (u == v) continue;
        if (used.count({u,v})) continue;
        used.insert({u,v});
        edges.push_back({u, v, cd(rng)});
    }
    m = (int)edges.size();

    graph.assign(n+1, {});
    for (auto& e : edges) addEdge((int)e[0], (int)e[1], e[2]);
    long long ans = maxFlow(n, 1, n);

    stringstream in; in << n << " " << m << "\n";
    for (auto& e : edges) in << e[0] << " " << e[1] << " " << e[2] << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Пропускна здатність труб (максимальний потік)"
#include "../../common/tester_main.inc"
