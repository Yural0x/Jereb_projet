// MED_bellman_ford_tester.cpp — "Пошук від'ємного циклу"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static bool hasNegativeCycle(int n, vector<array<long long,3>>& edges) {
    vector<long long> dist(n+1, 0); // стартуємо з усіх 0, щоб виявити цикл будь-де
    for (int iter = 0; iter < n; iter++) {
        bool changed = false;
        for (auto& e : edges) {
            int u=(int)e[0], v=(int)e[1]; long long w=e[2];
            if (dist[u] + w < dist[v]) { dist[v] = dist[u] + w; changed = true; }
        }
        if (!changed) return false;
    }
    return true;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 2 : uniform_int_distribution<int>(1, 800)(rng);
    bool wantNeg = uniform_int_distribution<int>(0,1)(rng);
    if (idx == 0) wantNeg = true;
    if (idx == 1) wantNeg = false;
    vector<array<long long,3>> edges;
    uniform_int_distribution<long long> wd(-1000000, 1000000);

    if (wantNeg && n >= 2) {
        // гарантований від'ємний цикл довжини 2: u->v вагою w1, v->u вагою w2, w1+w2<0
        int u = uniform_int_distribution<int>(1,n)(rng);
        int v = uniform_int_distribution<int>(1,n)(rng);
        while (v == u) v = uniform_int_distribution<int>(1,n)(rng);
        edges.push_back({u, v, -5});
        edges.push_back({v, u, -5});
    }
    // додаємо випадкові ациклічні ребра (за зростанням індексу) щоб не зіпсувати/не прибрати цикл
    int extra = uniform_int_distribution<int>(0, min(2000, n))(rng);
    for (int i = 0; i < extra && n >= 2; i++) {
        int a = uniform_int_distribution<int>(1, n-1)(rng);
        int b = uniform_int_distribution<int>(a+1, n)(rng);
        edges.push_back({a, b, wd(rng)});
    }
    int m = (int)edges.size();

    bool ans = hasNegativeCycle(n, edges);
    stringstream in; in << n << " " << m << "\n";
    for (auto& e : edges) in << e[0] << " " << e[1] << " " << e[2] << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = ans ? "YES" : "NO";
    return tc;
}
#define PROBLEM_NAME "Пошук від'ємного циклу (середня)"
#include "../../common/tester_main_multilang.inc"
