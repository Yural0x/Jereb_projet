// MED_dsu_tester.cpp — "Запити дружби"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 4 : uniform_int_distribution<int>(1, 2000)(rng);
    int q = idx == 0 ? 3 : uniform_int_distribution<int>(1, 500)(rng);
    vector<int> parent(n+1);
    for (int i = 0; i <= n; i++) parent[i] = i;
    function<int(int)> find = [&](int x){ while (parent[x]!=x) x=parent[x]=parent[parent[x]]; return x; };

    stringstream in; in << n << " " << q << "\n";
    stringstream out;
    uniform_int_distribution<int> nd(1, max(n,1));
    for (int t = 0; t < q; t++) {
        int type;
        if (idx == 0) type = (t == 1) ? 2 : 1;
        else type = uniform_int_distribution<int>(0,1)(rng) + 1;
        int a = nd(rng), b = nd(rng);
        if (idx == 0) { a = 1; b = (t==2) ? 3 : 2; }
        in << type << " " << a << " " << b << "\n";
        if (type == 1) {
            int ra = find(a), rb = find(b);
            if (ra != rb) parent[ra] = rb;
        } else {
            bool same = find(a) == find(b);
            out << (same ? "YES" : "NO") << "\n";
        }
    }
    string e = out.str();
    if (!e.empty() && e.back() == '\n') e.pop_back();
    TestCase tc; tc.input = in.str(); tc.expectedOutput = e;
    return tc;
}
#define PROBLEM_NAME "Запити дружби (DSU онлайн, середня)"
#include "../../common/tester_main.inc"
