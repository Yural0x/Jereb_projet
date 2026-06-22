// HARD_dsu_tester.cpp — "Видалення кабелів"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static vector<int> solveRef(int n, vector<pair<int,int>>& edges, vector<int>& delOrder) {
    // delOrder[k] - індекс ребра (0-based в edges), що видаляється на кроці k.
    int q = (int)delOrder.size();
    vector<bool> removed(edges.size(), false);
    for (int idx : delOrder) removed[idx] = true;

    vector<int> parent(n+1);
    for (int i=0;i<=n;i++) parent[i]=i;
    function<int(int)> find = [&](int x){ while(parent[x]!=x) x=parent[x]=parent[parent[x]]; return x; };
    int comps = n;
    // будуємо DSU лише з ребер, що НІКОЛИ не видаляються
    for (size_t i = 0; i < edges.size(); i++) {
        if (removed[i]) continue;
        int ra=find(edges[i].first), rb=find(edges[i].second);
        if (ra!=rb) { parent[ra]=rb; comps--; }
    }
    vector<int> answers(q);
    // ідемо у зворотному порядку видалень, додаючи їх назад (об'єднання)
    for (int k = q-1; k >= 0; k--) {
        int idx = delOrder[k];
        int ra=find(edges[idx].first), rb=find(edges[idx].second);
        if (ra!=rb) { parent[ra]=rb; comps--; }
        answers[k] = comps;
    }
    return answers;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 3 : uniform_int_distribution<int>(2, 1500)(rng);
    int m = idx == 0 ? 2 : uniform_int_distribution<int>(1, min(2000, n*(n-1)/2))(rng);
    vector<pair<int,int>> edges;
    set<pair<int,int>> used;
    if (idx == 0) edges = {{1,2},{2,3}};
    else {
        int attempts=0;
        while ((int)edges.size()<m && attempts<m*5+50) {
            attempts++;
            int a=uniform_int_distribution<int>(1,n)(rng), b=uniform_int_distribution<int>(1,n)(rng);
            if (a==b) continue;
            int x=min(a,b), y=max(a,b);
            if (used.count({x,y})) continue;
            used.insert({x,y}); edges.push_back({x,y});
        }
    }
    m = (int)edges.size();
    int q = idx == 0 ? 2 : m; // видаляємо всі ребра у випадковому порядку (відповідає умові "кожен присутній і видаляється раз")
    vector<int> order(m);
    for (int i=0;i<m;i++) order[i]=i;
    shuffle(order.begin(), order.end(), rng);
    vector<int> delOrder(order.begin(), order.begin() + q);

    vector<int> ans = solveRef(n, edges, delOrder);
    stringstream in; in << n << " " << m << " " << q << "\n";
    for (auto& e : edges) in << e.first << " " << e.second << "\n";
    for (int idxE : delOrder) in << edges[idxE].first << " " << edges[idxE].second << "\n";
    stringstream out;
    for (int i = 0; i < q; i++) out << ans[i] << "\n";
    string e = out.str();
    if (!e.empty() && e.back() == '\n') e.pop_back();
    TestCase tc; tc.input = in.str(); tc.expectedOutput = e;
    return tc;
}
#define PROBLEM_NAME "Видалення кабелів (offline DSU, складна)"
#include "../../common/tester_main.inc"
