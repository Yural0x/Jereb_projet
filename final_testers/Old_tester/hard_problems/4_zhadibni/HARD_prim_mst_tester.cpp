// HARD_prim_mst_tester.cpp — "Друге за вагою дерево"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 3000;

// Будуємо MST (Краскал), потім для кожного ребра НЕ з MST пробуємо додати його й
// видалити найважче ребро в циклі, що утворився (через DFS у MST-дереві між його кінцями).
// Складність O(m * n) - прийнятно для тестових n,m <= 600.
static long long secondMST(int n, vector<array<long long,3>>& edges) {
    int m = (int)edges.size();
    vector<int> order(m);
    for (int i=0;i<m;i++) order[i]=i;
    sort(order.begin(), order.end(), [&](int a, int b){ return edges[a][2] < edges[b][2]; });

    vector<int> parent(n+1);
    for (int i=0;i<=n;i++) parent[i]=i;
    function<int(int)> find = [&](int x){ while(parent[x]!=x) x=parent[x]=parent[parent[x]]; return x; };
    vector<bool> inMST(m, false);
    long long mstWeight = 0;
    vector<vector<pair<int,long long>>> treeAdj(n+1);
    for (int id : order) {
        int u=(int)edges[id][0], v=(int)edges[id][1]; long long w=edges[id][2];
        int ru=find(u), rv=find(v);
        if (ru!=rv) {
            parent[ru]=rv; inMST[id]=true; mstWeight += w;
            treeAdj[u].push_back({v,w}); treeAdj[v].push_back({u,w});
        }
    }

    long long best = LLONG_MAX;
    // для кожного небазового ребра знаходимо максимальне ребро на шляху між його кінцями в MST
    for (int id = 0; id < m; id++) {
        if (inMST[id]) continue;
        int u=(int)edges[id][0], v=(int)edges[id][1]; long long w=edges[id][2];
        // BFS/DFS по дереву від u до v, шукаючи максимальну вагу на шляху
        vector<int> parentTree(n+1,-1);
        vector<long long> maxEdgeOnPath(n+1, -1);
        vector<bool> visited(n+1,false);
        queue<int> q; q.push(u); visited[u]=true; maxEdgeOnPath[u]=0;
        while (!q.empty()) {
            int x=q.front(); q.pop();
            if (x==v) break;
            for (auto& [y,ew] : treeAdj[x]) {
                if (!visited[y]) {
                    visited[y]=true;
                    maxEdgeOnPath[y] = max(maxEdgeOnPath[x], ew);
                    q.push(y);
                }
            }
        }
        long long candidate = mstWeight - maxEdgeOnPath[v] + w;
        best = min(best, candidate);
    }
    return best;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 3 : uniform_int_distribution<int>(3, 400)(rng);
    vector<array<long long,3>> edges;
    set<pair<int,int>> used;
    uniform_int_distribution<long long> wd(1, 1000000000LL);
    // зв'язне дерево + додаткові ребра (гарантуємо хоча б одне небазове ребро)
    vector<int> perm(n);
    for (int i=0;i<n;i++) perm[i]=i+1;
    shuffle(perm.begin(), perm.end(), rng);
    for (int i=1;i<n;i++) {
        int u=perm[i], v=perm[uniform_int_distribution<int>(0,i-1)(rng)];
        int a=min(u,v), b=max(u,v);
        used.insert({a,b});
        edges.push_back({a,b,wd(rng)});
    }
    int extra = max(1, uniform_int_distribution<int>(1, min(500, n))(rng));
    int attempts=0;
    while ((int)edges.size() < n-1+extra && attempts < extra*5+50) {
        attempts++;
        int u=uniform_int_distribution<int>(1,n)(rng), v=uniform_int_distribution<int>(1,n)(rng);
        if (u==v) continue;
        int a=min(u,v), b=max(u,v);
        if (used.count({a,b})) continue;
        used.insert({a,b});
        edges.push_back({a,b,wd(rng)});
    }
    if (idx == 0) { n=3; edges = {{1,2,1},{2,3,2},{1,3,3}}; }
    int m = (int)edges.size();

    long long ans = secondMST(n, edges);
    stringstream in; in << n << " " << m << "\n";
    for (auto& e : edges) in << e[0] << " " << e[1] << " " << e[2] << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Друге за вагою дерево (MST, складна)"
#include "../../common/tester_main.inc"
