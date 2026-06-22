// common/tester_main.inc
// Підключається наприкінці кожного *_tester.cpp після визначення:
//   - PROBLEM_NAME (рядковий літерал з назвою задачі)
//   - NUM_TESTS, TL_MS (константи)
//   - TestCase genTest(mt19937& rng, int idx)  — генератор тесту
//   - (еталонний розв'язок викликається всередині genTest)
//
// Надає main(), що компілює розв'язок користувача (.cpp файл або текст коду
// через --source), запускає NUM_TESTS тестів і друкує вердикти.

#ifndef PROBLEM_NAME
#define PROBLEM_NAME "Без назви"
#endif

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Використання: " << argv[0] << " <шлях_до_solution.cpp | шлях_до_файлу_з_кодом>\n";
        cerr << "  Прапори:\n";
        cerr << "    --source   аргумент 1 - це файл, що містить ТЕКСТ коду (не .cpp напряму)\n";
        cerr << "    --quiet    компактний вивід (рядки символів замість рядка на кожен тест)\n";
        cerr << "    --seed N   змінити сід генератора тестів (за замовчуванням фіксований)\n";
        return 2;
    }

    string arg1 = argv[1];
    bool isSourceText = false;
    bool quiet = false;
    unsigned int seed = 20260620u;
    for (int i = 2; i < argc; i++) {
        string a = argv[i];
        if (a == "--source") isSourceText = true;
        else if (a == "--quiet") quiet = true;
        else if (a == "--seed" && i + 1 < argc) { seed = (unsigned int)stoul(argv[++i]); }
    }

    string workDir = "./_tester_work_" + string(PROBLEM_NAME).substr(0, 0) + to_string(getpid());
    int mkdirRc = system(("mkdir -p " + workDir).c_str());
    if (mkdirRc != 0) {
        cerr << "Не вдалося створити робочу теку\n";
        return 2;
    }
    string userBin = workDir + "/user_bin";

    string compileLog;
    bool compiled;
    if (isSourceText) {
        ifstream f(arg1);
        if (!f) { cerr << "Не вдалося відкрити файл: " << arg1 << "\n"; return 2; }
        stringstream ss; ss << f.rdbuf();
        compiled = compileFromSource(ss.str(), userBin, compileLog);
    } else {
        compiled = compileCpp(arg1, userBin, compileLog);
    }

    if (!compiled) {
        cout << "[" << verdictName(Verdict::CE) << "] Помилка компіляції розв'язку користувача:\n";
        cout << compileLog << "\n";
        (void)system(("rm -rf " + workDir).c_str());
        return 1;
    }

    mt19937 rng(seed);
    vector<TestCase> tests;
    tests.reserve(NUM_TESTS);
    for (int i = 0; i < NUM_TESTS; i++) tests.push_back(genTest(rng, i));

    cout << "Задача: " << PROBLEM_NAME << "\n";
    cout << "Кількість тестів: " << NUM_TESTS << ", ліміт часу: " << TL_MS << " мс\n\n";

    TestSummary summary = runAllTests(userBin, tests, TL_MS, !quiet);
    printSummary(summary);

    (void)system(("rm -rf " + workDir).c_str());
    return (summary.ok == summary.total) ? 0 : 1;
}
