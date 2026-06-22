// EZ_bellman_ford_tester.cpp — "Дороги зі знижками"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static long long bellmanFord(int n, vector<array<long long,3>>& edges, int src, int dst) {
    vector<long long> dist(n+1, LLONG_MAX);
    dist[src] = 0;
    for (int iter = 0; iter < n - 1; iter++) {
        bool changed = false;
        for (auto& e : edges) {
            int u = (int)e[0], v = (int)e[1]; long long w = e[2];
            if (dist[u] != LLONG_MAX && dist[u] + w < dist[v]) { dist[v] = dist[u] + w; changed = true; }
        }
        if (!changed) break;
    }
    return dist[dst] == LLONG_MAX ? -1 : dist[dst];
}

// Будуємо ациклічний орієнтований граф (за порядком вершин 1..n), щоб гарантовано
// уникнути від'ємних циклів, дозволяючи довільні (зокрема від'ємні) ваги.
static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 3 : uniform_int_distribution<int>(1, 1000)(rng);
    vector<array<long long,3>> edges;
    uniform_int_distribution<long long> wd(-1000000, 1000000);
    set<pair<int,int>> used;
    if (n > 1) {
        for (int i = 1; i < n; i++) {
            // ребро з меншого номера в більший - ациклічно за побудовою
            int u = i, v = i + 1;
            edges.push_back({u, v, wd(rng)});
            used.insert({u,v});
        }
    }
    int extra = uniform_int_distribution<int>(0, min(2000, n))(rng);
    for (int i = 0; i < extra && n >= 2; i++) {
        int u = uniform_int_distribution<int>(1, n-1)(rng);
        int v = uniform_int_distribution<int>(u+1, n)(rng);
        if (used.count({u,v})) continue;
        used.insert({u,v});
        edges.push_back({u, v, wd(rng)});
    }
    int m = (int)edges.size();

    long long ans = bellmanFord(n, edges, 1, n);
    stringstream in; in << n << " " << m << "\n";
    for (auto& e : edges) in << e[0] << " " << e[1] << " " << e[2] << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Дороги зі знижками (Беллман-Форд)"
#include "../../common/tester_main.inc"
