// MED_convex_hull_tester.cpp — "Площа ділянки"
#include "../../common/language.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

typedef long long ll;
struct Pt { ll x, y; };
static ll cross(const Pt& O, const Pt& A, const Pt& B) {
    return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
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
static ll doubleArea(const vector<Pt>& hull) {
    int n = (int)hull.size();
    if (n < 3) return 0;
    ll area = 0;
    for (int i = 0; i < n; i++) {
        int j = (i + 1) % n;
        area += hull[i].x * hull[j].y - hull[j].x * hull[i].y;
    }
    return abs(area);
}

static TestCase genTest(mt19937& rng, int idx) {
    int n;
    vector<Pt> pts;
    if (idx == 0) { n = 5; pts = {{0,0},{4,0},{4,4},{0,4},{2,2}}; }
    else {
        n = uniform_int_distribution<int>(3, 1000)(rng);
        uniform_int_distribution<ll> cd(-1000000, 1000000);
        set<pair<ll,ll>> used;
        while ((int)used.size() < n) used.insert({cd(rng), cd(rng)});
        for (auto& p : used) pts.push_back({p.first, p.second});
        n = (int)pts.size();
    }
    vector<Pt> hull = buildHull(pts);
    ll area2 = doubleArea(hull);

    stringstream in; in << n << "\n";
    for (auto& p : pts) in << p.x << " " << p.y << "\n";
    TestCase tc; tc.input = in.str(); tc.expectedOutput = to_string(area2);
    return tc;
}
#define PROBLEM_NAME "Площа ділянки (опукла оболонка, середня)"
#include "../../common/tester_main_multilang.inc"
