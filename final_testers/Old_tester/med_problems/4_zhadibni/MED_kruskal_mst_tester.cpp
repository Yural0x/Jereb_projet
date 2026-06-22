// MED_kruskal_mst_tester.cpp — "Чи з'єднати все"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static string solveRef(int n, vector<array<long long,3>>& edges) {
    sort(edges.begin(), edges.end(), [](const array<long long,3>& a, const array<long long,3>& b){
        return a[2] < b[2];
    });
    vector<int> parent(n+1);
    for (int i = 0; i <= n; i++) parent[i] = i;
    function<int(int)> find = [&](int x){ while (parent[x]!=x){ x = parent[x]=parent[parent[x]]; } return x; };
    long long total = 0;
    int components = n;
    for (auto& e : edges) {
        int u = (int)e[0], v = (int)e[1]; long long w = e[2];
        int ru = find(u), rv = find(v);
        if (ru != rv) { parent[ru] = rv; total += w; components--; }
    }
    if (components > 1) return "-1";
    return to_string(total);
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 4 : uniform_int_distribution<int>(2, 500)(rng);
    vector<array<long long,3>> edges;
    uniform_int_distribution<long long> wd(1, 1000000000LL);

    bool connected = uniform_int_distribution<int>(0,1)(rng);
    if (idx == 0) connected = false;

    if (connected) {
        vector<int> perm(n);
        for (int i = 0; i < n; i++) perm[i] = i + 1;
        shuffle(perm.begin(), perm.end(), rng);
        for (int i = 1; i < n; i++) {
            int u = perm[i], v = perm[uniform_int_distribution<int>(0, i-1)(rng)];
            edges.push_back({u, v, wd(rng)});
        }
    } else if (idx == 0) {
        edges.push_back({1,2,1}); edges.push_back({3,4,2});
    } else {
        // довільні, можливо незв'язні ребра
        int extraEdges = uniform_int_distribution<int>(0, n)(rng);
        for (int i = 0; i < extraEdges; i++) {
            int u = uniform_int_distribution<int>(1, n)(rng);
            int v = uniform_int_distribution<int>(1, n)(rng);
            if (u == v) continue;
            edges.push_back({u, v, wd(rng)});
        }
    }
    int extra = uniform_int_distribution<int>(0, min(300, n))(rng);
    for (int i = 0; i < extra; i++) {
        int u = uniform_int_distribution<int>(1, n)(rng);
        int v = uniform_int_distribution<int>(1, n)(rng);
        if (u == v) continue;
        edges.push_back({u, v, wd(rng)});
    }
    int m = (int)edges.size();

    string ans = solveRef(n, edges);
    stringstream in; in << n << " " << m << "\n";
    for (auto& e : edges) in << e[0] << " " << e[1] << " " << e[2] << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = ans;
    return tc;
}
#define PROBLEM_NAME "Чи з'єднати все (MST, середня)"
#include "../../common/tester_main.inc"
