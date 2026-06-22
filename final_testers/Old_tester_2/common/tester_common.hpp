// common/tester_common.hpp
// Спільна інфраструктура для всіх *_tester.cpp:
//  - компіляція розв'язку користувача (з .cpp-файлу або з тексту коду)
//  - запуск скомпільованої програми з обмеженням за часом (TL)
//  - точне порівняння виводу (strict diff, з толерантністю лише до
//    кінцевих пробілів/порожніх рядків і CRLF/LF)
//  - вердикти: OK, WA, TL, RE, CE
//
// Використання конкретною задачею: дивись шаблон у tester_template.cpp.
//
#pragma once
#include <bits/stdc++.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
using namespace std;

namespace tester {

enum class Verdict { OK, WA, TL, RE, CE };

inline string verdictName(Verdict v) {
    switch (v) {
        case Verdict::OK: return "OK";
        case Verdict::WA: return "WA";
        case Verdict::TL: return "TL";
        case Verdict::RE: return "RE";
        case Verdict::CE: return "CE";
    }
    return "??";
}

// Прибирає \r, прибирає кінцеві пробіли в кожному рядку та порожні рядки в кінці.
inline string normalize(const string& s) {
    string t;
    t.reserve(s.size());
    for (char c : s) if (c != '\r') t.push_back(c);
    // прибрати кінцеві пробіли/таби в кожному рядку
    vector<string> lines;
    string cur;
    for (char c : t) {
        if (c == '\n') { lines.push_back(cur); cur.clear(); }
        else cur.push_back(c);
    }
    lines.push_back(cur);
    for (auto& ln : lines) {
        while (!ln.empty() && (ln.back() == ' ' || ln.back() == '\t')) ln.pop_back();
    }
    while (!lines.empty() && lines.back().empty()) lines.pop_back();
    string res;
    for (size_t i = 0; i < lines.size(); i++) {
        res += lines[i];
        if (i + 1 < lines.size()) res += '\n';
    }
    return res;
}

inline bool outputsMatch(const string& expected, const string& actual) {
    return normalize(expected) == normalize(actual);
}

// Компілює cpp-файл у виконуваний бінарник. Повертає true при успіху.
inline bool compileCpp(const string& cppPath, const string& outBinPath, string& compileLog) {
    string cmd = "g++ -O2 -std=c++17 -o " + outBinPath + " " + cppPath + " 2> " + outBinPath + ".clog";
    int rc = system(cmd.c_str());
    ifstream log(outBinPath + ".clog");
    stringstream ss;
    ss << log.rdbuf();
    compileLog = ss.str();
    return rc == 0;
}

// Якщо нам дали текст коду (не файл) - записуємо у тимчасовий .cpp і компілюємо.
inline bool compileFromSource(const string& sourceCode, const string& outBinPath, string& compileLog) {
    string tmpCpp = outBinPath + ".cpp";
    ofstream f(tmpCpp);
    f << sourceCode;
    f.close();
    return compileCpp(tmpCpp, outBinPath, compileLog);
}

// Результат запуску програми на одному тесті.
struct RunResult {
    bool timedOut = false;
    bool crashed = false;   // ненульовий код виходу / сигнал
    string output;
    long long timeMs = 0;
};

// Запускає програму за вектором аргументів команди (argv[0] = виконуваний файл
// або інтерпретатор, далі його аргументи), подає input на stdin, обмежує час
// timeLimitMs. Робоча тека для процесу - workDir (потрібно для мов, де файли
// допоміжних класів/модулів мають лежати у певній теці, напр. Java).
inline RunResult runCommand(const vector<string>& argv, const string& input,
                             int timeLimitMs, const string& workDir = "") {
    RunResult result;
    if (argv.empty()) { result.crashed = true; return result; }

    int inPipe[2];
    int outPipe[2];
    if (pipe(inPipe) != 0 || pipe(outPipe) != 0) {
        result.crashed = true;
        return result;
    }

    pid_t pid = fork();
    if (pid == 0) {
        dup2(inPipe[0], STDIN_FILENO);
        dup2(outPipe[1], STDOUT_FILENO);
        close(inPipe[0]); close(inPipe[1]);
        close(outPipe[0]); close(outPipe[1]);
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull >= 0) { dup2(devnull, STDERR_FILENO); close(devnull); }
        if (!workDir.empty()) chdir(workDir.c_str());

        vector<char*> cargv;
        cargv.reserve(argv.size() + 1);
        for (auto& s : argv) cargv.push_back(const_cast<char*>(s.c_str()));
        cargv.push_back(nullptr);
        execvp(cargv[0], cargv.data());
        _exit(127); // якщо execvp не вдався (не знайдено інтерпретатор/бінарник)
    }

    close(inPipe[0]);
    close(outPipe[1]);

    pid_t writerPid = fork();
    if (writerPid == 0) {
        close(outPipe[0]);
        size_t written = 0;
        const char* data = input.c_str();
        size_t len = input.size();
        while (written < len) {
            ssize_t w = write(inPipe[1], data + written, len - written);
            if (w <= 0) break;
            written += w;
        }
        close(inPipe[1]);
        _exit(0);
    }
    close(inPipe[1]);

    auto start = chrono::steady_clock::now();
    string output;
    char buf[65536];
    fcntl(outPipe[0], F_SETFL, O_NONBLOCK);

    bool childDone = false;
    int status = 0;
    bool statusKnown = false;
    while (true) {
        auto now = chrono::steady_clock::now();
        long long elapsed = chrono::duration_cast<chrono::milliseconds>(now - start).count();
        if (elapsed >= timeLimitMs) {
            result.timedOut = true;
            kill(pid, SIGKILL);
            waitpid(pid, &status, 0);
            statusKnown = true;
            break;
        }
        ssize_t r = read(outPipe[0], buf, sizeof(buf));
        if (r > 0) {
            output.append(buf, r);
            continue;
        } else if (r == 0) {
            long long remaining = timeLimitMs - elapsed;
            if (remaining <= 0) { result.timedOut = true; kill(pid, SIGKILL); waitpid(pid, &status, 0); statusKnown = true; break; }
            auto eofStart = chrono::steady_clock::now();
            bool exited = false;
            while (true) {
                int wr = waitpid(pid, &status, WNOHANG);
                if (wr == pid) { exited = true; statusKnown = true; break; }
                auto nowInner = chrono::steady_clock::now();
                long long elapsedTotal = chrono::duration_cast<chrono::milliseconds>(nowInner - start).count();
                if (elapsedTotal >= timeLimitMs) break;
                long long elapsedSinceEof = chrono::duration_cast<chrono::milliseconds>(nowInner - eofStart).count();
                if (elapsedSinceEof >= 200) break;
                usleep(1000);
            }
            if (exited) { childDone = true; break; }
            continue;
        } else {
            int wr = waitpid(pid, &status, WNOHANG);
            if (wr == pid) { childDone = true; statusKnown = true; break; }
            usleep(2000);
        }
    }
    auto end = chrono::steady_clock::now();
    result.timeMs = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    if (childDone) {
        fcntl(outPipe[0], F_SETFL, O_NONBLOCK);
        while (true) {
            ssize_t r = read(outPipe[0], buf, sizeof(buf));
            if (r > 0) output.append(buf, r);
            else break;
        }
    }

    close(outPipe[0]);
    int wstatus;
    waitpid(writerPid, &wstatus, 0);

    if (!result.timedOut && statusKnown) {
        if (WIFEXITED(status)) {
            int code = WEXITSTATUS(status);
            if (code != 0) result.crashed = true;
        } else if (WIFSIGNALED(status)) {
            result.crashed = true;
        }
    } else if (!result.timedOut && !statusKnown) {
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            int code = WEXITSTATUS(status);
            if (code != 0) result.crashed = true;
        } else if (WIFSIGNALED(status)) {
            result.crashed = true;
        }
    }
    result.output = output;
    return result;
}

// Сумісність зі старим кодом (120 існуючих *_tester.cpp викликають саме цю
// сигнатуру): запуск одного скомпільованого бінарника без аргументів.
inline RunResult runWithInput(const string& binPath, const string& input, int timeLimitMs) {
    return runCommand({binPath}, input, timeLimitMs);
}

struct TestCase {
    string input;
    string expectedOutput;
};

struct TestSummary {
    int total = 0;
    int ok = 0;
    int wa = 0;
    int tl = 0;
    int re = 0;
    vector<pair<int, Verdict>> perTest; // (номер тесту з 1, вердикт)
};

// Основний прогін: компілює розв'язок користувача, виконує всі тести,
// друкує вердикт по кожному й підсумок.
// verbose=true друкує рядок на кожен тест; verbose=false друкує лише
// підсумкову стрічку символів (О=OK, W=WA, T=TL, R=RE) для компактності.
inline TestSummary runAllTests(const string& userBinPath,
                                const vector<TestCase>& tests,
                                int timeLimitMs,
                                bool verbose = true) {
    TestSummary summary;
    summary.total = (int)tests.size();
    string compactLine;
    for (int i = 0; i < (int)tests.size(); i++) {
        const auto& tc = tests[i];
        RunResult rr = runWithInput(userBinPath, tc.input, timeLimitMs);
        Verdict v;
        if (rr.timedOut) { v = Verdict::TL; summary.tl++; }
        else if (rr.crashed) { v = Verdict::RE; summary.re++; }
        else if (outputsMatch(tc.expectedOutput, rr.output)) { v = Verdict::OK; summary.ok++; }
        else { v = Verdict::WA; summary.wa++; }
        summary.perTest.push_back({i + 1, v});
        if (verbose) {
            cout << "Тест " << setw(3) << (i + 1) << "/" << tests.size()
                 << "  [" << verdictName(v) << "]"
                 << "  час=" << rr.timeMs << "мс" << "\n";
        } else {
            char c = v == Verdict::OK ? '.' : (v == Verdict::WA ? 'W' : (v == Verdict::TL ? 'T' : 'R'));
            compactLine.push_back(c);
            if (compactLine.size() % 50 == 0) { cout << compactLine << "\n"; compactLine.clear(); }
        }
    }
    if (!verbose && !compactLine.empty()) cout << compactLine << "\n";
    return summary;
}

inline void printSummary(const TestSummary& s) {
    cout << "\n========== ПІДСУМОК ==========\n";
    cout << "Усього тестів: " << s.total << "\n";
    cout << "OK (правильно): " << s.ok << "\n";
    cout << "WA (неправильна відповідь): " << s.wa << "\n";
    cout << "TL (перевищено час): " << s.tl << "\n";
    cout << "RE (помилка виконання): " << s.re << "\n";
    double pct = s.total ? (100.0 * s.ok / s.total) : 0.0;
    cout << fixed << setprecision(1) << "Пройдено: " << pct << "%\n";
    if (s.ok == s.total) cout << "РЕЗУЛЬТАТ: ACCEPTED\n";
    else cout << "РЕЗУЛЬТАТ: REJECTED\n";
}

} // namespace tester
