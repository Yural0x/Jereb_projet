// MED_floyd_warshall_tester.cpp — "Діаметр кампусу"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;
static const long long INF = LLONG_MAX / 4;

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 3 : uniform_int_distribution<int>(2, 150)(rng);
    vector<vector<long long>> dist(n+1, vector<long long>(n+1, INF));
    for (int i = 1; i <= n; i++) dist[i][i] = 0;
    vector<array<long long,3>> edges;
    uniform_int_distribution<long long> wd(1, 1000000);

    // гарантуємо зв'язність деревом
    vector<int> perm(n);
    for (int i = 0; i < n; i++) perm[i] = i+1;
    shuffle(perm.begin(), perm.end(), rng);
    set<pair<int,int>> used;
    for (int i = 1; i < n; i++) {
        int u = perm[i], v = perm[uniform_int_distribution<int>(0,i-1)(rng)];
        int a = min(u,v), b = max(u,v);
        long long w = wd(rng);
        edges.push_back({a,b,w}); used.insert({a,b});
        if (w < dist[a][b]) { dist[a][b] = w; dist[b][a] = w; }
    }
    int extra = uniform_int_distribution<int>(0, min(2000, n*n/4))(rng);
    for (int i = 0; i < extra; i++) {
        int a = uniform_int_distribution<int>(1,n)(rng);
        int b = uniform_int_distribution<int>(1,n)(rng);
        if (a==b) continue;
        int x = min(a,b), y = max(a,b);
        if (used.count({x,y})) continue;
        used.insert({x,y});
        long long w = wd(rng);
        edges.push_back({x,y,w});
        if (w < dist[x][y]) { dist[x][y] = w; dist[y][x] = w; }
    }
    int m = (int)edges.size();

    for (int k=1;k<=n;k++)for(int i=1;i<=n;i++)for(int j=1;j<=n;j++)
        if (dist[i][k]+dist[k][j]<dist[i][j]) dist[i][j]=dist[i][k]+dist[k][j];

    long long diameter = 0;
    for (int i=1;i<=n;i++)for(int j=1;j<=n;j++) if (dist[i][j]<INF) diameter = max(diameter, dist[i][j]);

    stringstream in; in << n << " " << m << "\n";
    for (auto& e : edges) in << e[0] << " " << e[1] << " " << e[2] << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(diameter);
    return tc;
}
#define PROBLEM_NAME "Діаметр кампусу (Флойд-Уоршелл, середня)"
#include "../../common/tester_main_multilang.inc"
