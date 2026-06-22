// EZ_topological_sort_tester.cpp — "Порядок завдань"
// Примітка: топологічний порядок зазвичай не унікальний, тому для точного
// порівняння (strict diff) генератор будує граф із УНІКАЛЬНИМ топологічним
// порядком - це ланцюг залежностей (кожне завдання залежить від попереднього),
// можливо, з додатковими "надлишковими" ребрами, що узгоджуються з тим самим
// порядком і не створюють альтернатив.
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 3 : uniform_int_distribution<int>(1, 2000)(rng);
    vector<int> order(n);
    for (int i = 0; i < n; i++) order[i] = i + 1;
    shuffle(order.begin(), order.end(), rng);
    // order[i] має йти перед order[i+1] - єдиний топологічний порядок

    vector<pair<int,int>> edges;
    for (int i = 0; i + 1 < n; i++) edges.push_back({order[i], order[i+1]});
    // додаємо кілька надлишкових ребер "i перед j" для i<j у порядку (узгоджені, не створюють альтернатив)
    int extra = uniform_int_distribution<int>(0, min(n, 100))(rng);
    set<pair<int,int>> used(edges.begin(), edges.end());
    for (int t = 0; t < extra && n >= 3; t++) {
        int i = uniform_int_distribution<int>(0, n - 2)(rng);
        int j = uniform_int_distribution<int>(i + 1, n - 1)(rng);
        pair<int,int> e = {order[i], order[j]};
        if (!used.count(e)) { used.insert(e); edges.push_back(e); }
    }
    int m = (int)edges.size();

    stringstream in; in << n << " " << m << "\n";
    for (auto& e : edges) in << e.first << " " << e.second << "\n";
    stringstream out;
    for (int i = 0; i < n; i++) out << order[i] << (i+1<n?' ':' ');
    string e = out.str();
    while (!e.empty() && e.back() == ' ') e.pop_back();
    TestCase tc; tc.input = in.str(); tc.expectedOutput = e;
    return tc;
}
#define PROBLEM_NAME "Порядок завдань (топологічне сортування)"
#include "../../common/tester_main.inc"
