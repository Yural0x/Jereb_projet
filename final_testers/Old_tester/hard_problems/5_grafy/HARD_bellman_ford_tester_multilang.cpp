// HARD_bellman_ford_tester.cpp — "Валютний арбітраж"
// Примітка: щоб уникнути проблем точності з рядковим порівнянням на граничних
// випадках "майже 1.0", генератор будує явно однозначні приклади: або точно
// побудований цикл прибутку (добуток курсів суттєво > 1), або суто ациклічний
// граф обмінів без жодного циклу взагалі (тоді арбітражу гарантовано немає).
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

static bool hasNegativeCycleLog(int n, vector<array<double,3>>& edges) {
    // ребра вже як (u, v, -log(rate))
    vector<double> dist(n+1, 0.0);
    for (int iter = 0; iter < n; iter++) {
        bool changed = false;
        for (auto& e : edges) {
            int u=(int)e[0], v=(int)e[1]; double w=e[2];
            if (dist[u] + w < dist[v] - 1e-9) { dist[v] = dist[u] + w; changed = true; }
        }
        if (!changed) return false;
    }
    return true;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 2 : uniform_int_distribution<int>(2, 500)(rng);
    bool wantArb = uniform_int_distribution<int>(0,1)(rng);
    if (idx == 0) wantArb = true;

    vector<array<double,3>> logEdges; // для еталону
    vector<array<double,2>> rawRates; // (u,v) -> rate, для виводу як double
    vector<array<int,2>> pairs;

    if (wantArb && n >= 2) {
        // явний прибутковий цикл: 1->2 з курсом 2.0, 2->1 з курсом 0.6 (добуток 1.2 > 1)
        int u = 1, v = 2;
        pairs.push_back({u, v}); rawRates.push_back({2.0, 0});
        pairs.push_back({v, u}); rawRates.push_back({0.6, 0});
        logEdges.push_back({(double)u, (double)v, -log(2.0)});
        logEdges.push_back({(double)v, (double)u, -log(0.6)});
    } else if (n >= 2) {
        // ациклічний орієнтований граф обмінів (тільки i -> j для i < j), курси з малим прибутком
        // не можуть утворити жодного циклу, бо ребра завжди йдуть "вперед".
        uniform_real_distribution<double> rd(0.5, 1.5);
        int m = uniform_int_distribution<int>(0, min(2000, n))(rng);
        set<pair<int,int>> used;
        int attempts = 0;
        while ((int)pairs.size() < m && attempts < m*5+50) {
            attempts++;
            int a = uniform_int_distribution<int>(1, n-1)(rng);
            int b = uniform_int_distribution<int>(a+1, n)(rng);
            if (used.count({a,b})) continue;
            used.insert({a,b});
            double rate = rd(rng);
            pairs.push_back({a,b}); rawRates.push_back({rate, 0});
            logEdges.push_back({(double)a, (double)b, -log(rate)});
        }
    }
    int m = (int)pairs.size();

    bool ans = hasNegativeCycleLog(n, logEdges);
    stringstream in; in << n << " " << m << "\n";
    in << fixed << setprecision(6);
    for (int i = 0; i < m; i++) in << pairs[i][0] << " " << pairs[i][1] << " " << rawRates[i][0] << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = ans ? "YES" : "NO";
    return tc;
}
#define PROBLEM_NAME "Валютний арбітраж (Беллман-Форд, складна)"
#include "../../common/tester_main_multilang.inc"
