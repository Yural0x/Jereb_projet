// EZ_floyd_warshall_tester.cpp — "Відстані між корпусами"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;
static const long long INF = LLONG_MAX / 4;

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 3 : uniform_int_distribution<int>(1, 150)(rng);
    vector<vector<long long>> dist(n+1, vector<long long>(n+1, INF));
    for (int i = 1; i <= n; i++) dist[i][i] = 0;
    vector<array<long long,3>> edges;
    uniform_int_distribution<long long> wd(1, 1000000);
    set<pair<int,int>> used;
    for (int i = 1; i < n; i++) {
        for (int j = i+1; j <= n; j++) {
            if (uniform_int_distribution<int>(0, 4)(rng) == 0) { // ~20% щільність
                long long w = wd(rng);
                edges.push_back({i, j, w});
                if (w < dist[i][j]) { dist[i][j] = w; dist[j][i] = w; }
            }
        }
    }
    // Гарантуємо зв'язність випадковим деревом, якщо граф розпався
    vector<int> perm(n);
    for (int i = 0; i < n; i++) perm[i] = i+1;
    shuffle(perm.begin(), perm.end(), rng);
    for (int i = 1; i < n; i++) {
        int u = perm[i], v = perm[uniform_int_distribution<int>(0,i-1)(rng)];
        int a = min(u,v), b = max(u,v);
        if (!used.count({a,b})) {
            long long w = wd(rng);
            edges.push_back({a, b, w});
            used.insert({a,b});
            if (w < dist[a][b]) { dist[a][b] = w; dist[b][a] = w; }
        }
    }
    int m = (int)edges.size();

    for (int k = 1; k <= n; k++)
        for (int i = 1; i <= n; i++)
            for (int j = 1; j <= n; j++)
                if (dist[i][k] + dist[k][j] < dist[i][j]) dist[i][j] = dist[i][k] + dist[k][j];

    int q = idx == 0 ? 1 : uniform_int_distribution<int>(1, 30)(rng);
    stringstream in; in << n << " " << m << " " << q << "\n";
    for (auto& e : edges) in << e[0] << " " << e[1] << " " << e[2] << "\n";
    stringstream out;
    uniform_int_distribution<int> nd(1, n);
    for (int t = 0; t < q; t++) {
        int a = nd(rng), b = nd(rng);
        in << a << " " << b << "\n";
        long long d = dist[a][b];
        out << (d >= INF ? -1 : d) << "\n";
    }
    string e2 = out.str();
    if (!e2.empty() && e2.back() == '\n') e2.pop_back();
    TestCase tc; tc.input = in.str(); tc.expectedOutput = e2;
    return tc;
}
#define PROBLEM_NAME "Відстані між корпусами (Флойд-Уоршелл)"
#include "../../common/tester_main.inc"
