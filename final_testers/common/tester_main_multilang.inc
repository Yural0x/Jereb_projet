// common/tester_main_multilang.inc
// Альтернатива tester_main.inc: те саме, але розв'язок користувача може бути
// будь-якою з 10 підтримуваних мов (визначається за розширенням файлу).
// Підключається наприкінці *_tester.cpp після визначення:
//   - PROBLEM_NAME, NUM_TESTS, TL_MS
//   - TestCase genTest(mt19937& rng, int idx)
// і після #include "../../common/language.hpp" (замість tester_common.hpp напряму).
//
// Відмінність від tester_main.inc: аргумент командного рядка - це шлях до
// файлу з розв'язком у БУДЬ-ЯКІЙ із підтримуваних мов (визначається за
// розширенням: .cpp .c .py .js .ts .java .cs .go .rs .php). Прапор --source
// тут не застосовується - мова потребує реального файлу з правильним
// розширенням (а не довільного тексту), бо саме розширення визначає, який
// компілятор/інтерпретатор використати.

#ifndef PROBLEM_NAME
#define PROBLEM_NAME "Без назви"
#endif

// Просте base64-кодування — потрібне щоб безпечно передати багаторядкові
// вхідні/очікувані/фактичні дані тесту через stdout одним рядком (без
// конфлікту з символами нового рядка чи розділювачами власного формату).
inline string b64encode(const string& data) {
    static const char* tbl =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    string out;
    int val = 0, valb = -6;
    for (unsigned char c : data) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(tbl[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(tbl[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Використання: " << argv[0] << " <шлях_до_розвязку>\n";
        cerr << "  Мова визначається за розширенням файлу:\n";
        cerr << "    .cpp/.cc/.cxx -> C++      .c -> C            .py -> Python\n";
        cerr << "    .js/.mjs -> JavaScript    .ts -> TypeScript  .java -> Java\n";
        cerr << "    .cs -> C#                 .go -> Go          .rs -> Rust\n";
        cerr << "    .php -> PHP\n";
        cerr << "  Прапори:\n";
        cerr << "    --quiet    компактний вивід\n";
        cerr << "    --seed N   змінити сід генератора тестів\n";
        return 2;
    }

    string sourcePath = argv[1];
    bool quiet = false;
    unsigned int seed = 20260620u;
    for (int i = 2; i < argc; i++) {
        string a = argv[i];
        if (a == "--quiet") quiet = true;
        else if (a == "--seed" && i + 1 < argc) { seed = (unsigned int)stoul(argv[++i]); }
    }

    Lang lang = detectLanguage(sourcePath);
    if (lang == Lang::UNKNOWN) {
        cout << "[" << verdictName(Verdict::CE) << "] Не вдалося визначити мову за розширенням файлу: "
             << sourcePath << "\n";
        return 1;
    }

    string workDir = "./_tester_work_" + to_string(getpid());
    int mkdirRc = system(("mkdir -p " + workDir).c_str());
    if (mkdirRc != 0) {
        cerr << "Не вдалося створити робочу теку\n";
        return 2;
    }

    LanguagePrep prep;
    string prepLog;
    bool ok = prepareSolution(sourcePath, workDir, prep, prepLog);
    if (!ok) {
        cout << "[" << verdictName(Verdict::CE) << "] Помилка підготовки розв'язку (" << langName(lang) << "):\n";
        cout << prepLog << "\n";
        (void)system(("rm -rf " + workDir).c_str());
        return 1;
    }

    mt19937 rng(seed);
    vector<TestCase> tests;
    tests.reserve(NUM_TESTS);
    for (int i = 0; i < NUM_TESTS; i++) tests.push_back(genTest(rng, i));

    cout << "Задача: " << PROBLEM_NAME << "\n";
    cout << "Мова розв'язку: " << langName(lang) << "\n";
    cout << "Кількість тестів: " << NUM_TESTS << ", ліміт часу: " << TL_MS << " мс\n\n";

    TestSummary summary;
    summary.total = (int)tests.size();
    string compactLine;
    // Дані кожного тесту (вхід/очікуваний вивід/фактичний вивід) у
    // base64 — друкуються в окремому блоці наприкінці, щоб не заважати
    // звичайному парсингу рядків "Тест N/M [VERDICT] час=Xмс" вище.
    vector<string> testDataLines;
    for (int i = 0; i < (int)tests.size(); i++) {
        const auto& tc = tests[i];
        RunResult rr = runCommand(prep.runArgv, tc.input, TL_MS, prep.runDir);
        Verdict v;
        if (rr.timedOut) { v = Verdict::TL; summary.tl++; }
        else if (rr.crashed) { v = Verdict::RE; summary.re++; }
        else if (outputsMatch(tc.expectedOutput, rr.output)) { v = Verdict::OK; summary.ok++; }
        else { v = Verdict::WA; summary.wa++; }
        summary.perTest.push_back({i + 1, v});
        if (!quiet) {
            cout << "Тест " << setw(3) << (i + 1) << "/" << tests.size()
                 << "  [" << verdictName(v) << "]"
                 << "  час=" << rr.timeMs << "мс" << "\n";
        } else {
            char c = v == Verdict::OK ? '.' : (v == Verdict::WA ? 'W' : (v == Verdict::TL ? 'T' : 'R'));
            compactLine.push_back(c);
            if (compactLine.size() % 50 == 0) { cout << compactLine << "\n"; compactLine.clear(); }
        }
        // TESTDATA <idx> <input_b64> <expected_b64> <actual_b64>
        // КРИТИЧНО: b64encode("") повертає порожній рядок "" (а не "=="
        // чи щось непорожнє) — якщо фактичний вивід розв'язку порожній
        // (типово для WA, коли користувач забув print() або вивів у
        // stderr), четверте поле в рядку стає порожнім, і рядок виглядає
        // як "TESTDATA 3 AAA= BBB= " — лише 3 непорожні токени замість 4.
        // Парсер на Python-стороні (regex з (\S+) для кожного з 4 полів)
        // не зможе матчити такий рядок взагалі, бо четверте \S+ вимагає
        // хоч один непробільний символ. Тому для порожніх полів вставляємо
        // явний маркер "EMPTY" — він завжди недвозначно непорожній.
        {
            string inB64 = b64encode(tc.input);
            string expB64 = b64encode(tc.expectedOutput);
            string actB64 = b64encode(rr.output);
            if (inB64.empty())  inB64  = "EMPTY";
            if (expB64.empty()) expB64 = "EMPTY";
            if (actB64.empty()) actB64 = "EMPTY";
            testDataLines.push_back(
                "TESTDATA " + to_string(i + 1) + " " + inB64 + " " + expB64 + " " + actB64
            );
        }
    }
    if (quiet && !compactLine.empty()) cout << compactLine << "\n";

    printSummary(summary);

    // Блок з деталями кожного тесту — завжди друкується (і в звичайному,
    // і в --quiet режимі), окремою секцією після підсумку.
    cout << "\n===TESTDATA-BEGIN===\n";
    for (const auto& line : testDataLines) cout << line << "\n";
    cout << "===TESTDATA-END===\n";

    (void)system(("rm -rf " + workDir).c_str());
    return (summary.ok == summary.total) ? 0 : 1;
}
