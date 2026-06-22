// common/language.hpp
// Мовно-агностичний шар над tester_common.hpp: визначення мови за розширенням
// файлу, підготовка (компіляція/перевірка) розв'язку користувача та побудова
// команди його запуску для 10 мов: C++, C, Python, JavaScript, TypeScript,
// Java, C#, Go, Rust, PHP.
//
// Використання (у *_tester.cpp, ОПЦІЙНО - старий шлях через compileCpp/
// runWithInput далі працює без змін для зворотної сумісності):
//
//   #include "../../common/language.hpp"
//   using namespace tester;
//
//   LanguagePrep prep;
//   string prepLog;
//   bool ok = prepareSolution(sourcePath, workDir, prep, prepLog);
//   if (!ok) { /* CE, показати prepLog */ }
//   RunResult rr = runCommand(prep.runArgv, input, timeLimitMs, prep.runDir);
//
#pragma once
#include "tester_common.hpp"

namespace tester {

enum class Lang { CPP, C, PYTHON, JAVASCRIPT, TYPESCRIPT, JAVA, CSHARP, GO, RUST, PHP, UNKNOWN };

// Визначає мову за розширенням файлу. Регістр розширення не важливий.
inline Lang detectLanguage(const string& path) {
    auto pos = path.find_last_of('.');
    if (pos == string::npos) return Lang::UNKNOWN;
    string ext = path.substr(pos + 1);
    for (auto& c : ext) c = (char)tolower((unsigned char)c);
    if (ext == "cpp" || ext == "cc" || ext == "cxx") return Lang::CPP;
    if (ext == "c") return Lang::C;
    if (ext == "py") return Lang::PYTHON;
    if (ext == "js" || ext == "mjs") return Lang::JAVASCRIPT;
    if (ext == "ts") return Lang::TYPESCRIPT;
    if (ext == "java") return Lang::JAVA;
    if (ext == "cs") return Lang::CSHARP;
    if (ext == "go") return Lang::GO;
    if (ext == "rs") return Lang::RUST;
    if (ext == "php") return Lang::PHP;
    return Lang::UNKNOWN;
}

inline string langName(Lang l) {
    switch (l) {
        case Lang::CPP: return "C++";
        case Lang::C: return "C";
        case Lang::PYTHON: return "Python";
        case Lang::JAVASCRIPT: return "JavaScript";
        case Lang::TYPESCRIPT: return "TypeScript";
        case Lang::JAVA: return "Java";
        case Lang::CSHARP: return "C#";
        case Lang::GO: return "Go";
        case Lang::RUST: return "Rust";
        case Lang::PHP: return "PHP";
        default: return "невідома";
    }
}

// Результат підготовки розв'язку: готова команда для запуску й тека,
// з якої її слід запускати (важливо для Java/C#/TS, де поряд лежать
// допоміжні файли - .class, dll, скомпільований .js тощо).
struct LanguagePrep {
    vector<string> runArgv;  // напр. {"./solution"} або {"python3","solution.py"} або {"java","Main"}
    string runDir;           // робоча тека для запуску (chdir перед execvp); може бути ""
};

namespace detail {

inline bool runShellLogged(const string& cmd, string& log) {
    string logFile = "/tmp/prep_log_" + to_string(getpid()) + "_" + to_string(rand()) + ".txt";
    string full = cmd + " > " + logFile + " 2>&1";
    int rc = system(full.c_str());
    ifstream f(logFile);
    stringstream ss; ss << f.rdbuf();
    log = ss.str();
    remove(logFile.c_str());
    return rc == 0;
}

// Java: ім'я публічного класу повинно збігатися з ім'ям файлу. Витягуємо
// ім'я класу, що містить public static void main, простим пошуком за
// регулярним виразом - достатньо для типових розв'язків змагального
// програмування (один публічний клас на файл).
inline string extractJavaMainClass(const string& source, const string& fallbackName) {
    // шукаємо "public class XYZ" або "class XYZ" (якщо public не вказано явно,
    // javac все одно вимагатиме збігу імені файлу з публічним класом, якщо він є)
    size_t pos = source.find("public class");
    if (pos != string::npos) {
        size_t start = pos + 13;
        while (start < source.size() && isspace((unsigned char)source[start])) start++;
        size_t end = start;
        while (end < source.size() && (isalnum((unsigned char)source[end]) || source[end] == '_')) end++;
        if (end > start) return source.substr(start, end - start);
    }
    return fallbackName;
}

} // namespace detail

// Готує розв'язок користувача до запуску: компілює (де потрібно) або
// перевіряє синтаксис (де компіляція не потрібна), повертає готову команду
// запуску. workDir - тека, де можна створювати тимчасові файли (бінарники,
// .class, скомпільований .js тощо); має існувати.
inline bool prepareSolution(const string& sourcePath, const string& workDirIn,
                             LanguagePrep& prep, string& log) {
    // Перетворюємо workDir на абсолютний шлях, бо runCommand() робить chdir(runDir)
    // у дочірньому процесі ДО execvp; якщо runArgv міститиме відносний шлях
    // (наприклад "./work/solution_bin"), після chdir він буде шуканий відносно
    // НОВОЇ директорії і не знайдеться. Абсолютні шляхи унеможливлюють цю помилку.
    char absBuf[4096];
    string workDir = workDirIn;
    if (!workDirIn.empty() && workDirIn[0] != '/') {
        if (realpath(workDirIn.c_str(), absBuf) != nullptr) workDir = string(absBuf);
    }
    Lang lang = detectLanguage(sourcePath);
    string base = workDir + "/solution";

    switch (lang) {
        case Lang::CPP: {
            string bin = base + "_bin";
            string cmd = "g++ -O2 -std=c++17 -o " + bin + " " + sourcePath;
            if (!detail::runShellLogged(cmd, log)) return false;
            prep.runArgv = {bin};
            prep.runDir = workDir;
            return true;
        }
        case Lang::C: {
            string bin = base + "_bin";
            string cmd = "gcc -O2 -std=c17 -o " + bin + " " + sourcePath + " -lm";
            if (!detail::runShellLogged(cmd, log)) return false;
            prep.runArgv = {bin};
            prep.runDir = workDir;
            return true;
        }
        case Lang::PYTHON: {
            // компіляції немає; перевіряємо лише синтаксис (py_compile), щоб
            // дати вердикт CE на синтаксичну помилку, а не RE.
            string cmd = "python3 -m py_compile " + sourcePath;
            if (!detail::runShellLogged(cmd, log)) return false;
            prep.runArgv = {"python3", sourcePath};
            prep.runDir = workDir;
            return true;
        }
        case Lang::JAVASCRIPT: {
            // Node не вимагає окремої компіляції; швидка перевірка синтаксису
            // через --check, щоб синтаксичні помилки давали CE.
            string cmd = "node --check " + sourcePath;
            if (!detail::runShellLogged(cmd, log)) return false;
            prep.runArgv = {"node", sourcePath};
            prep.runDir = workDir;
            return true;
        }
        case Lang::TYPESCRIPT: {
            // Транспілюємо в JS у робочу теку. Типові помилки (наприклад,
            // відсутній @types/node для require/process) самі по собі не
            // повинні блокувати запуск - у змагальному програмуванні важлива
            // коректність виконання, а не суворість типів. Але якщо помилок
            // багато й вони синтаксичні (TS1xxx), вихідний JS буде невалідним -
            // тому додатково перевіряємо лог: якщо там є помилки КРІМ
            // "Cannot find name 'require'/'process'" (типові помилки відсутніх
            // @types/node), вважаємо це падінням компіляції (CE).
            string outDir = workDir;
            string cmd = "tsc --outDir " + outDir + " --target ES2020 --module commonjs "
                         "--skipLibCheck --noImplicitAny false --noEmitOnError false " + sourcePath;
            detail::runShellLogged(cmd, log);
            string fname = sourcePath.substr(sourcePath.find_last_of("/\\") + 1);
            string jsName = fname.substr(0, fname.find_last_of('.')) + ".js";
            string jsPath = outDir + "/" + jsName;
            ifstream check(jsPath);
            if (!check.good()) return false;

            // Якщо в логі є будь-яка помилка, що НЕ стосується відсутності
            // @types/node (Cannot find name 'require'/'process'/'console'/
            // '__dirname' тощо), вважаємо це справжньою помилкою компіляції.
            istringstream logStream(log);
            string line;
            while (getline(logStream, line)) {
                if (line.find("error TS") == string::npos) continue;
                bool isNodeTypesIssue =
                    line.find("Cannot find name 'require'") != string::npos ||
                    line.find("Cannot find name 'process'") != string::npos ||
                    line.find("Cannot find name '__dirname'") != string::npos ||
                    line.find("Cannot find module") != string::npos;
                if (!isNodeTypesIssue) return false; // справжня помилка - CE
            }
            prep.runArgv = {"node", jsPath};
            prep.runDir = workDir;
            return true;
        }
        case Lang::JAVA: {
            // Java вимагає, щоб ім'я файлу збігалося з ім'ям публічного класу.
            // Копіюємо джерело під правильним іменем у workDir перед компіляцією.
            ifstream src(sourcePath);
            stringstream ss; ss << src.rdbuf();
            string code = ss.str();
            string className = detail::extractJavaMainClass(code, "Main");
            string javaFile = workDir + "/" + className + ".java";
            ofstream out(javaFile);
            out << code;
            out.close();

            string cmd = "javac -d " + workDir + " " + javaFile;
            if (!detail::runShellLogged(cmd, log)) return false;
            prep.runArgv = {"java", "-cp", workDir, className};
            prep.runDir = workDir;
            return true;
        }
        case Lang::CSHARP: {
            // Компілюємо як окремий виконуваний файл через dotnet-script-подібний
            // підхід не використовуємо (потребує проєкт). Натомість покладаємось
            // на присутність mcs (Mono C# compiler) як найпростіший варіант для
            // одиночного файлу; якщо в системі є лише dotnet, потрібен csproj
            // (див. README.md "Нотатки щодо C#" у пакеті мовної підтримки).
            string exe = base + ".exe";
            string cmd = "mcs -optimize+ -out:" + exe + " " + sourcePath;
            if (!detail::runShellLogged(cmd, log)) return false;
            prep.runArgv = {"mono", exe};
            prep.runDir = workDir;
            return true;
        }
        case Lang::GO: {
            string bin = base + "_bin";
            string cmd = "go build -o " + bin + " " + sourcePath;
            if (!detail::runShellLogged(cmd, log)) return false;
            prep.runArgv = {bin};
            prep.runDir = workDir;
            return true;
        }
        case Lang::RUST: {
            string bin = base + "_bin";
            string cmd = "rustc -O -o " + bin + " " + sourcePath;
            if (!detail::runShellLogged(cmd, log)) return false;
            prep.runArgv = {bin};
            prep.runDir = workDir;
            return true;
        }
        case Lang::PHP: {
            string cmd = "php -l " + sourcePath; // -l = lint, лише перевірка синтаксису
            if (!detail::runShellLogged(cmd, log)) return false;
            prep.runArgv = {"php", sourcePath};
            prep.runDir = workDir;
            return true;
        }
        default:
            log = "Не вдалося визначити мову програмування за розширенням файлу: " + sourcePath +
                  "\nПідтримувані розширення: .cpp .cc .cxx .c .py .js .mjs .ts .java .cs .go .rs .php";
            return false;
    }
}

} // namespace tester
