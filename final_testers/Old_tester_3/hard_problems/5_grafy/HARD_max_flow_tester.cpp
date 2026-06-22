// HARD_max_flow_tester.cpp — "Розрив мережі"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

struct Edge { int to; long long cap; int rev; };
static vector<vector<Edge>> graph;
static long long bfsAugment(int N, int s, int t, vector<int>& prevV, vector<int>& prevE) {
    vector<bool> visited(N+1, false);
    queue<int> q; q.push(s); visited[s]=true;
    prevV.assign(N+1,-1); prevE.assign(N+1,-1);
    while (!q.empty()) {
        int u = q.front(); q.pop();
        if (u==t) break;
        for (int i=0;i<(int)graph[u].size();i++) {
            auto& e = graph[u][i];
            if (!visited[e.to] && e.cap>0) { visited[e.to]=true; prevV[e.to]=u; prevE[e.to]=i; q.push(e.to); }
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
    long long total=0; vector<int> pv,pe;
    while (true) { long long f=bfsAugment(N,s,t,pv,pe); if (f==0) break; total+=f; }
    return total;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 4 : uniform_int_distribution<int>(2, 300)(rng);
    vector<pair<int,int>> edges;
    set<pair<int,int>> used;
    int maxM = min(2000, n*(n-1)/2);
    int m = idx == 0 ? 4 : uniform_int_distribution<int>(0, maxM)(rng);
    int attempts = 0;
    while ((int)edges.size() < m && attempts < m*5+50) {
        attempts++;
        int u = uniform_int_distribution<int>(1,n)(rng);
        int v = uniform_int_distribution<int>(1,n)(rng);
        if (u==v) continue;
        int a=min(u,v), b=max(u,v);
        if (used.count({a,b})) continue;
        used.insert({a,b}); edges.push_back({a,b});
    }
    m = (int)edges.size();

    // Кожен двосторонній канал пропускної здатності 1: для коректного min-cut
    // моделюємо як одне неорієнтоване ребро ємністю 1 (через дві пари зустрічних дуг).
    graph.assign(n+1, {});
    for (auto& e : edges) {
        // ребро ємністю 1 в обидва боки: пара дуг u->v(1) і v->u(1), кожна зі своїм зворотним
        graph[e.first].push_back({e.second, 1, (int)graph[e.second].size()});
        graph[e.second].push_back({e.first, 0, (int)graph[e.first].size()-1});
        graph[e.second].push_back({e.first, 1, (int)graph[e.first].size()});
        graph[e.first].push_back({e.second, 0, (int)graph[e.second].size()-1});
    }
    long long ans = maxFlow(n, 1, n);

    stringstream in; in << n << " " << m << "\n";
    for (auto& e : edges) in << e.first << " " << e.second << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Розрив мережі (min cut, складна)"
#include "../../common/tester_main.inc"
