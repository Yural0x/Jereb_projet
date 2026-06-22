// HARD_linear_search_tester.cpp — "Дві третини голосів"
#include "../../common/tester_common.hpp"
using namespace tester;
static const int NUM_TESTS = 100;
static const int TL_MS = 2000;

// Узагальнений алгоритм Бойера-Мура для пошуку елементів з частотою > n/3.
static vector<long long> solveRef(int n, vector<long long>& a) {
    long long c1 = 0, c2 = 0; int cnt1 = 0, cnt2 = 0;
    bool has1 = false, has2 = false;
    for (long long x : a) {
        if (has1 && x == c1) cnt1++;
        else if (has2 && x == c2) cnt2++;
        else if (cnt1 == 0) { c1 = x; cnt1 = 1; has1 = true; }
        else if (cnt2 == 0) { c2 = x; cnt2 = 1; has2 = true; }
        else { cnt1--; cnt2--; }
    }
    cnt1 = cnt2 = 0;
    for (long long x : a) { if (has1 && x == c1) cnt1++; else if (has2 && x == c2) cnt2++; }
    vector<long long> res;
    if (has1 && cnt1 > n / 3) res.push_back(c1);
    if (has2 && cnt2 > n / 3) res.push_back(c2);
    sort(res.begin(), res.end());
    return res;
}

static TestCase genTest(mt19937& rng, int idx) {
    int n = idx == 0 ? 1 : uniform_int_distribution<int>(1, 5000)(rng);
    uniform_int_distribution<long long> vd(1, 1000000000LL);
    vector<long long> a(n);
    // інколи штучно створюємо явного лідера, щоб перевірити позитивні випадки
    if (n >= 3 && uniform_int_distribution<int>(0,1)(rng)) {
        long long leader = vd(rng);
        int leaderCount = n/2 + 1 + uniform_int_distribution<int>(0, n/4 + 1)(rng);
        leaderCount = min(leaderCount, n);
        for (int i = 0; i < leaderCount; i++) a[i] = leader;
        for (int i = leaderCount; i < n; i++) a[i] = vd(rng);
        shuffle(a.begin(), a.end(), rng);
    } else {
        for (auto& v : a) v = vd(rng);
    }

    vector<long long> res = solveRef(n, a);
    stringstream in; in << n << "\n";
    for (int i = 0; i < n; i++) in << a[i] << (i+1<n?' ':'\n');
    stringstream out;
    for (size_t i = 0; i < res.size(); i++) out << res[i] << (i+1<res.size()?' ':' ');
    string e = out.str();
    while (!e.empty() && e.back() == ' ') e.pop_back();
    TestCase tc; tc.input = in.str(); tc.expectedOutput = e;
    return tc;
}
#define PROBLEM_NAME "Дві третини голосів (Бойер-Мур, складна)"
#include "../../common/tester_main.inc"
