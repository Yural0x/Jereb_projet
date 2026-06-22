// HARD_fenwick_tree_tester.cpp — "К-те у множині"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static TestCase genTest(mt19937& rng, int idx) {
    int q = idx == 0 ? 5 : uniform_int_distribution<int>(1, 3000)(rng);
    multiset<long long> ms;
    uniform_int_distribution<long long> vd(1, 1000000000LL);

    stringstream in; in << q << "\n";
    stringstream out;

    vector<array<long long,2>> fixedOps; // для idx==0
    if (idx == 0) {
        fixedOps = {{0,5},{0,1},{0,3},{1,2},{1,3}}; // +5 +1 +3 ?2 ?3
    }

    for (int t = 0; t < q; t++) {
        int op;
        long long val;
        if (idx == 0) { op = (int)fixedOps[t][0]; val = fixedOps[t][1]; }
        else {
            if (ms.empty()) op = 0; // якщо порожньо - тільки додавання
            else op = uniform_int_distribution<int>(0, 2)(rng); // 0:add,1:query(k),2:remove
        }

        if (op == 0) {
            val = (idx==0) ? val : vd(rng);
            in << "+ " << val << "\n";
            ms.insert(val);
        } else if (op == 2 && !ms.empty()) {
            // видаляємо випадковий елемент із множини
            int pos = uniform_int_distribution<int>(0, (int)ms.size()-1)(rng);
            auto it = ms.begin(); advance(it, pos);
            long long v = *it;
            in << "- " << v << "\n";
            ms.erase(it);
        } else {
            int k = (idx==0) ? (int)val : uniform_int_distribution<int>(1, (int)ms.size())(rng);
            in << "? " << k << "\n";
            auto it = ms.begin(); advance(it, k-1);
            out << *it << "\n";
        }
    }
    string e = out.str();
    if (!e.empty() && e.back() == '\n') e.pop_back();
    TestCase tc; tc.input = in.str(); tc.expectedOutput = e;
    return tc;
}
#define PROBLEM_NAME "К-те у множині (Fenwick, складна)"
#include "../../common/tester_main_multilang.inc"
