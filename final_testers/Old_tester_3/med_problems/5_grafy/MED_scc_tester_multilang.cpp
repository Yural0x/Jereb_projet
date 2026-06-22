// MED_scc_tester.cpp — "Найбільша спільнота"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static int solveRef(int n, vector<vector<int>>& adj) {
    vector<int> disc(n+1,-1), low(n+1,-1);
    vector<bool> onStack(n+1,false);
    vector<int> stk;
    int timer=0;
    int best = 0;
    function<void(int)> dfs = [&](int u) {
        disc[u]=low[u]=timer++;
        stk.push_back(u); onStack[u]=true;
        for (int v : adj[u]) {
            if (disc[v]==-1) { dfs(v); low[u]=min(low[u],low[v]); }
            else if (onStack[v]) low[u]=min(low[u],disc[v]);
        }
        if (low[u]==disc[u]) {
            int size=0;
            while (true) { int v=stk.back(); stk.pop_back(); onStack[v]=false; size++; if (v==u) break; }
            best = max(best, size);
        }
    };
    for (int i=1;i<=n;i++) if (disc[i]==-1) dfs(i);
    return best;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 5 : uniform_int_distribution<int>(1, 1500)(rng);
    vector<pair<int,int>> edges;
    set<pair<int,int>> used;

    if (idx == 0) {
        edges = {{1,2},{2,1},{2,3},{3,4},{4,3}};
        used = {{1,2},{2,1},{2,3},{3,4},{4,3}};
    } else {
        int m = n<=1?0:uniform_int_distribution<int>(0, min(3000, n*3))(rng);
        int attempts=0;
        uniform_int_distribution<int> nd(1, max(n,1));
        while ((int)edges.size()<m && attempts<m*5+50) {
            attempts++;
            int u = nd(rng), v = nd(rng);
            if (u==v) continue;
            if (used.count({u,v})) continue;
            used.insert({u,v}); edges.push_back({u,v});
        }
    }
    int m = (int)edges.size();

    vector<vector<int>> adj(n+1);
    for (auto& e : edges) adj[e.first].push_back(e.second);
    int ans = solveRef(n, adj);

    stringstream in; in << n << " " << m << "\n";
    for (auto& e : edges) in << e.first << " " << e.second << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Найбільша спільнота (SCC, середня)"
#include "../../common/tester_main_multilang.inc"
