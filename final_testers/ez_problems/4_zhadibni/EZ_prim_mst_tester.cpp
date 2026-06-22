// EZ_prim_mst_tester.cpp — "Мережа Прімом"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static long long solveRef(int n, int m, vector<array<long long,3>>& edges) {
    // алгоритм Крускала для еталона (простіше реалізувати правильно)
    sort(edges.begin(), edges.end(), [](const array<long long,3>& a, const array<long long,3>& b){
        return a[2] < b[2];
    });
    vector<int> parent(n+1);
    for (int i = 0; i <= n; i++) parent[i] = i;
    function<int(int)> find = [&](int x){ while (parent[x]!=x){ x = parent[x]=parent[parent[x]]; } return x; };
    long long total = 0;
    int used = 0;
    for (auto& e : edges) {
        int u = (int)e[0], v = (int)e[1]; long long w = e[2];
        int ru = find(u), rv = find(v);
        if (ru != rv) { parent[ru] = rv; total += w; used++; }
    }
    return total;
}

// Генерує зв'язний граф: спочатку дерево (гарантує зв'язність), потім додає випадкові ребра.
static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 3 : uniform_int_distribution<int>(2, 500)(rng);
    vector<array<long long,3>> edges;
    uniform_int_distribution<long long> wd(1, 1000000000LL);
    vector<int> perm(n);
    for (int i = 0; i < n; i++) perm[i] = i + 1;
    shuffle(perm.begin(), perm.end(), rng);
    for (int i = 1; i < n; i++) {
        int u = perm[i], v = perm[uniform_int_distribution<int>(0, i-1)(rng)];
        edges.push_back({u, v, wd(rng)});
    }
    int extra = uniform_int_distribution<int>(0, min(500, n))(rng);
    for (int i = 0; i < extra; i++) {
        int u = uniform_int_distribution<int>(1, n)(rng);
        int v = uniform_int_distribution<int>(1, n)(rng);
        if (u == v) continue;
        edges.push_back({u, v, wd(rng)});
    }
    int m = (int)edges.size();

    long long ans = solveRef(n, m, edges);
    stringstream in; in << n << " " << m << "\n";
    for (auto& e : edges) in << e[0] << " " << e[1] << " " << e[2] << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Мережа Прімом (MST)"
#include "../../common/tester_main.inc"
