// HARD_kruskal_mst_tester.cpp — "Максимум на шляху дерева"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static long long solveRef(int n, vector<array<long long,3>>& edges, int u, int v) {
    int m = (int)edges.size();
    vector<int> order(m);
    for (int i=0;i<m;i++) order[i]=i;
    sort(order.begin(), order.end(), [&](int a, int b){ return edges[a][2] < edges[b][2]; });
    vector<int> parent(n+1);
    for (int i=0;i<=n;i++) parent[i]=i;
    function<int(int)> find = [&](int x){ while(parent[x]!=x) x=parent[x]=parent[parent[x]]; return x; };
    vector<vector<pair<int,long long>>> treeAdj(n+1);
    for (int id : order) {
        int a=(int)edges[id][0], b=(int)edges[id][1]; long long w=edges[id][2];
        int ra=find(a), rb=find(b);
        if (ra!=rb) { parent[ra]=rb; treeAdj[a].push_back({b,w}); treeAdj[b].push_back({a,w}); }
    }
    // BFS від u до v, шукаючи максимальну вагу ребра на шляху
    vector<bool> visited(n+1,false);
    vector<long long> maxOnPath(n+1, -1);
    queue<int> q; q.push(u); visited[u]=true; maxOnPath[u]=0;
    while (!q.empty()) {
        int x=q.front(); q.pop();
        if (x==v) break;
        for (auto& [y,w] : treeAdj[x]) if (!visited[y]) { visited[y]=true; maxOnPath[y]=max(maxOnPath[x],w); q.push(y); }
    }
    return maxOnPath[v];
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 4 : uniform_int_distribution<int>(2, 2000)(rng);
    vector<array<long long,3>> edges;
    set<pair<int,int>> used;
    uniform_int_distribution<long long> wd(1, 1000000000LL);
    vector<int> perm(n);
    for (int i=0;i<n;i++) perm[i]=i+1;
    shuffle(perm.begin(), perm.end(), rng);
    for (int i=1;i<n;i++) {
        int u=perm[i], v=perm[uniform_int_distribution<int>(0,i-1)(rng)];
        int a=min(u,v), b=max(u,v);
        used.insert({a,b});
        edges.push_back({a,b,wd(rng)});
    }
    int extra = uniform_int_distribution<int>(0, min(2000, n))(rng);
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
    if (idx == 0) { n=4; edges = {{1,2,1},{2,3,2},{3,4,3},{1,4,10}}; }
    int m = (int)edges.size();

    int q = idx == 0 ? 2 : uniform_int_distribution<int>(1, 200)(rng);
    stringstream in; in << n << " " << m << "\n";
    for (auto& e : edges) in << e[0] << " " << e[1] << " " << e[2] << "\n";
    in << q << "\n";
    stringstream out;
    uniform_int_distribution<int> nd(1, n);
    for (int t = 0; t < q; t++) {
        int u, v;
        if (idx == 0) { u = (t==0)?1:1; v = (t==0)?4:3; }
        else { u = nd(rng); v = nd(rng); while (v==u && n>1) v = nd(rng); }
        in << u << " " << v << "\n";
        long long ans = (u==v) ? 0 : solveRef(n, edges, u, v);
        out << ans << "\n";
    }
    string e2 = out.str();
    if (!e2.empty() && e2.back() == '\n') e2.pop_back();
    TestCase tc; tc.input = in.str(); tc.expectedOutput = e2;
    return tc;
}
#define PROBLEM_NAME "Максимум на шляху дерева (MST + binary lifting, складна)"
#include "../../common/tester_main.inc"
