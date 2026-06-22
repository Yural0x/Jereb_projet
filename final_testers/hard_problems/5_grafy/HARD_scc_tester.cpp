// HARD_scc_tester.cpp — "Дві умови" (2-SAT)
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

// літерал x>0 означає змінну x (1-індекс), x<0 означає заперечення.
// вузол для змінної i: 2*(i-1) = true, 2*(i-1)+1 = false.
static int lit2node(int lit, bool wantTrue) {
    int var = abs(lit) - 1;
    bool positive = lit > 0;
    bool nodeForTrue = (positive == wantTrue);
    return 2*var + (nodeForTrue ? 0 : 1);
}

static bool solve2SAT(int n, vector<pair<int,int>>& clauses) {
    int N = 2*n;
    vector<vector<int>> adj(N);
    auto addImpl = [&](int from, int to) { adj[from].push_back(to); };
    for (auto& [a, b] : clauses) {
        // (a OR b): not a => b, not b => a
        int notA = lit2node(a, false), Bn = lit2node(b, true);
        int notB = lit2node(b, false), An = lit2node(a, true);
        addImpl(notA, Bn);
        addImpl(notB, An);
    }
    vector<int> disc(N,-1), low(N,-1), comp(N,-1);
    vector<bool> onStack(N,false);
    vector<int> stk;
    int timer=0, compCount=0;
    function<void(int)> dfs = [&](int u) {
        disc[u]=low[u]=timer++;
        stk.push_back(u); onStack[u]=true;
        for (int v : adj[u]) {
            if (disc[v]==-1) { dfs(v); low[u]=min(low[u],low[v]); }
            else if (onStack[v]) low[u]=min(low[u],disc[v]);
        }
        if (low[u]==disc[u]) {
            while (true) { int v=stk.back(); stk.pop_back(); onStack[v]=false; comp[v]=compCount; if (v==u) break; }
            compCount++;
        }
    };
    for (int i = 0; i < N; i++) if (disc[i]==-1) dfs(i);
    for (int var = 0; var < n; var++) {
        if (comp[2*var] == comp[2*var+1]) return false; // x і not x в одній SCC
    }
    return true;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 2 : uniform_int_distribution<int>(1, 1000)(rng);
    vector<pair<int,int>> clauses;
    uniform_int_distribution<int> vd(1, max(n,1));

    if (idx == 0) {
        clauses = {{1,2},{-1,2}}; // SAT приклад
    } else if (idx == 1 && n >= 1) {
        clauses = {{1,1},{-1,-1}}; // UNSAT приклад: x=true і x=false одночасно
        n = 1;
    } else {
        int m = uniform_int_distribution<int>(1, min(2000, n*2))(rng);
        for (int i = 0; i < m; i++) {
            int v1 = vd(rng) * (uniform_int_distribution<int>(0,1)(rng) ? 1 : -1);
            int v2 = vd(rng) * (uniform_int_distribution<int>(0,1)(rng) ? 1 : -1);
            clauses.push_back({v1, v2});
        }
    }
    int m = (int)clauses.size();

    bool sat = solve2SAT(n, clauses);
    stringstream in; in << n << " " << m << "\n";
    for (auto& c : clauses) in << c.first << " " << c.second << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = sat ? "YES" : "NO";
    return tc;
}
#define PROBLEM_NAME "Дві умови (2-SAT, складна)"
#include "../../common/tester_main.inc"
