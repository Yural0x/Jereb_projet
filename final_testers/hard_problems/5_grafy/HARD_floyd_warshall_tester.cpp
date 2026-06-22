// HARD_floyd_warshall_tester.cpp — "Найлегший цикл"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;
static const long long INF = LLONG_MAX / 4;

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 3 : uniform_int_distribution<int>(1, 300)(rng);
    vector<vector<long long>> dist(n+1, vector<long long>(n+1, INF));
    vector<array<long long,3>> edges;
    uniform_int_distribution<long long> wd(1, 1000000);

    // Завжди будуємо хоча б один явний цикл, щоб переконатись що відповідь не -1 у half випадків
    bool guaranteeCycle = uniform_int_distribution<int>(0,1)(rng);
    if (idx == 0) guaranteeCycle = true;

    if (guaranteeCycle && n >= 2) {
        // простий цикл через усі вершини 1->2->...->n->1
        vector<int> perm(n);
        for (int i=0;i<n;i++) perm[i]=i+1;
        shuffle(perm.begin(), perm.end(), rng);
        for (int i = 0; i < n; i++) {
            int u = perm[i], v = perm[(i+1)%n];
            long long w = wd(rng);
            edges.push_back({u,v,w});
            if (w < dist[u][v]) dist[u][v] = w;
        }
    }
    int extra = uniform_int_distribution<int>(0, min(2000, n*2))(rng);
    set<pair<int,int>> used;
    for (auto& e : edges) used.insert({(int)e[0],(int)e[1]});
    for (int i = 0; i < extra; i++) {
        int u = uniform_int_distribution<int>(1, max(n,1))(rng);
        int v = uniform_int_distribution<int>(1, max(n,1))(rng);
        if (u==v) continue;
        if (used.count({u,v})) continue;
        used.insert({u,v});
        long long w = wd(rng);
        edges.push_back({u,v,w});
        if (w < dist[u][v]) dist[u][v] = w;
    }
    int m = (int)edges.size();

    for (int k=1;k<=n;k++)for(int i=1;i<=n;i++)for(int j=1;j<=n;j++)
        if (dist[i][k]<INF && dist[k][j]<INF && dist[i][k]+dist[k][j] < dist[i][j]) dist[i][j]=dist[i][k]+dist[k][j];

    long long best = INF;
    for (int i = 1; i <= n; i++) best = min(best, dist[i][i]);

    stringstream in; in << n << " " << m << "\n";
    for (auto& e : edges) in << e[0] << " " << e[1] << " " << e[2] << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = (best>=INF ? "-1" : to_string(best));
    return tc;
}
#define PROBLEM_NAME "Найлегший цикл (Флойд-Уоршелл, складна)"
#include "../../common/tester_main.inc"
