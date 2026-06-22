// MED_max_flow_tester.cpp — "Розподіл по кімнатах"
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
static long long bfsAugment(int N, int s, int t, vector<int>& prevV, vector<int>& prevE) {
    vector<bool> visited(N+1, false);
    queue<int> q; q.push(s); visited[s] = true;
    prevV.assign(N+1, -1); prevE.assign(N+1, -1);
    while (!q.empty()) {
        int u = q.front(); q.pop();
        if (u == t) break;
        for (int i = 0; i < (int)graph[u].size(); i++) {
            auto& e = graph[u][i];
            if (!visited[e.to] && e.cap > 0) { visited[e.to]=true; prevV[e.to]=u; prevE[e.to]=i; q.push(e.to); }
        }
    }
    if (!visited[t]) return 0;
    long long flow = LLONG_MAX;
    for (int v=t; v!=s; v=prevV[v]) flow = min(flow, graph[prevV[v]][prevE[v]].cap);
    for (int v=t; v!=s; v=prevV[v]) {
        graph[prevV[v]][prevE[v]].cap -= flow;
        int rev = graph[prevV[v]][prevE[v]].rev;
        graph[v][rev].cap += flow;
    }
    return flow;
}
static long long maxFlow(int N, int s, int t) {
    long long total = 0;
    vector<int> pv, pe;
    while (true) { long long f = bfsAugment(N, s, t, pv, pe); if (f==0) break; total += f; }
    return total;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 2 : uniform_int_distribution<int>(1, 300)(rng);
    int m = idx == 0 ? 2 : uniform_int_distribution<int>(1, 300)(rng);
    vector<pair<int,int>> pairs;
    set<pair<int,int>> used;
    int maxK = n * m;
    int k = idx == 0 ? 3 : uniform_int_distribution<int>(0, min(maxK, 2000))(rng);
    int attempts = 0;
    while ((int)pairs.size() < k && attempts < k*5+50) {
        attempts++;
        int s = uniform_int_distribution<int>(1,n)(rng);
        int r = uniform_int_distribution<int>(1,m)(rng);
        if (used.count({s,r})) continue;
        used.insert({s,r}); pairs.push_back({s,r});
    }
    k = (int)pairs.size();

    // Побудова мережі: джерело(0) -> студенти(1..n) -> кімнати(n+1..n+m) -> сток(n+m+1)
    int S = 0, T = n + m + 1;
    int N = n + m + 1;
    graph.assign(N+1, {});
    for (int i = 1; i <= n; i++) addEdge(S, i, 1);
    for (int j = 1; j <= m; j++) addEdge(n+j, T, 1);
    for (auto& p : pairs) addEdge(p.first, n + p.second, 1);
    long long ans = maxFlow(N, S, T);

    stringstream in; in << n << " " << m << " " << k << "\n";
    for (auto& p : pairs) in << p.first << " " << p.second << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Розподіл по кімнатах (max flow, середня)"
#include "../../common/tester_main.inc"
