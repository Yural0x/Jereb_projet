// MED_bfs_tester.cpp — "Лабіринт гуртожитку"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static int bfsGrid(int n, int m, vector<string>& g) {
    vector<vector<int>> dist(n, vector<int>(m, -1));
    queue<pair<int,int>> q;
    dist[0][0] = 0; q.push({0,0});
    int dx[4] = {0,0,1,-1}, dy[4] = {1,-1,0,0};
    while (!q.empty()) {
        auto [x,y] = q.front(); q.pop();
        for (int d = 0; d < 4; d++) {
            int nx = x+dx[d], ny = y+dy[d];
            if (nx<0||nx>=n||ny<0||ny>=m) continue;
            if (g[nx][ny] == '#') continue;
            if (dist[nx][ny] != -1) continue;
            dist[nx][ny] = dist[x][y] + 1;
            q.push({nx,ny});
        }
    }
    return dist[n-1][m-1];
}

static TestCase genTest(mt19937& rng, int idx) {
    int n, m;
    vector<string> g;
    if (idx == 0) { n=3; m=3; g={"...",".#.","..."}; }
    else {
        n = uniform_int_distribution<int>(1, 200)(rng);
        m = uniform_int_distribution<int>(1, 200)(rng);
        g.assign(n, string(m, '.'));
        int wallProb = uniform_int_distribution<int>(0, 30)(rng); // % стін
        for (int i = 0; i < n; i++)
            for (int j = 0; j < m; j++)
                if (uniform_int_distribution<int>(0,99)(rng) < wallProb) g[i][j] = '#';
        g[0][0] = '.'; g[n-1][m-1] = '.';
    }

    int d = bfsGrid(n, m, g);
    stringstream in; in << n << " " << m << "\n";
    for (auto& row : g) in << row << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(d);
    return tc;
}
#define PROBLEM_NAME "Лабіринт гуртожитку (BFS на сітці, середня)"
#include "../../common/tester_main.inc"
