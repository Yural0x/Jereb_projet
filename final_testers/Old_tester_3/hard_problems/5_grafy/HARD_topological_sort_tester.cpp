// HARD_topological_sort_tester.cpp — "Найдовший ланцюг"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static long long longestPathDAG(int n, vector<vector<pair<int,long long>>>& adj, vector<int>& indeg) {
    vector<long long> dp(n+1, 0);
    queue<int> q;
    vector<int> ind = indeg;
    for (int i = 1; i <= n; i++) if (ind[i] == 0) q.push(i);
    vector<int> order;
    while (!q.empty()) {
        int u = q.front(); q.pop(); order.push_back(u);
        for (auto& [v,w] : adj[u]) { if (--ind[v] == 0) q.push(v); }
    }
    long long best = 0;
    for (int u : order) {
        for (auto& [v,w] : adj[u]) {
            if (dp[u] + w > dp[v]) dp[v] = dp[u] + w;
        }
        best = max(best, dp[u]);
    }
    return best;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 3 : uniform_int_distribution<int>(1, 2000)(rng);
    vector<int> labels(n);
    for (int i = 0; i < n; i++) labels[i] = i+1;
    shuffle(labels.begin(), labels.end(), rng);
    vector<array<long long,3>> edges;
    set<pair<int,int>> used;
    uniform_int_distribution<long long> wd(1, 1000000000LL);
    int m = uniform_int_distribution<int>(0, min(2000,n))(rng);
    int attempts = 0;
    while ((int)edges.size() < m && n >= 2 && attempts < m*5+50) {
        attempts++;
        int i = uniform_int_distribution<int>(0,n-2)(rng);
        int j = uniform_int_distribution<int>(i+1,n-1)(rng);
        int u = labels[i], v = labels[j];
        if (used.count({u,v})) continue;
        used.insert({u,v});
        edges.push_back({u,v,wd(rng)});
    }
    m = (int)edges.size();

    vector<vector<pair<int,long long>>> adj(n+1);
    vector<int> indeg(n+1,0);
    for (auto& e : edges) { adj[e[0]].push_back({(int)e[1], e[2]}); indeg[e[1]]++; }
    long long ans = longestPathDAG(n, adj, indeg);

    stringstream in; in << n << " " << m << "\n";
    for (auto& e : edges) in << e[0] << " " << e[1] << " " << e[2] << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Найдовший ланцюг (DAG, складна)"
#include "../../common/tester_main.inc"
