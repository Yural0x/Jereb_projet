// MED_topological_sort_tester.cpp — "Найменший порядок"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static vector<int> solveRef(int n, vector<pair<int,int>>& edges) {
    vector<vector<int>> adj(n+1);
    vector<int> indeg(n+1, 0);
    for (auto& e : edges) { adj[e.first].push_back(e.second); indeg[e.second]++; }
    priority_queue<int, vector<int>, greater<int>> pq;
    for (int i = 1; i <= n; i++) if (indeg[i] == 0) pq.push(i);
    vector<int> order;
    while (!pq.empty()) {
        int u = pq.top(); pq.pop();
        order.push_back(u);
        for (int v : adj[u]) { indeg[v]--; if (indeg[v] == 0) pq.push(v); }
    }
    return order;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 3 : uniform_int_distribution<int>(1, 2000)(rng);
    // Випадковий DAG: ребра лише від меншого індексу до більшого (за випадковою нумерацією),
    // щоб гарантовано не було циклів.
    vector<int> labels(n);
    for (int i = 0; i < n; i++) labels[i] = i + 1;
    shuffle(labels.begin(), labels.end(), rng);
    // labels[i] - довільна нумерація; для ациклічності генеруємо ребра за індексом у векторі labels
    vector<pair<int,int>> edges;
    set<pair<int,int>> used;
    int m = uniform_int_distribution<int>(0, min(2000, n))(rng);
    int attempts = 0;
    while ((int)edges.size() < m && n >= 2 && attempts < m*5+50) {
        attempts++;
        int i = uniform_int_distribution<int>(0, n-2)(rng);
        int j = uniform_int_distribution<int>(i+1, n-1)(rng);
        int u = labels[i], v = labels[j];
        if (used.count({u,v})) continue;
        used.insert({u,v});
        edges.push_back({u,v});
    }
    m = (int)edges.size();

    vector<int> order = solveRef(n, edges);
    stringstream in; in << n << " " << m << "\n";
    for (auto& e : edges) in << e.first << " " << e.second << "\n";
    stringstream out;
    for (int i = 0; i < n; i++) out << order[i] << (i+1<n?' ':' ');
    string e = out.str();
    while (!e.empty() && e.back() == ' ') e.pop_back();
    TestCase tc; tc.input = in.str(); tc.expectedOutput = e;
    return tc;
}
#define PROBLEM_NAME "Найменший порядок (топологічне сортування, середня)"
#include "../../common/tester_main_multilang.inc"
