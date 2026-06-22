// HARD_convex_hull_tester.cpp — "Діаметр множини точок"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

typedef long long ll;
struct Pt { ll x, y; };
static ll cross(const Pt& O, const Pt& A, const Pt& B) {
    return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}
static ll dist2(const Pt& a, const Pt& b) {
    ll dx = a.x-b.x, dy = a.y-b.y;
    return dx*dx + dy*dy;
}
static vector<Pt> buildHull(vector<Pt> pts) {
    sort(pts.begin(), pts.end(), [](const Pt& a, const Pt& b){
        if (a.x != b.x) return a.x < b.x; return a.y < b.y;
    });
    pts.erase(unique(pts.begin(), pts.end(), [](const Pt&a, const Pt&b){return a.x==b.x && a.y==b.y;}), pts.end());
    int n = (int)pts.size();
    if (n <= 2) return pts;
    vector<Pt> hull(2*n);
    int k = 0;
    for (int i = 0; i < n; i++) {
        while (k >= 2 && cross(hull[k-2], hull[k-1], pts[i]) <= 0) k--;
        hull[k++] = pts[i];
    }
    int lower = k + 1;
    for (int i = n - 2; i >= 0; i--) {
        while (k >= lower && cross(hull[k-2], hull[k-1], pts[i]) <= 0) k--;
        hull[k++] = pts[i];
    }
    hull.resize(k - 1);
    return hull;
}
// Метод обертових опор для діаметра опуклого багатокутника
static ll diameterSquared(vector<Pt> pts) {
    int n0 = (int)pts.size();
    if (n0 == 1) return 0;
    if (n0 == 2) return dist2(pts[0], pts[1]);
    vector<Pt> hull = buildHull(pts);
    int n = (int)hull.size();
    if (n == 1) return 0;
    if (n == 2) return dist2(hull[0], hull[1]);

    ll best = 0;
    int j = 1;
    for (int i = 0; i < n; i++) {
        int ni = (i + 1) % n;
        while (true) {
            int nj = (j + 1) % n;
            ll cur = abs(cross(hull[i], hull[ni], hull[nj]));
            ll prevv = abs(cross(hull[i], hull[ni], hull[j]));
            if (cur > prevv) j = nj; else break;
        }
        best = max(best, dist2(hull[i], hull[j]));
        best = max(best, dist2(hull[ni], hull[j]));
    }
    return best;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n;
    vector<Pt> pts;
    if (idx == 0) { n = 4; pts = {{0,0},{4,0},{4,4},{0,4}}; }
    else {
        n = uniform_int_distribution<int>(2, 2000)(rng);
        uniform_int_distribution<ll> cd(-1000000, 1000000);
        set<pair<ll,ll>> used;
        while ((int)used.size() < n) used.insert({cd(rng), cd(rng)});
        for (auto& p : used) pts.push_back({p.first, p.second});
        n = (int)pts.size();
    }

    ll ans = diameterSquared(pts);
    stringstream in; in << n << "\n";
    for (auto& p : pts) in << p.x << " " << p.y << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(ans);
    return tc;
}
#define PROBLEM_NAME "Діаметр множини точок (складна)"
#include "../../common/tester_main.inc"
