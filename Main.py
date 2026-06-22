import sys
import os
import re
import json
import urllib.request
import urllib.error

from PySide6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QStackedWidget,
    QVBoxLayout, QHBoxLayout, QPushButton,
    QLabel, QSizePolicy, QSpacerItem,
    QGraphicsDropShadowEffect, QScrollArea,
    QTextEdit, QPlainTextEdit, QFileDialog, QListWidget, QListWidgetItem,
    QTextBrowser, QComboBox, QDialog, QLineEdit, QCheckBox
)
from PySide6.QtCore import Qt, QPoint, QRect, QPropertyAnimation, QEasingCurve, QSize, QUrl, QThread, Signal, QProcess, QTimer
from PySide6.QtGui import (
    QResizeEvent, QColor, QPainter, QPixmap,
    QPainterPath, QBrush, QPen, QIcon, QLinearGradient, QFont
)

# ──────────────────────────────────────────────
#  QWebEngineView — потрібен для виконання JS у HTML-умовах задач
#  (кнопки "копіювати" в problem.html використовують navigator.clipboard).
#  QTextBrowser HTML не виконує JS, тому без WebEngine кнопки не працюють.
#  Якщо пакет 'PySide6-Addons' / QtWebEngine не встановлений — програма
#  автоматично відкочується на QTextBrowser (умова відображається, але
#  кнопка копіювання в такому разі не активна).
# ──────────────────────────────────────────────
try:
    from PySide6.QtWebEngineWidgets import QWebEngineView
    from PySide6.QtWebEngineCore import QWebEngineSettings, QWebEnginePage
    _WEBENGINE_AVAILABLE = True
except ImportError:
    _WEBENGINE_AVAILABLE = False

# ──────────────────────────────────────────────
#  Базова папка проєкту — визначається відносно
#  місця розташування цього файлу (app.py),
#  а не від конкретного користувача/диска.
#
#  Очікувана структура:
#    Jereb_project_1/
#      Short_discription/  SD_Search, SD_Sort, ...
#      Main_discriptio/    Search, Sort, ...
#      <тут лежить app.py або pythonProject/app.py>
#
#  Якщо app.py лежить НЕ всередині Jereb_project_1,
#  поправ _PROJECT_ROOT нижче вручну.
# ──────────────────────────────────────────────
def _find_project_root() -> str:
    """Шукає папку 'Jereb_project_1' серед батьківських директорій файлу."""
    here = os.path.abspath(os.path.dirname(__file__))
    cur = here
    for _ in range(8):  # обмежена кількість підйомів вгору
        if os.path.basename(cur) == "Jereb_project_1":
            return cur
        candidate = os.path.join(cur, "Jereb_project_1")
        if os.path.isdir(candidate):
            return candidate
        parent = os.path.dirname(cur)
        if parent == cur:
            break
        cur = parent
    # Фолбек: припускаємо що app.py лежить прямо в Jereb_project_1
    return here


_PROJECT_ROOT = _find_project_root()

# ──────────────────────────────────────────────
#  Шлях до папки з короткими описами (картка 1)
# ──────────────────────────────────────────────
_SD_BASE = os.path.join(_PROJECT_ROOT, "Short_discription")

_SD_DIRS = {
    "search":  os.path.join(_SD_BASE, "SD_Search"),
    "sort":    os.path.join(_SD_BASE, "SD_Sort"),
    "math":    os.path.join(_SD_BASE, "SD_Math"),
    "greedy":  os.path.join(_SD_BASE, "SD_Gridy"),
    "graphs":  os.path.join(_SD_BASE, "SD_Graphs"),
    "dynamic": os.path.join(_SD_BASE, "SD_Dynamic"),
    "string":  os.path.join(_SD_BASE, "SD_String"),
    "trees":   os.path.join(_SD_BASE, "SD_trees"),
}

# ──────────────────────────────────────────────
#  Шлях до папки з детальними описами (картка 2 — "Детальніше")
# ──────────────────────────────────────────────
_MD_BASE = os.path.join(_PROJECT_ROOT, "Main_discriptio")

_MD_DIRS = {
    "search":  os.path.join(_MD_BASE, "Search"),
    "sort":    os.path.join(_MD_BASE, "Sort"),
    "math":    os.path.join(_MD_BASE, "Math"),
    "greedy":  os.path.join(_MD_BASE, "Greedy"),
    "graphs":  os.path.join(_MD_BASE, "Graphs"),
    "dynamic": os.path.join(_MD_BASE, "Dynamic"),
    "string":  os.path.join(_MD_BASE, "Strings"),
    "trees":   os.path.join(_MD_BASE, "Trees"),
}

# ──────────────────────────────────────────────
#  Шлях до HTML-умови для "Випадкової задачі"
#  (Jereb_project_1/Problems/problem.html)
# ──────────────────────────────────────────────
PROBLEM_HTML_PATH = os.path.join(_PROJECT_ROOT, "Problems", "problem.html")

# ──────────────────────────────────────────────
#  Умови задач за рівнями складності
#  (Jereb_project_1/Problems/{ez,med,hard}_problems/<категорія>/<PREFIX>_<file>.html)
#
#  Для кожного алгоритму є 3 варіанти умови задачі — Easy/Medium/Hard.
#  Це НЕЗАЛЕЖНО від розділу рівневої системи (Easy/Medium/Hard/Insane),
#  з якого користувач відкрив картку — там лише групування алгоритмів,
#  а тут вибір складності самої задачі.
# ──────────────────────────────────────────────
_PROBLEMS_BASE = os.path.join(_PROJECT_ROOT, "Problems")

# Рівні задач, що реально є у файловій системі (немає 'insane' умов).
PROBLEM_LEVELS = [
    ("ez",   "Easy",   "ez_problems"),
    ("med",  "Medium", "med_problems"),
    ("hard", "Hard",   "hard_problems"),
]
PROBLEM_LEVEL_PREFIX = {"ez": "EZ", "med": "MED", "hard": "HARD"}

# subdir_key (як у _SD_DIRS/_MD_DIRS) → номер+назва категорійної папки в Problems
_PROBLEM_CATEGORY_DIR = {
    "search":  "1_poshuk_ta_masyvy",
    "sort":    "2_sortuvannia",
    "math":    "3_matematyka",
    "greedy":  "4_zhadibni",
    "graphs":  "5_grafy",
    "dynamic": "6_dynamichne_programuvannia",
    "string":  "7_riadkovi_algoritmy",
    "trees":   "8_struktury_danykh",
}

# Деякі файли задач названі коротше, ніж filename у _load()/_load_detail().
# Мапінг: (subdir_key, load_filename) → problem_filename
_PROBLEM_FILENAME_OVERRIDE = {
    ("greedy",  "greedy_algorithms"):              "greedy",
    ("graphs",  "strongly_connected_components"):  "scc",
    ("dynamic", "dynamic_programming_basics"):     "dp_basics",
    ("trees",   "disjoint_set_union"):              "dsu",
}


def _problem_filename(subdir_key: str, load_filename: str) -> str:
    """Перетворює filename, який використовується в _load(), на ім'я файлу задачі."""
    return _PROBLEM_FILENAME_OVERRIDE.get((subdir_key, load_filename), load_filename)


def get_problem_path(subdir_key: str, load_filename: str, level_key: str) -> str:
    """
    Будує шлях до HTML-умови задачі для конкретного алгоритму й рівня складності.

    subdir_key    — категорія алгоритму ('search', 'sort', 'graphs', ...)
    load_filename — ім'я файлу як у _load()/_load_detail() (напр. 'linear_search')
    level_key     — 'ez' | 'med' | 'hard'
    """
    category_dir = _PROBLEM_CATEGORY_DIR.get(subdir_key)
    if category_dir is None:
        return ""
    fname = _problem_filename(subdir_key, load_filename)
    prefix = PROBLEM_LEVEL_PREFIX.get(level_key, level_key.upper())
    level_dir = dict((k, d) for k, _, d in PROBLEM_LEVELS).get(level_key, "")
    return os.path.join(_PROBLEMS_BASE, level_dir, category_dir, f"{prefix}_{fname}.html")


# ──────────────────────────────────────────────
#  Тестери (final_testers) — компіляція й запуск
#  розв'язку користувача проти еталону для кожної задачі.
# ──────────────────────────────────────────────
_TESTERS_BASE = os.path.join(_PROJECT_ROOT, "final_testers")

PROBLEM_LEVEL_TESTER_DIR = {"ez": "ez_problems", "med": "med_problems", "hard": "hard_problems"}

# Мови розв'язку, що підтримуються мультимовним тестером (final_testers/common/language.hpp).
# Кожен запис: (label_для_кнопки, розширення_файлу_без_крапки)
SOLUTION_LANGUAGES = [
    ("C++",         "cpp"),
    ("C",           "c"),
    ("Python",      "py"),
    ("JavaScript",  "js"),
    ("TypeScript",  "ts"),
    ("Java",        "java"),
    ("C#",          "cs"),
    ("Go",          "go"),
    ("Rust",        "rs"),
    ("PHP",         "php"),
]
DEFAULT_LANGUAGE_EXT = "cpp"

# ──────────────────────────────────────────────
#  Перевірка наявності компіляторів/інтерпретаторів
#  для кожної підтримуваної мови.
# ──────────────────────────────────────────────
import shutil as _shutil

# ext → список можливих імен команди (перша знайдена — використовується)
_LANG_TOOL_CANDIDATES = {
    "cpp":  ["g++"],
    "c":    ["gcc"],
    "py":   ["python3", "python"],
    "js":   ["node"],
    "ts":   ["tsc"],
    "java": ["javac"],
    "cs":   ["mcs", "csc", "dotnet"],
    "go":   ["go"],
    "rs":   ["rustc"],
    "php":  ["php"],
}


def find_tool(ext: str):
    """
    Повертає шлях до знайденого інструмента для розширення мови, або None.
    Це для КОМПІЛЯЦІЇ РОЗВ'ЯЗКУ КОРИСТУВАЧА — там підійде будь-який g++
    у PATH (ucrt64/mingw64 чи MSYS, fork() не потрібен для самого розв'язку).
    """
    for name in _LANG_TOOL_CANDIDATES.get(ext, []):
        path = _shutil.which(name)
        if path:
            return path
    return None


def check_compilers() -> dict:
    """
    Перевіряє наявність усіх потрібних компіляторів/інтерпретаторів
    для МОВ РОЗВ'ЯЗКІВ користувача (будь-який g++ підходить для cpp).
    Повертає {ext: bool}.

    ОКРЕМО від цього — компіляція самого тестера (final_testers) вимагає
    специфічно MSYS-g++ з підтримкою fork(); це перевіряється функцією
    find_msys_gpp() / is_msys_gpp_available(), не тут.
    """
    return {ext: find_tool(ext) is not None for _, ext in SOLUTION_LANGUAGES}


def missing_languages() -> list:
    """Список (label, ext) мов, для яких не знайдено інструмент."""
    status = check_compilers()
    return [(label, ext) for label, ext in SOLUTION_LANGUAGES if not status.get(ext)]


# ──────────────────────────────────────────────
#  Автоматичне встановлення MSYS2 + g++ (Windows)
#  через winget. Потребує підвищених прав (UAC).
# ──────────────────────────────────────────────
def is_windows() -> bool:
    return os.name == "nt"


def msys2_ucrt64_bin_path() -> str:
    return os.path.join("C:\\", "msys64", "ucrt64", "bin")


def msys2_msys_bin_path() -> str:
    """
    Шлях до базового MSYS-середовища (НЕ ucrt64/mingw64!).
    Тестери final_testers використовують fork()/pipe()/waitpid() —
    повноцінний POSIX API, доступний лише через msys-2.0.dll
    (компілятор з усього /usr/bin, а не з ucrt64/mingw64).
    """
    return os.path.join("C:\\", "msys64", "usr", "bin")


def find_msys_gpp() -> str:
    """
    Шукає g++ саме в MSYS-середовищі (з підтримкою fork). Перевіряє
    кілька стандартних місць встановлення, бо MSYS2 завжди ставиться
    в один із них.
    """
    candidates = [
        os.path.join(msys2_msys_bin_path(), "g++.exe"),
        os.path.join("C:\\", "msys64", "usr", "bin", "g++.exe"),
    ]
    for c in candidates:
        if os.path.isfile(c):
            return c
    return ""


def is_msys_gpp_available() -> bool:
    """
    Чи доступний компілятор, здатний зібрати САМ ТЕСТЕР (з fork()/waitpid()).
    На Windows — це обов'язково MSYS-g++ (C:\\msys64\\usr\\bin\\g++.exe).
    На Linux/macOS — звичайний g++ з PATH вже має повний POSIX, достатньо.
    """
    if is_windows():
        return bool(find_msys_gpp())
    return _shutil.which("g++") is not None


def install_gpp_windows_async(on_line, on_done):
    """
    Запускає встановлення MSYS2 + toolchain (g++) у фоновому процесі з
    підвищеними правами (UAC), через PowerShell.

    ВАЖЛИВО: Start-Process -Verb RunAs створює процес у НОВОМУ контексті —
    його stdout НЕ потрапляє в pipe батьківського процесу. Тому підвищений
    процес пише весь вивід у тимчасовий лог-файл (через Start-Transcript),
    а зовнішній (непідвищений, видимий для QProcess) процес лише чекає
    завершення й одним блоком віддає нам вміст файлу — який ми емітимо
    рядок за рядком через on_line.

    on_line(str) викликається для кожного рядка виводу,
    on_done(success, message) — наприкінці.
    Повертає QProcess (тримайте посилання, інакше об'єкт знищиться GC).
    """
    import tempfile, uuid

    # ВАЖЛИВО: тестери final_testers використовують fork()/waitpid() (POSIX),
    # тому потрібен компілятор саме з базового MSYS-середовища (usr/bin),
    # а НЕ з ucrt64/bin — той генерує нативні Windows-бінарники без fork().
    bin_dir = msys2_msys_bin_path()
    log_path = os.path.join(
        tempfile.gettempdir(), f"gpp_install_{uuid.uuid4().hex}.log"
    )
    # PowerShell-екранування: подвоюємо одинарні лапки всередині рядків.
    log_path_ps = log_path.replace("'", "''")
    bin_dir_ps = bin_dir.replace("'", "''")

    # Скрипт, що буде виконано З ПІДВИЩЕНИМИ ПРАВАМИ.
    # Весь вивід (включно з помилками) йде у файл через Start-Transcript,
    # тому навіть якщо щось впаде — побачимо причину в лозі.
    elevated_script = (
        f"Start-Transcript -Path '{log_path_ps}' -Force | Out-Null; "
        "try { "
        "  Write-Output 'Перевірка winget...'; "
        "  if (-not (Get-Command winget -ErrorAction SilentlyContinue)) { "
        "    Write-Output 'ПОМИЛКА: winget не знайдено. Встановіть App Installer з Microsoft Store.'; "
        "    Stop-Transcript | Out-Null; exit 1 "
        "  }; "
        "  if (-not (Test-Path 'C:\\msys64\\usr\\bin\\pacman.exe')) { "
        "    Write-Output 'Встановлення MSYS2 через winget (це може тривати кілька хвилин)...'; "
        "    winget install --id=MSYS2.MSYS2 -e --accept-source-agreements --accept-package-agreements "
        "      --disable-interactivity | ForEach-Object { Write-Output $_ }; "
        "  } else { Write-Output 'MSYS2 вже встановлено, пропускаємо.'; }; "
        "  if (-not (Test-Path 'C:\\msys64\\usr\\bin\\pacman.exe')) { "
        "    Write-Output 'ПОМИЛКА: MSYS2 не встановився (pacman.exe відсутній після winget install).'; "
        "    Stop-Transcript | Out-Null; exit 2 "
        "  }; "
        "  if (Test-Path 'C:\\msys64\\var\\lib\\pacman\\db.lck') { "
        "    Write-Output 'Знайдено застарілий lock-файл pacman — видаляю...'; "
        "    Remove-Item -Force 'C:\\msys64\\var\\lib\\pacman\\db.lck' -ErrorAction SilentlyContinue; "
        "  }; "
        "  Write-Output 'Оновлення бази пакетів MSYS2...'; "
        "  & 'C:\\msys64\\usr\\bin\\pacman.exe' -Syu --noconfirm 2>&1 | ForEach-Object { Write-Output $_ }; "
        "  if (Test-Path 'C:\\msys64\\var\\lib\\pacman\\db.lck') { "
        "    Write-Output 'Видаляю lock-файл перед встановленням toolchain...'; "
        "    Remove-Item -Force 'C:\\msys64\\var\\lib\\pacman\\db.lck' -ErrorAction SilentlyContinue; "
        "  }; "
        "  Write-Output 'Встановлення toolchain (g++, base-devel)...'; "
        "  & 'C:\\msys64\\usr\\bin\\pacman.exe' -S --noconfirm --needed base-devel gcc "
        "    2>&1 | ForEach-Object { Write-Output $_ }; "
        "  if (-not (Test-Path 'C:\\msys64\\usr\\bin\\g++.exe')) { "
        "    Write-Output 'ПОМИЛКА: g++.exe не з''явився після встановлення toolchain.'; "
        "    Stop-Transcript | Out-Null; exit 3 "
        "  }; "
        f"  Write-Output 'Додавання до PATH користувача: {bin_dir_ps}'; "
        "  $cur = [Environment]::GetEnvironmentVariable('Path', 'User'); "
        f"  if ($cur -notlike '*{bin_dir_ps}*') {{ "
        f"    [Environment]::SetEnvironmentVariable('Path', $cur + ';{bin_dir_ps}', 'User'); "
        "  }; "
        "  Write-Output 'ГОТОВО'; "
        "  Stop-Transcript | Out-Null; "
        "} catch { "
        "  Write-Output ('ВИНЯТОК: ' + $_.Exception.Message); "
        "  Stop-Transcript | Out-Null; exit 99 "
        "}"
    )
    # Кодуємо весь підвищений скрипт у Base64, щоб уникнути проблем з
    # вкладеним екрануванням лапок при передачі через -EncodedCommand.
    import base64
    encoded = base64.b64encode(elevated_script.encode("utf-16-le")).decode("ascii")

    # Зовнішній (непідвищений) скрипт: піднімає права, чекає завершення,
    # потім читає файл лога повністю (навіть якщо процес вище впав).
    outer_script = (
        "$p = Start-Process powershell -Verb RunAs -PassThru -Wait -ArgumentList "
        f"'-NoProfile','-ExecutionPolicy','Bypass','-EncodedCommand','{encoded}'; "
        f"if (Test-Path '{log_path_ps}') {{ Get-Content -Path '{log_path_ps}' -Raw }} "
        "else { Write-Output 'ПОМИЛКА: користувач відхилив запит UAC або лог-файл не створено.' }; "
        "exit $p.ExitCode"
    )

    process = QProcess()
    process.setProgram("powershell.exe")
    process.setArguments(["-NoProfile", "-ExecutionPolicy", "Bypass", "-Command", outer_script])

    def handle_output():
        data = process.readAllStandardOutput().data().decode("utf-8", errors="ignore")
        for line in data.splitlines():
            if line.strip():
                on_line(line.strip())

    def handle_finished(exit_code, _exit_status):
        # Підчищаємо лог-файл незалежно від результату.
        try:
            if os.path.isfile(log_path):
                os.remove(log_path)
        except OSError:
            pass

        if exit_code == 0:
            on_done(True, "Встановлення завершено. Натисніть 'Перевірити ще раз'.")
        elif exit_code == 1223:
            # Стандартний код Windows при відмові від UAC-запиту.
            on_done(False, "Запит підвищення прав (UAC) було відхилено.")
        else:
            on_done(False, f"Процес завершився з кодом {exit_code}. Деталі — у лозі вище.")

    process.readyReadStandardOutput.connect(handle_output)
    process.finished.connect(handle_finished)
    process.start()
    return process


def get_tester_source_path(subdir_key: str, load_filename: str, level_key: str) -> str:
    """Шлях до *_tester_multilang.cpp для конкретного алгоритму й рівня."""
    category_dir = _PROBLEM_CATEGORY_DIR.get(subdir_key)
    if category_dir is None:
        return ""
    fname = _problem_filename(subdir_key, load_filename)
    prefix = PROBLEM_LEVEL_PREFIX.get(level_key, level_key.upper())
    level_dir = PROBLEM_LEVEL_TESTER_DIR.get(level_key, "")
    return os.path.join(
        _TESTERS_BASE, level_dir, category_dir,
        f"{prefix}_{fname}_tester_multilang.cpp"
    )


def get_tester_binary_path(tester_cpp_path: str) -> str:
    """Шлях до скомпільованого бінарника поряд із вихідним .cpp (кешується)."""
    base, _ = os.path.splitext(tester_cpp_path)
    suffix = ".exe" if os.name == "nt" else ""
    return base + "_bin" + suffix


def _is_windows_store_stub(exe_path: str) -> bool:
    """
    Windows App Execution Alias — "заглушка" python.exe/python3.exe, яку
    Windows 10/11 автоматично кладе в
    %LOCALAPPDATA%\\Microsoft\\WindowsApps\\, навіть якщо справжній Python
    НЕ встановлений з Microsoft Store. Ця заглушка при запуску без
    встановленого Store-додатку видає лише текст "Python" в stderr і
    завершується з кодом 49 — НЕ є робочим інтерпретатором.
    shutil.which() легко знаходить саме її першою, бо WindowsApps зазвичай
    стоїть в PATH раніше за справжню директорію встановлення Python.
    """
    normalized = exe_path.replace("/", "\\").lower()
    return "\\appdata\\local\\microsoft\\windowsapps\\" in normalized


def find_real_python() -> str:
    """
    Шукає СПРАВЖНІЙ python.exe — не Windows Store stub.
    На відміну від shutil.which() (який повертає лише перший знайдений
    збіг і легко "натикається" на WindowsApps-заглушку), ця функція
    перебирає УСІ збіги в PATH і відкидає ті, що лежать у WindowsApps.
    Якщо в PATH нічого не лишилось — додатково перевіряє типові стандартні
    шляхи встановлення (python.org інсталятор, py launcher тощо).
    """
    import glob as _glob

    candidates = []
    for name in ("python3.exe", "python.exe") if is_windows() else ("python3", "python"):
        # shutil.which з повним переглядом PATH (не лише перший збіг):
        path_dirs = os.environ.get("PATH", "").split(os.pathsep)
        for d in path_dirs:
            cand = os.path.join(d, name)
            if os.path.isfile(cand):
                candidates.append(cand)

    # Фолбек: типові місця встановлення python.org на Windows.
    if is_windows():
        local_appdata = os.environ.get("LOCALAPPDATA", "")
        program_files = os.environ.get("ProgramFiles", "C:\\Program Files")
        glob_patterns = [
            os.path.join(local_appdata, "Programs", "Python", "Python3*", "python.exe"),
            os.path.join(program_files, "Python3*", "python.exe"),
            "C:\\Python3*\\python.exe",
        ]
        for pattern in glob_patterns:
            candidates.extend(_glob.glob(pattern))

    for c in candidates:
        if not _is_windows_store_stub(c) and os.path.isfile(c):
            return c
    return ""


def _ensure_python3_alias():
    """
    На Windows офіційний інсталятор python.org створює лише python.exe,
    не python3.exe — а final_testers/common/language.hpp викликає рівно
    "python3" (жорстко прописано в C++ коді тестера, без фолбеку на
    "python").

    ВАЖЛИВО — три пастки тут:

    1. НЕ можна просто скопіювати python.exe в інше місце — сучасний
       CPython-launcher визначає шлях до своєї стандартної бібліотеки
       (Lib/, DLLs/, python3XX.dll) ВІДНОСНО власного розташування на
       диску. Копія без супутніх файлів миттєво падає при старті.

    2. НЕ можна покласти .cmd/.bat-файл — MSYS-shell (sh.exe), через
       який тестер виконує python3 за допомогою system(), НЕ вміє
       виконувати .cmd/.bat файли напряму (це Windows cmd.exe формат,
       MSYS розуміє лише POSIX shell-скрипти/ELF-подібні PE-бінарники).

    3. shutil.which("python")/which("python3") на Windows 10/11 часто
       знаходить НЕ справжній Python, а "заглушку" (App Execution Alias)
       з %LOCALAPPDATA%\\Microsoft\\WindowsApps\\ — файл, що лише
       перенаправляє в Microsoft Store і нічого корисного не виконує.
       Тому шукаємо через find_real_python(), яка явно відкидає цей шлях.

    Правильне рішення: створюємо файл "python3" (БЕЗ розширення) —
    звичайний POSIX shell-скрипт із шебангом #!/bin/sh, який сам
    викликає cmd.exe для запуску справжнього python.exe за повним
    абсолютним шляхом. MSYS-shell виконує такі файли нативно.
    """
    if not is_windows():
        return

    msys_bin = msys2_msys_bin_path()

    # Прибираємо артефакти попередніх (помилкових) версій цієї функції,
    # якщо вони лишились від старішого запуску — інакше which("python3")
    # знайде саме їх і помилково подумає, що все гаразд.
    for stale_name in ("python3.exe", "python3.cmd"):
        stale_path = os.path.join(msys_bin, stale_name)
        if os.path.isfile(stale_path):
            try:
                os.remove(stale_path)
            except OSError:
                pass

    python_exe = find_real_python()
    if not python_exe:
        return  # немає жодного РЕАЛЬНОГО python — нічим допомогти
    if not os.path.isdir(msys_bin):
        return

    target = os.path.join(msys_bin, "python3")
    # ЗАВЖДИ перезаписуємо (а не "якщо файл відсутній") — інакше зламана
    # версія з попередньої невдалої спроби лишається назавжди й приховує
    # подальші виправлення цієї функції.
    try:
        with open(target, "w", encoding="utf-8", newline="\n") as f:
            f.write("#!/bin/sh\n")
            f.write(f'exec cmd.exe /c "{python_exe}" "$@"\n')
        os.chmod(target, 0o755)
    except OSError:
        pass  # некритично — якщо не вдалось, просто лишиться попередня помилка


def diagnose_python3_wrapper() -> str:
    """
    РЕАЛЬНО запускає обгортку python3 через MSYS-shell (sh.exe) і повертає
    детальний звіт: чи знайдено python.exe, чи створено скрипт, що саме
    sh.exe бачить і виконує. Призначено для показу користувачу при
    повторюваній помилці — щоб бачити факти, а не гіпотези.
    """
    lines = []
    if not is_windows():
        return "Діагностика доступна лише на Windows."

    python_exe = find_real_python()
    stub_exe = _shutil.which("python3") or _shutil.which("python")
    if stub_exe and _is_windows_store_stub(stub_exe):
        lines.append(f"⚠ shutil.which() знаходить WindowsApps-ЗАГЛУШКУ: {stub_exe}")
        lines.append("  (це НЕ справжній Python — App Execution Alias, перенаправляє в Store)")
    lines.append(f"Справжній python.exe (find_real_python): {python_exe or '— НЕ ЗНАЙДЕНО —'}")

    msys_bin = msys2_msys_bin_path()
    lines.append(f"Каталог MSYS usr/bin: {msys_bin} (існує: {os.path.isdir(msys_bin)})")

    sh_exe = os.path.join(msys_bin, "sh.exe")
    lines.append(f"sh.exe знайдено: {os.path.isfile(sh_exe)} ({sh_exe})")

    wrapper = os.path.join(msys_bin, "python3")
    if os.path.isfile(wrapper):
        try:
            with open(wrapper, encoding="utf-8", errors="replace") as f:
                content = f.read()
            lines.append(f"Вміст обгортки {wrapper}:\n{content}")
        except OSError as e:
            lines.append(f"Не вдалось прочитати обгортку: {e}")
    else:
        lines.append(f"Файл обгортки {wrapper} НЕ ІСНУЄ.")

    cmd_exe = _shutil.which("cmd.exe") or os.path.join(
        os.environ.get("SystemRoot", "C:\\Windows"), "System32", "cmd.exe"
    )
    lines.append(f"cmd.exe: {cmd_exe} (існує: {os.path.isfile(cmd_exe)})")

    # Реальний тестовий запуск: sh.exe -c "python3 --version" з тим самим env,
    # яке отримає справжній тестер.
    if os.path.isfile(sh_exe) and os.path.isfile(wrapper):
        import subprocess
        try:
            env = _msys_subprocess_env()
            test_proc = subprocess.run(
                [sh_exe, "-c", "python3 --version"],
                env=env, capture_output=True,
                encoding="utf-8", errors="replace", timeout=15
            )
            lines.append(
                f"Тестовий запуск 'sh.exe -c \"python3 --version\"':\n"
                f"  exit code: {test_proc.returncode}\n"
                f"  stdout: {test_proc.stdout!r}\n"
                f"  stderr: {test_proc.stderr!r}"
            )
        except Exception as e:
            lines.append(f"Тестовий запуск провалився з винятком: {e}")
    else:
        lines.append("Тестовий запуск пропущено (sh.exe або обгортка відсутні).")

    return "\n".join(lines)


def _strip_windows_apps_from_path(path_str: str) -> str:
    """
    Видаляє з рядка PATH усі записи, що ведуть у
    %LOCALAPPDATA%\\Microsoft\\WindowsApps — саме там лежать App Execution
    Alias заглушки python.exe/python3.exe/node.exe тощо, які при викликові
    БЕЗ аргументів відкривають Microsoft Store, а З аргументами завершуються
    кодом 49 і виводять лише назву програми в stderr. Зберігати цей запис
    у PATH для дочірнього процесу шкідливо: він "перехоплює" виклик команди
    раніше, ніж дійде до справжнього інтерпретатора, навіть коли справжній
    теж є в PATH (просто пізніше).
    """
    parts = path_str.split(os.pathsep)
    kept = [p for p in parts if "\\appdata\\local\\microsoft\\windowsapps" not in p.lower()]
    return os.pathsep.join(kept)


def _msys_subprocess_env() -> dict:
    """
    Будує середовище (env) для запуску MSYS-бінарників (g++, cc1plus тощо)
    і для самого скомпільованого тестера (який всередині себе через system()
    викликає python3/node/javac тощо для перевірки/запуску розв'язку
    користувача іншими мовами).

    Кілька накладених проблем тут вирішуються:

    1. cc1plus.exe (внутрішній компілятор, який запускає g++.exe) шукає свої
       залежні DLL (msys-2.0.dll, msys-isl-*.dll тощо) ЧЕРЕЗ PATH поточного
       процесу — абсолютний шлях до самого g++.exe для цього недостатній.

    2. Тестер викликає python3/node/javac через system(), що запускає
       MSYS-shell (sh.exe). За замовчуванням той бачить ЛИШЕ свій внутрішній
       PATH (/usr/bin, /bin), тому потрібен MSYS2_PATH_TYPE=inherit, щоб
       успадкувати повний Windows PATH (конвертований у POSIX-стиль).

    3. ПАСТКА: MSYS2_PATH_TYPE=inherit сам перебудовує internal PATH
       MSYS-shell з Windows PATH "як є" — включно з
       %LOCALAPPDATA%\\Microsoft\\WindowsApps, де лежить App Execution
       Alias заглушка python3.exe. Якщо цей запис опиняється в PATH РАНІШЕ
       за наш C:\\msys64\\usr\\bin\\python3 (обгортку), MSYS-shell виконує
       заглушку замість нашої обгортки навіть при простому "python3 ...".
       Тому ПРИБИРАЄМО WindowsApps із PATH ще на Python-рівні, перед тим
       як MSYS-shell взагалі його побачить — це не залежить від порядку
       додавання інших каталогів і гарантовано усуває перехоплення.
    """
    env = os.environ.copy()
    if is_windows():
        _ensure_python3_alias()
        msys_bin = msys2_msys_bin_path()
        cur_path = _strip_windows_apps_from_path(env.get("PATH", ""))
        if msys_bin not in cur_path:
            env["PATH"] = msys_bin + os.pathsep + cur_path
        else:
            env["PATH"] = cur_path
        env["MSYS2_PATH_TYPE"] = "inherit"
    return env


def compile_tester(tester_cpp_path: str) -> tuple:
    """
    Компілює тестер (якщо ще не скомпільований або вихідник новіший за бінарник).
    Повертає (bin_path або None, error_message або "").

    На Windows тестер ПОВИНЕН компілюватись MSYS-компілятором (з базового
    /usr/bin, не ucrt64/mingw64) — final_testers/common/tester_common.hpp
    використовує fork()/pipe()/waitpid(), а це повноцінний POSIX API,
    доступний лише через msys-2.0.dll. Звичайний MinGW/UCRT64 g++ збирає
    нативні Windows-бінарники без fork() і впаде на #include <sys/wait.h>.
    """
    if not os.path.isfile(tester_cpp_path):
        return None, f"Файл тестера не знайдено: {tester_cpp_path}"

    bin_path = get_tester_binary_path(tester_cpp_path)
    need_build = (
        not os.path.isfile(bin_path)
        or os.path.getmtime(tester_cpp_path) > os.path.getmtime(bin_path)
    )
    if not need_build:
        return bin_path, ""

    gpp_cmd = "g++"
    if is_windows():
        msys_gpp = find_msys_gpp()
        if msys_gpp:
            gpp_cmd = msys_gpp
        else:
            return None, (
                "Компілятор g++ з підтримкою fork() (потрібен для тестерів) не знайдено.\n"
                "Очікувався шлях: C:\\msys64\\usr\\bin\\g++.exe\n"
                "Звичайний MinGW/UCRT64 g++ не підійде — final_testers використовує "
                "POSIX fork()/waitpid(), якого немає в ucrt64/mingw64 збірці.\n"
                "Встановіть пакет 'gcc' у базовому MSYS-середовищі: "
                "pacman -S --needed base-devel gcc"
            )

    import subprocess
    try:
        proc = subprocess.run(
            [
                gpp_cmd, "-O2", "-std=c++17",
                # _GNU_SOURCE розкриває POSIX/GNU-розширення (kill, usleep,
                # realpath) у заголовках MSYS-newlib unistd.h/signal.h —
                # без цього прапора вони не оголошуються в строгому
                # ISO C++17 режимі компіляції.
                "-D_GNU_SOURCE",
                "-o", bin_path, tester_cpp_path
            ],
            cwd=os.path.dirname(tester_cpp_path),
            env=_msys_subprocess_env(),
            capture_output=True,
            encoding="utf-8", errors="replace",
            timeout=120
        )
    except FileNotFoundError:
        return None, "Компілятор g++ не знайдено в PATH. Встановіть MinGW/MSYS2 (Windows) або build-essential (Linux)."
    except subprocess.TimeoutExpired:
        return None, "Перевищено час компіляції тестера."
    except Exception as e:
        return None, f"Помилка запуску компілятора: {e}"

    if proc.returncode != 0:
        return None, f"Помилка компіляції тестера:\n{proc.stderr}"
    return bin_path, ""


# Парсить рядки виду: "Тест   1/100  [OK]  час=12мс"
_TESTER_LINE_RE = re.compile(r"Тест\s+(\d+)/(\d+)\s+\[(\w+)\]\s+час=(\d+)мс")
_TESTER_SUMMARY_RE = re.compile(r"РЕЗУЛЬТАТ:\s*(\w+)")
_TESTER_CE_RE = re.compile(r"^\[CE\]")
# TESTDATA <idx> <input_b64> <expected_b64> <actual_b64> — друкується
# тестером (tester_main_multilang.inc) у блоці ===TESTDATA-BEGIN/END===.
_TESTDATA_RE = re.compile(r"^TESTDATA (\d+) (\S+) (\S+) (\S+)$", re.MULTILINE)


def run_tester(bin_path: str, solution_path: str, timeout_s: int = 60) -> dict:
    """
    Запускає скомпільований мультимовний тестер на конкретному розв'язку.
    Повертає словник:
      {
        "ok": bool,                 # чи вдалося взагалі запустити (без CE/системних помилок)
        "verdict": str,             # "ACCEPTED" / "REJECTED" / "CE" / "ERROR"
        "tests": [(idx, total, verdict, time_ms), ...],
        "test_data": {idx: {"input": str, "expected": str, "actual": str}, ...},
        "raw": str,                 # повний stdout (для діагностики/ШІ)
        "error": str,               # повідомлення про помилку, якщо ok=False
      }
    """
    import subprocess, base64
    result = {"ok": False, "verdict": "ERROR", "tests": [], "test_data": {}, "raw": "", "error": ""}

    if not os.path.isfile(bin_path):
        result["error"] = f"Бінарник тестера не знайдено: {bin_path}"
        return result
    if not os.path.isfile(solution_path):
        result["error"] = f"Файл розв'язку не знайдено: {solution_path}"
        return result

    try:
        proc = subprocess.run(
            [bin_path, solution_path],
            cwd=os.path.dirname(bin_path),
            env=_msys_subprocess_env(),
            capture_output=True,
            # Явно UTF-8: на Windows text=True без encoding бере системну
            # кодову сторінку (CP1251/CP866), а тестер і вихідні файли —
            # UTF-8. Без цього кирилиця в виводі перетворюється на "крякозябри".
            encoding="utf-8", errors="replace",
            timeout=timeout_s
        )
    except subprocess.TimeoutExpired:
        result["error"] = "Перевищено загальний час тестування (можливе зависання розв'язку)."
        return result
    except Exception as e:
        result["error"] = f"Помилка запуску тестера: {e}"
        return result

    out = proc.stdout or ""
    # КРИТИЧНО: на Windows вивід тестера (через MSYS/pipe) міг прийти з
    # CRLF (\r\n) навіть попри text-режим subprocess.run. Це ламає regex
    # ^TESTDATA ... $ у MULTILINE-режимі: \S не матчить \r, тож зайвий \r
    # перед \n залишається "висячим" символом, і весь рядок TESTDATA не
    # проходить парсинг — саме це давало "дані для тесту не знайдено"
    # лише для частини тестів (тих, що опинились після проблемного місця
    # у виводі). Нормалізуємо явно, незалежно від причини появи \r.
    out = out.replace("\r\n", "\n").replace("\r", "\n")
    result["raw"] = out + ("\n" + proc.stderr if proc.stderr else "")

    if _TESTER_CE_RE.search(out):
        result["verdict"] = "CE"
        result["ok"] = True  # запуск відбувся, просто результат — помилка компіляції розв'язку
        return result

    tests = []
    for m in _TESTER_LINE_RE.finditer(out):
        idx, total, verdict, time_ms = m.groups()
        tests.append((int(idx), int(total), verdict, int(time_ms)))
    result["tests"] = tests

    # Парсимо блок з вхідними/очікуваними/фактичними даними кожного тесту.
    # C++ тестер замінює порожній base64 (коли вивід/вхід справді порожній)
    # на маркер "EMPTY" — без цього рядок мав би менше за 4 токени і regex
    # нижче взагалі не матчив би його (саме це й ламало парсинг WA-тестів,
    # де rr.output часто порожній). Тут перетворюємо маркер назад.
    def _decode_field(b64_token: str) -> str:
        if b64_token == "EMPTY":
            return ""
        return base64.b64decode(b64_token).decode("utf-8", errors="replace")

    test_data = {}
    for m in _TESTDATA_RE.finditer(out):
        idx_s, in_b64, exp_b64, act_b64 = m.groups()
        idx = int(idx_s)
        try:
            test_data[idx] = {
                "input":    _decode_field(in_b64),
                "expected": _decode_field(exp_b64),
                "actual":   _decode_field(act_b64),
            }
        except Exception:
            pass  # некоректний base64 — пропускаємо цей тест, інші лишаються доступні
    result["test_data"] = test_data

    summary_m = _TESTER_SUMMARY_RE.search(out)
    result["verdict"] = summary_m.group(1) if summary_m else ("ERROR" if not tests else "UNKNOWN")
    result["ok"] = True
    return result



def _load(subdir_key: str, filename: str) -> str:
    """
    Читає txt-файл із папки коротких описів (для першої картки).
    subdir_key — ключ з _SD_DIRS (напр. 'search').
    filename   — ім'я файлу БЕЗ розширення (напр. 'linear_search').
    Повертає вміст файлу або повідомлення про помилку.
    """
    path = os.path.join(_SD_DIRS[subdir_key], filename + ".txt")
    try:
        with open(path, encoding="utf-8") as f:
            return f.read().strip()
    except FileNotFoundError:
        return f"[Файл не знайдено: {path}]"
    except Exception as e:
        return f"[Помилка читання: {e}]"


def _load_detail(subdir_key: str, filename: str) -> str:
    """
    Читає txt-файл із папки детальних описів (для другої картки — "Детальніше").
    subdir_key — ключ з _MD_DIRS (напр. 'search').
    filename   — ім'я файлу БЕЗ розширення (напр. 'linear_search').
    """
    path = os.path.join(_MD_DIRS[subdir_key], filename + ".txt")
    try:
        with open(path, encoding="utf-8") as f:
            return f.read().strip()
    except FileNotFoundError:
        return f"[Файл не знайдено: {path}]"
    except Exception as e:
        return f"[Помилка читання: {e}]"


# ══════════════════════════════════════════════════════════════════
#  ✏️  ДАНІ РІВНІВ — редагуй кожен блок окремо
#
#  Формат кожного рядка списку:
#  ( "Назва кнопки", "Заголовок картки", "Текст картки", "Текст деталей" )
#
#  ROW0 — верхній ряд (5 кнопок)
#  ROW1 — нижній ряд (4 кнопки)
# ══════════════════════════════════════════════════════════════════

# ──────────────────────────────────────────────
#  EASY
# ──────────────────────────────────────────────
# ══════════════════════════════════════════════════════════════════
#  ДАНІ РІВНІВ — формат: (назва кнопки, заголовок картки, текст картки, текст деталей)
# ══════════════════════════════════════════════════════════════════

# ──────────────────────────────────────────────
#  EASY — Базові (найпростіші)
# ──────────────────────────────────────────────
EASY_ROW0 = [
    ("Лінійний пошук",
     "Лінійний пошук (Linear Search)",
     _load("search", "linear_search"),
     _load_detail("search", "linear_search"), ("search", "linear_search")),

    ("Бінарний пошук",
     "Бінарний пошук (Binary Search)",
     _load("search", "binary_search"),
     _load_detail("search", "binary_search"), ("search", "binary_search")),

    ("Сортування вибором",
     "Сортування вибором (Selection Sort)",
     _load("sort", "selection_sort"),
     _load_detail("sort", "selection_sort"), ("sort", "selection_sort")),

    ("Сортування вставками",
     "Сортування вставками (Insertion Sort)",
     _load("sort", "insertion_sort"),
     _load_detail("sort", "insertion_sort"), ("sort", "insertion_sort")),

    ("Метод двох вказівників",
     "Метод двох вказівників (Two Pointers)",
     _load("search", "two_pointers"),
     _load_detail("search", "two_pointers"), ("search", "two_pointers")),
]
EASY_ROW1 = [
    ("Ковзне вікно",
     "Ковзне вікно (Sliding Window)",
     _load("search", "sliding_window"),
     _load_detail("search", "sliding_window"), ("search", "sliding_window")),

    ("Префіксні суми",
     "Префіксні суми (Prefix Sums)",
     _load("search", "prefix_sums"),
     _load_detail("search", "prefix_sums"), ("search", "prefix_sums")),

    ("Сортування підрахунком",
     "Сортування підрахунком (Counting Sort)",
     _load("sort", "counting_sort"),
     _load_detail("sort", "counting_sort"), ("sort", "counting_sort")),

    ("Алгоритм Евкліда",
     "Алгоритм Евкліда (НСД, GCD)",
     _load("math", "euclidean_gcd"),
     _load_detail("math", "euclidean_gcd"), ("math", "euclidean_gcd")),
]

# ──────────────────────────────────────────────
#  MEDIUM — Середній рівень
# ──────────────────────────────────────────────
MEDIUM_ROW0 = [
    ("Швидке сортування",
     "Швидке сортування (Quick Sort)",
     _load("sort", "quick_sort"),
     _load_detail("sort", "quick_sort"), ("sort", "quick_sort")),

    ("Сортування злиттям",
     "Сортування злиттям (Merge Sort)",
     _load("sort", "merge_sort"),
     _load_detail("sort", "merge_sort"), ("sort", "merge_sort")),

    ("Пірамідальне сортування",
     "Пірамідальне сортування (Heap Sort)",
     _load("sort", "heap_sort"),
     _load_detail("sort", "heap_sort"), ("sort", "heap_sort")),

    ("BFS",
     "Обхід графа в ширину (BFS)",
     _load("graphs", "bfs"),
     _load_detail("graphs", "bfs"), ("graphs", "bfs")),

    ("DFS",
     "Обхід графа в глибину (DFS)",
     _load("graphs", "dfs"),
     _load_detail("graphs", "dfs"), ("graphs", "dfs")),
]
MEDIUM_ROW1 = [
    ("Решето Ератосфена",
     "Решето Ератосфена (Sieve of Eratosthenes)",
     _load("math", "sieve_of_eratosthenes"),
     _load_detail("math", "sieve_of_eratosthenes"), ("math", "sieve_of_eratosthenes")),

    ("Бінарне піднесення",
     "Швидке піднесення до степеня (Binary Exponentiation)",
     _load("math", "binary_exponentiation"),
     _load_detail("math", "binary_exponentiation"), ("math", "binary_exponentiation")),

    ("Жадібні алгоритми",
     "Жадібні алгоритми (Greedy)",
     _load("greedy", "greedy_algorithms"),
     _load_detail("greedy", "greedy_algorithms"), ("greedy", "greedy_algorithms")),

    ("Union-Find / DSU",
     "Система неперетинних множин (Union-Find / DSU)",
     _load("trees", "disjoint_set_union"),
     _load_detail("trees", "disjoint_set_union"), ("trees", "disjoint_set_union")),
]

# ──────────────────────────────────────────────
#  HARD — Просунутий рівень
# ──────────────────────────────────────────────
HARD_ROW0 = [
    ("Алгоритм Дейкстри",
     "Алгоритм Дейкстри (найкоротший шлях)",
     _load("graphs", "dijkstra"),
     _load_detail("graphs", "dijkstra"), ("graphs", "dijkstra")),

    ("Беллман-Форд",
     "Алгоритм Беллмана-Форда",
     _load("graphs", "bellman_ford"),
     _load_detail("graphs", "bellman_ford"), ("graphs", "bellman_ford")),

    ("Флойд-Уоршелл",
     "Алгоритм Флойда-Уоршелла",
     _load("graphs", "floyd_warshall"),
     _load_detail("graphs", "floyd_warshall"), ("graphs", "floyd_warshall")),

    ("Топологічне сортування",
     "Топологічне сортування (Topological Sort)",
     _load("graphs", "topological_sort"),
     _load_detail("graphs", "topological_sort"), ("graphs", "topological_sort")),

    ("Краскал",
     "Мінімальне кістякове дерево — Краскал (Kruskal)",
     _load("greedy", "kruskal_mst"),
     _load_detail("greedy", "kruskal_mst"), ("greedy", "kruskal_mst")),
]
HARD_ROW1 = [
    ("Прім",
     "Мінімальне кістякове дерево — Прім (Prim)",
     _load("greedy", "prim_mst"),
     _load_detail("greedy", "prim_mst"), ("greedy", "prim_mst")),

    ("Bitmask DP",
     "ДП на підмножинах / бітові маски (Bitmask DP)",
     _load("dynamic", "bitmask_dp"),
     _load_detail("dynamic", "bitmask_dp"), ("dynamic", "bitmask_dp")),

    ("LIS",
     "Найдовша зростаюча підпослідовність (LIS)",
     _load("dynamic", "longest_increasing_subsequence"),
     _load_detail("dynamic", "longest_increasing_subsequence"), ("dynamic", "longest_increasing_subsequence")),

    ("Алгоритм Кадане",
     "Алгоритм Кадане (максимальна сума підмасиву)",
     _load("search", "kadane"),
     _load_detail("search", "kadane"), ("search", "kadane")),
]

# ──────────────────────────────────────────────
#  INSANE — Спеціалізовані
# ──────────────────────────────────────────────
INSANE_ROW0 = [
    ("Segment Tree",
     "Дерево відрізків (Segment Tree)",
     _load("trees", "segment_tree"),
     _load_detail("trees", "segment_tree"), ("trees", "segment_tree")),

    ("Fenwick Tree / BIT",
     "Дерево Фенвіка (Fenwick Tree / BIT)",
     _load("trees", "fenwick_tree"),
     _load_detail("trees", "fenwick_tree"), ("trees", "fenwick_tree")),

    ("KMP",
     "Алгоритм Кнута-Морріса-Пратта (KMP)",
     _load("string", "kmp"),
     _load_detail("string", "kmp"), ("string", "kmp")),

    ("Рабін-Карп",
     "Алгоритм Рабіна-Карпа (хешування рядків)",
     _load("string", "rabin_karp"),
     _load_detail("string", "rabin_karp"), ("string", "rabin_karp")),

    ("Z-функція",
     "Z-функція (Z-algorithm)",
     _load("string", "z_function"),
     _load_detail("string", "z_function"), ("string", "z_function")),
]
INSANE_ROW1 = [
    ("LCA",
     "Найменший спільний предок (LCA, binary lifting)",
     _load("trees", "lowest_common_ancestor"),
     _load_detail("trees", "lowest_common_ancestor"), ("trees", "lowest_common_ancestor")),

    ("Максимальний потік",
     "Максимальний потік — Форд-Фалкерсон / Едмондс-Карп",
     _load("graphs", "max_flow"),
     _load_detail("graphs", "max_flow"), ("graphs", "max_flow")),

    ("SCC Tarjan/Kosaraju",
     "Сильно зв'язні компоненти (Tarjan / Kosaraju)",
     _load("graphs", "strongly_connected_components"),
     _load_detail("graphs", "strongly_connected_components"), ("graphs", "strongly_connected_components")),

    ("Trie",
     "Префіксне дерево (Trie)",
     _load("trees", "trie"),
     _load_detail("trees", "trie"), ("trees", "trie")),
]

# ──────────────────────────────────────────────
#  КОЛЬОРИ
# ══════════════════════════════════════════════
COLORS = {
    "bg_main":           "#070d18",
    "bg_titlebar":       "#0a1220",
    "bg_button":         "#0f1f36",
    "bg_btn_hover":      "#1a3a6b",
    "fg_text":           "#a8bcd8",
    "fg_title":          "#c8d8f0",
    "accent":            "#1e4080",
    "close_bg":          "#5a0a0a",
    "close_hover":       "#8b1a1a",
    "border":            "#102040",
    "separator":         "#0d1a30",
    "overlay_bg":        "#050b14",
    "card_bg":           "#0c1a2e",
    "card_border":       "#1e4080",
    "card_btn1_bg":      "#0f3060",
    "card_btn1_hover":   "#1a4a8a",
    "card_btn1_border":  "#2a60b0",
    "card_btn2_bg":      "#1a3a6b",
    "card_btn2_hover":   "#2050a0",
    "card_btn2_border":  "#1e4080",
    "scroll_bg":         "#081526",
    "scroll_handle":     "#1e4080",
    "scroll_handle_hov": "#2a5aaa",
    # title screen extras
    "title_btn_primary":        "#1a3a6b",
    "title_btn_primary_hover":  "#2050a0",
    "title_btn_primary_border": "#2a60b0",
    "title_btn_small":          "#0f1f36",
    "title_btn_small_hover":    "#1a3a6b",
    "title_btn_small_border":   "#1e4080",
}

# Кольори вердиктів у списку результатів тестування (TaskScreen):
# OK — зелений, WA (неправильна відповідь) — червоний,
# решта (TL, RE, інше) — жовтий.
COLORS_VERDICT_OK    = "#5fd97a"
COLORS_VERDICT_WA    = "#ff6b6b"
COLORS_VERDICT_OTHER = "#ffd866"

STYLESHEET = f"""
    QMainWindow, #centralWidget {{
        background-color: {COLORS['bg_main']};
    }}
    #titleBar {{
        background-color: {COLORS['bg_titlebar']};
        border-bottom: 1px solid {COLORS['separator']};
    }}
    #appTitle {{
        color: {COLORS['fg_title']};
        font-family: 'Segoe UI', sans-serif;
        font-size: 13px;
        font-weight: bold;
        padding-left: 12px;
        letter-spacing: 1px;
        background: transparent;
    }}
    #sysBtn {{
        background-color: {COLORS['accent']};
        color: {COLORS['fg_title']};
        border: none;
        border-radius: 4px;
        font-size: 13px;
        font-weight: bold;
        min-width: 36px; max-width: 36px;
        min-height: 28px; max-height: 28px;
        margin: 4px 2px;
        font-family: 'Segoe UI', sans-serif;
    }}
    #sysBtn:hover {{ background-color: {COLORS['bg_btn_hover']}; }}
    #closeBtn {{
        background-color: {COLORS['close_bg']};
        color: {COLORS['fg_title']};
        border: none;
        border-radius: 4px;
        font-size: 13px;
        font-weight: bold;
        min-width: 36px; max-width: 36px;
        min-height: 28px; max-height: 28px;
        margin: 4px 6px 4px 2px;
        font-family: 'Segoe UI', sans-serif;
    }}
    #closeBtn:hover {{ background-color: {COLORS['close_hover']}; }}
    #contentArea {{ background-color: {COLORS['bg_main']}; }}
    #mainBtn {{
        background-color: {COLORS['bg_button']};
        color: {COLORS['fg_text']};
        border: 1px solid {COLORS['border']};
        border-radius: 8px;
        font-size: 13px;
        font-family: 'Segoe UI', sans-serif;
        padding: 0px;
        letter-spacing: 0.5px;
        text-align: center;
    }}
    #mainBtn:hover {{
        background-color: {COLORS['bg_btn_hover']};
        border: 1px solid {COLORS['accent']};
        color: {COLORS['fg_title']};
    }}
    #mainBtn:pressed {{ background-color: {COLORS['accent']}; }}

    #overlayBg {{ background-color: transparent; }}

    #card {{
        background-color: {COLORS['card_bg']};
        border: 1px solid {COLORS['card_border']};
        border-radius: 14px;
    }}
    #cardTitle {{
        color: {COLORS['fg_title']};
        font-family: 'Segoe UI', sans-serif;
        font-size: 20px;
        font-weight: bold;
        letter-spacing: 0.5px;
        background: transparent;
        padding: 0px;
    }}
    #cardText {{
        color: {COLORS['fg_text']};
        font-family: 'Segoe UI', sans-serif;
        font-size: 16px;
        background: transparent;
        padding: 0px;
    }}
    #cardDivider {{
        background-color: {COLORS['border']};
        max-height: 1px;
        min-height: 1px;
    }}
    #cardBtnInfo {{
        background-color: {COLORS['card_btn1_bg']};
        color: {COLORS['fg_title']};
        border: 1px solid {COLORS['card_btn1_border']};
        border-radius: 7px;
        font-size: 15px;
        font-family: 'Segoe UI', sans-serif;
        font-weight: bold;
        padding: 8px 22px 8px 16px;
        letter-spacing: 0.4px;
    }}
    #cardBtnInfo:hover {{
        background-color: {COLORS['card_btn1_hover']};
        border: 1px solid {COLORS['bg_btn_hover']};
    }}
    #cardBtnInfo:pressed {{ background-color: {COLORS['accent']}; }}
    #cardBtnPlay {{
        background-color: {COLORS['card_btn2_bg']};
        color: {COLORS['fg_title']};
        border: 1px solid {COLORS['card_btn2_border']};
        border-radius: 7px;
        font-size: 15px;
        font-family: 'Segoe UI', sans-serif;
        font-weight: bold;
        padding: 8px 22px 8px 16px;
        letter-spacing: 0.4px;
    }}
    #cardBtnPlay:hover {{
        background-color: {COLORS['card_btn2_hover']};
        border: 1px solid {COLORS['bg_btn_hover']};
    }}
    #cardBtnPlay:pressed {{ background-color: {COLORS['accent']}; }}
    QScrollBar:vertical {{
        background: {COLORS['scroll_bg']};
        width: 6px;
        border-radius: 3px;
        margin: 0px;
    }}
    QScrollBar::handle:vertical {{
        background: {COLORS['scroll_handle']};
        border-radius: 3px;
        min-height: 24px;
    }}
    QScrollBar::handle:vertical:hover {{
        background: {COLORS['scroll_handle_hov']};
    }}
    QScrollBar::add-line:vertical,
    QScrollBar::sub-line:vertical {{ height: 0px; }}
    QScrollBar::add-page:vertical,
    QScrollBar::sub-page:vertical {{ background: transparent; }}
    #scrollArea {{
        background: transparent;
        border: none;
    }}
    #scrollContent {{
        background: transparent;
    }}

    /* ── Title screen buttons ── */
    #titleBtnPrimary {{
        background-color: {COLORS['title_btn_primary']};
        color: {COLORS['fg_title']};
        border: 1px solid {COLORS['title_btn_primary_border']};
        border-radius: 12px;
        font-size: 20px;
        font-family: 'Segoe UI', sans-serif;
        font-weight: bold;
        letter-spacing: 0.6px;
    }}
    #titleBtnPrimary:hover {{
        background-color: {COLORS['title_btn_primary_hover']};
        border: 1px solid {COLORS['bg_btn_hover']};
        color: #e0ecff;
    }}
    #titleBtnPrimary:pressed {{
        background-color: {COLORS['accent']};
    }}
    #titleBtnSmall {{
        background-color: {COLORS['title_btn_small']};
        color: {COLORS['fg_text']};
        border: 1px solid {COLORS['title_btn_small_border']};
        border-radius: 9px;
        font-size: 15px;
        font-family: 'Segoe UI', sans-serif;
        font-weight: bold;
        letter-spacing: 0.4px;
    }}
    #titleBtnSmall:hover {{
        background-color: {COLORS['title_btn_small_hover']};
        border: 1px solid {COLORS['title_btn_primary_border']};
        color: {COLORS['fg_title']};
    }}
    #titleBtnSmall:pressed {{
        background-color: {COLORS['accent']};
    }}
    #titleScreenLabel {{
        color: {COLORS['fg_title']};
        font-family: 'Segoe UI', sans-serif;
        font-size: 42px;
        font-weight: bold;
        letter-spacing: 3px;
        background: transparent;
    }}
    #titleScreenArea {{
        background-color: {COLORS['bg_main']};
    }}

    /* ── Level screen nav buttons ── */
    #levelNavBtn {{
        background-color: {COLORS['bg_button']};
        color: {COLORS['fg_text']};
        border: 1px solid {COLORS['border']};
        border-radius: 8px;
        font-size: 13px;
        font-family: 'Segoe UI', sans-serif;
        font-weight: bold;
        letter-spacing: 0.4px;
        padding: 0px 10px;
    }}
    #levelNavBtn:hover {{
        background-color: {COLORS['bg_btn_hover']};
        border: 1px solid {COLORS['accent']};
        color: {COLORS['fg_title']};
    }}
    #levelNavBtn:pressed {{ background-color: {COLORS['accent']}; }}

    #levelTitleLabel {{
        color: {COLORS['fg_title']};
        font-family: 'Segoe UI', sans-serif;
        font-size: 26px;
        font-weight: bold;
        letter-spacing: 2px;
        background: transparent;
    }}
    #levelTitleBig {{
        color: {COLORS['fg_title']};
        font-family: 'Segoe UI', sans-serif;
        font-size: 72px;
        font-weight: bold;
        letter-spacing: 6px;
        background: transparent;
    }}

    /* ── Task screen ── */
    #taskScreen {{
        background-color: {COLORS['bg_main']};
    }}
    #taskNavBtn {{
        background-color: {COLORS['bg_button']};
        color: {COLORS['fg_text']};
        border: 1px solid {COLORS['border']};
        border-radius: 8px;
        font-size: 13px;
        font-family: 'Segoe UI', sans-serif;
        font-weight: bold;
        letter-spacing: 0.4px;
        padding: 0px 10px;
    }}
    #taskNavBtn:hover {{
        background-color: {COLORS['bg_btn_hover']};
        border: 1px solid {COLORS['accent']};
        color: {COLORS['fg_title']};
    }}
    #taskNavBtn:pressed {{ background-color: {COLORS['accent']}; }}
    #taskLevelBtn {{
        background-color: {COLORS['bg_button']};
        color: {COLORS['fg_text']};
        border: 1px solid {COLORS['border']};
        border-radius: 8px;
        font-size: 13px;
        font-family: 'Segoe UI', sans-serif;
        font-weight: bold;
        letter-spacing: 0.4px;
        padding: 0px 18px;
    }}
    #taskLevelBtn:hover {{
        background-color: {COLORS['bg_btn_hover']};
        border: 1px solid {COLORS['accent']};
        color: {COLORS['fg_title']};
    }}
    #taskLevelBtn:checked {{
        background-color: {COLORS['accent']};
        border: 1px solid {COLORS['scroll_handle_hov']};
        color: {COLORS['fg_title']};
    }}
    #taskPanel {{
        background-color: {COLORS['card_bg']};
        border: 1px solid {COLORS['border']};
        border-radius: 10px;
    }}
    #taskDescBrowser {{
        color: {COLORS['fg_text']};
        font-family: 'Segoe UI', sans-serif;
        font-size: 15px;
        background: transparent;
        border: none;
        padding: 8px;
    }}
    #taskEditor {{
        color: {COLORS['fg_text']};
        font-family: 'Consolas', 'Courier New', monospace;
        font-size: 14px;
        background-color: {COLORS['bg_main']};
        border: 1px solid {COLORS['border']};
        border-radius: 8px;
        padding: 8px;
        selection-background-color: {COLORS['accent']};
    }}
    #taskListWidget {{
        background-color: {COLORS['bg_main']};
        border: 1px solid {COLORS['border']};
        border-radius: 8px;
        color: {COLORS['fg_text']};
        font-family: 'Segoe UI', sans-serif;
        font-size: 14px;
        outline: none;
    }}
    #taskListWidget::item {{
        padding: 10px 14px;
        border-bottom: 1px solid {COLORS['separator']};
        min-height: 36px;
    }}
    #taskListWidget::item:selected {{
        background-color: {COLORS['accent']};
        color: {COLORS['fg_title']};
    }}
    #taskListWidget::item:hover {{
        background-color: {COLORS['bg_btn_hover']};
    }}
    #taskSubmitBtn {{
        background-color: {COLORS['card_btn1_bg']};
        color: {COLORS['fg_title']};
        border: 1px solid {COLORS['card_btn1_border']};
        border-radius: 8px;
        font-size: 15px;
        font-family: 'Segoe UI', sans-serif;
        font-weight: bold;
        letter-spacing: 0.5px;
    }}
    #taskSubmitBtn:hover {{
        background-color: {COLORS['card_btn1_hover']};
        border: 1px solid {COLORS['bg_btn_hover']};
    }}
    #taskSubmitBtn:pressed {{ background-color: {COLORS['accent']}; }}
    #taskAiBtn {{
        background-color: {COLORS['card_btn2_bg']};
        color: {COLORS['fg_title']};
        border: 1px solid {COLORS['card_btn2_border']};
        border-radius: 8px;
        font-size: 15px;
        font-family: 'Segoe UI', sans-serif;
        font-weight: bold;
        letter-spacing: 0.5px;
    }}
    #taskAiBtn:hover {{
        background-color: {COLORS['card_btn2_hover']};
        border: 1px solid {COLORS['bg_btn_hover']};
    }}
    #taskAiBtn:pressed {{ background-color: {COLORS['accent']}; }}
    #taskUploadBtn {{
        background-color: {COLORS['bg_button']};
        color: {COLORS['fg_text']};
        border: 1px solid {COLORS['border']};
        border-radius: 7px;
        font-size: 13px;
        font-family: 'Segoe UI', sans-serif;
        font-weight: bold;
        padding: 0px 14px;
    }}
    #taskUploadBtn:hover {{
        background-color: {COLORS['bg_btn_hover']};
        border: 1px solid {COLORS['accent']};
        color: {COLORS['fg_title']};
    }}
    #taskUploadBtn:pressed {{ background-color: {COLORS['accent']}; }}
    #taskEditorBtn {{
        background-color: {COLORS['card_btn2_bg']};
        color: {COLORS['fg_title']};
        border: 1px solid {COLORS['card_btn2_border']};
        border-radius: 7px;
        font-size: 14px;
        font-family: 'Segoe UI', sans-serif;
        font-weight: bold;
        padding: 0px 18px;
        letter-spacing: 0.4px;
        min-width: 100px;
    }}
    #taskEditorBtn:hover {{
        background-color: {COLORS['card_btn2_hover']};
        border: 1px solid {COLORS['bg_btn_hover']};
        color: #e8f0ff;
    }}
    #taskEditorBtn:pressed {{ background-color: {COLORS['accent']}; }}
    #taskToggleBtn {{
        background-color: {COLORS['card_btn1_bg']};
        border: 1px solid {COLORS['card_btn1_border']};
        border-radius: 8px;
    }}
    #taskToggleBtn:hover {{
        background-color: {COLORS['card_btn1_hover']};
        border: 1px solid {COLORS['bg_btn_hover']};
    }}
    #taskToggleBtn:pressed {{ background-color: {COLORS['accent']}; }}
    #algoBtnMain {{
        color: {COLORS['fg_title']};
        font-family: 'Segoe UI', sans-serif;
        font-size: 13px;
        font-weight: bold;
        background: transparent;
        letter-spacing: 0.3px;
    }}
    #algoBtnSub {{
        color: {COLORS['fg_text']};
        font-family: 'Segoe UI', sans-serif;
        font-size: 11px;
        background: transparent;
        letter-spacing: 0.2px;
    }}
    #taskUploadLabel {{
        color: {COLORS['fg_text']};
        font-family: 'Segoe UI', sans-serif;
        font-size: 13px;
        background: transparent;
    }}
    #taskFileNameLabel {{
        color: {COLORS['accent']};
        font-family: 'Segoe UI', sans-serif;
        font-size: 12px;
        background: transparent;
    }}
    #taskLangCombo {{
        background-color: {COLORS['bg_button']};
        color: {COLORS['fg_text']};
        border: 1px solid {COLORS['border']};
        border-radius: 7px;
        font-size: 13px;
        font-family: 'Segoe UI', sans-serif;
        font-weight: bold;
        padding: 4px 10px;
    }}
    #taskLangCombo:hover {{
        background-color: {COLORS['bg_btn_hover']};
        border: 1px solid {COLORS['accent']};
        color: {COLORS['fg_title']};
    }}
    #taskLangCombo::drop-down {{
        border: none;
        width: 22px;
    }}
    #taskLangCombo QAbstractItemView {{
        background-color: {COLORS['bg_button']};
        color: {COLORS['fg_text']};
        border: 1px solid {COLORS['accent']};
        selection-background-color: {COLORS['accent']};
        selection-color: {COLORS['fg_title']};
        outline: none;
    }}
    #taskSectionLabel {{
        color: {COLORS['fg_title']};
        font-family: 'Segoe UI', sans-serif;
        font-size: 11px;
        font-weight: bold;
        letter-spacing: 1px;
        background: transparent;
    }}

    /* ── Діалог перевірки компіляторів ── */
    #compilerDialog {{
        background-color: {COLORS['bg_main']};
    }}
    #compilerDialogTitle {{
        color: {COLORS['fg_title']};
        font-family: 'Segoe UI', sans-serif;
        font-size: 19px;
        font-weight: bold;
        background: transparent;
    }}
    #compilerDialogText {{
        color: {COLORS['fg_text']};
        font-family: 'Segoe UI', sans-serif;
        font-size: 13px;
        background: transparent;
    }}
    #compilerDialogList {{
        color: {COLORS['accent']};
        font-family: 'Segoe UI', sans-serif;
        font-size: 13px;
        font-weight: bold;
        background: transparent;
    }}
    #compilerDialogLog {{
        background-color: {COLORS['bg_titlebar']};
        color: {COLORS['fg_text']};
        border: 1px solid {COLORS['border']};
        border-radius: 8px;
        font-family: 'Consolas', 'Courier New', monospace;
        font-size: 12px;
        padding: 8px;
    }}
    #compilerInstallBtn {{
        background-color: {COLORS['card_btn1_bg']};
        color: {COLORS['fg_title']};
        border: 1px solid {COLORS['card_btn1_border']};
        border-radius: 8px;
        font-size: 13px;
        font-family: 'Segoe UI', sans-serif;
        font-weight: bold;
    }}
    #compilerInstallBtn:hover {{
        background-color: {COLORS['card_btn1_hover']};
        border: 1px solid {COLORS['bg_btn_hover']};
    }}
    #compilerInstallBtn:disabled {{
        background-color: {COLORS['bg_button']};
        color: {COLORS['fg_text']};
        border: 1px solid {COLORS['border']};
    }}
    #compilerCloseBtn {{
        background-color: {COLORS['bg_button']};
        color: {COLORS['fg_text']};
        border: 1px solid {COLORS['border']};
        border-radius: 8px;
        font-size: 13px;
        font-family: 'Segoe UI', sans-serif;
        font-weight: bold;
        padding: 0px 16px;
    }}
    #compilerCloseBtn:hover {{
        background-color: {COLORS['bg_btn_hover']};
        border: 1px solid {COLORS['accent']};
        color: {COLORS['fg_title']};
    }}
"""

EDGE_MARGIN    = 8
EDGE_NONE      = 0
EDGE_LEFT      = 1
EDGE_RIGHT     = 2
EDGE_TOP       = 3
EDGE_BOTTOM    = 4
EDGE_TOP_LEFT  = 5
EDGE_TOP_RIGHT = 6
EDGE_BOT_LEFT  = 7
EDGE_BOT_RIGHT = 8

CURSOR_MAP = {
    EDGE_LEFT:      Qt.SizeHorCursor,
    EDGE_RIGHT:     Qt.SizeHorCursor,
    EDGE_TOP:       Qt.SizeVerCursor,
    EDGE_BOTTOM:    Qt.SizeVerCursor,
    EDGE_TOP_LEFT:  Qt.SizeFDiagCursor,
    EDGE_BOT_RIGHT: Qt.SizeFDiagCursor,
    EDGE_TOP_RIGHT: Qt.SizeBDiagCursor,
    EDGE_BOT_LEFT:  Qt.SizeBDiagCursor,
}


# ──────────────────────────────────────────────
#  ПІКТОГРАМИ
# ──────────────────────────────────────────────
def make_info_icon(size: int = 18, color: QColor = None) -> QPixmap:
    if color is None:
        color = QColor(COLORS["fg_title"])
    px = QPixmap(size, size)
    px.fill(Qt.transparent)
    p = QPainter(px)
    p.setRenderHint(QPainter.Antialiasing)
    pen = QPen(color, 1.5)
    p.setPen(pen)
    p.setBrush(Qt.NoBrush)
    margin = 1
    p.drawEllipse(margin, margin, size - margin * 2, size - margin * 2)
    dot_r = max(1, size // 10)
    cx    = size // 2
    dot_y = size // 4
    p.setBrush(QBrush(color))
    p.setPen(Qt.NoPen)
    p.drawEllipse(cx - dot_r, dot_y - dot_r, dot_r * 2, dot_r * 2)
    pen2 = QPen(color, max(1.5, size / 12))
    pen2.setCapStyle(Qt.RoundCap)
    p.setPen(pen2)
    line_x   = cx
    line_top = size // 2 - 1
    line_bot = size - size // 4
    p.drawLine(line_x, line_top, line_x, line_bot)
    half_w = max(2, size // 6)
    p.drawLine(line_x - half_w, line_bot, line_x + half_w, line_bot)
    p.end()
    return px


def make_play_icon(size: int = 16, color: QColor = None) -> QPixmap:
    if color is None:
        color = QColor(COLORS["fg_title"])
    px = QPixmap(size, size)
    px.fill(Qt.transparent)
    p = QPainter(px)
    p.setRenderHint(QPainter.Antialiasing)
    p.setPen(Qt.NoPen)
    p.setBrush(QBrush(color))
    path = QPainterPath()
    m = size * 0.12
    path.moveTo(m,        m)
    path.lineTo(size - m, size / 2)
    path.lineTo(m,        size - m)
    path.closeSubpath()
    p.drawPath(path)
    p.end()
    return px


def make_back_icon(size: int = 18, color: QColor = None) -> QPixmap:
    """Стрілка ліворуч."""
    if color is None:
        color = QColor(COLORS["fg_title"])
    px = QPixmap(size, size)
    px.fill(Qt.transparent)
    p = QPainter(px)
    p.setRenderHint(QPainter.Antialiasing)
    pen = QPen(color, max(1.8, size / 10))
    pen.setCapStyle(Qt.RoundCap)
    pen.setJoinStyle(Qt.RoundJoin)
    p.setPen(pen)
    p.setBrush(Qt.NoBrush)
    cy   = size / 2
    tip  = size * 0.18
    tail = size * 0.82
    arm  = size * 0.28
    p.drawLine(int(tip), int(cy), int(tail), int(cy))
    path = QPainterPath()
    path.moveTo(tip + arm, cy - arm)
    path.lineTo(tip,       cy)
    path.lineTo(tip + arm, cy + arm)
    p.drawPath(path)
    p.end()
    return px


def make_next_icon(size: int = 16, color: QColor = None) -> QPixmap:
    """Стрілка праворуч (для 'наступний рівень')."""
    if color is None:
        color = QColor(COLORS["fg_title"])
    px = QPixmap(size, size)
    px.fill(Qt.transparent)
    p = QPainter(px)
    p.setRenderHint(QPainter.Antialiasing)
    pen = QPen(color, max(1.8, size / 10))
    pen.setCapStyle(Qt.RoundCap)
    pen.setJoinStyle(Qt.RoundJoin)
    p.setPen(pen)
    p.setBrush(Qt.NoBrush)
    cy   = size / 2
    tip  = size * 0.82
    tail = size * 0.18
    arm  = size * 0.28
    p.drawLine(int(tail), int(cy), int(tip), int(cy))
    path = QPainterPath()
    path.moveTo(tip - arm, cy - arm)
    path.lineTo(tip,       cy)
    path.lineTo(tip - arm, cy + arm)
    p.drawPath(path)
    p.end()
    return px


def make_home_icon(size: int = 18, color: QColor = None) -> QPixmap:
    """Піктограма будинку."""
    if color is None:
        color = QColor(COLORS["fg_title"])
    px = QPixmap(size, size)
    px.fill(Qt.transparent)
    p = QPainter(px)
    p.setRenderHint(QPainter.Antialiasing)
    pen = QPen(color, max(1.6, size / 11))
    pen.setCapStyle(Qt.RoundCap)
    pen.setJoinStyle(Qt.RoundJoin)
    p.setPen(pen)
    p.setBrush(Qt.NoBrush)
    # Дах — трикутник
    mx  = size / 2
    roof_top  = size * 0.08
    roof_base = size * 0.52
    wall_l    = size * 0.12
    wall_r    = size * 0.88
    path = QPainterPath()
    path.moveTo(mx, roof_top)
    path.lineTo(wall_r, roof_base)
    path.lineTo(wall_l, roof_base)
    path.closeSubpath()
    p.drawPath(path)
    # Стіни — прямокутник
    wall_top  = roof_base
    wall_bot  = size * 0.92
    wall_il   = size * 0.14
    wall_ir   = size * 0.86
    p.drawRect(int(wall_il), int(wall_top), int(wall_ir - wall_il), int(wall_bot - wall_top))
    # Двері — маленький прямокутник знизу по центру
    door_w  = size * 0.22
    door_h  = size * 0.26
    door_x  = mx - door_w / 2
    door_y  = wall_bot - door_h
    p.drawRect(int(door_x), int(door_y), int(door_w), int(door_h))
    p.end()
    return px


# ──────────────────────────────────────────────
#  БАЗОВА КАРТКА
# ──────────────────────────────────────────────
class BaseCard(QWidget):

    def __init__(self, parent: QWidget, scrollable: bool = False):
        super().__init__(parent)
        self.setObjectName("card")
        self.setAttribute(Qt.WA_StyledBackground, True)
        self.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Preferred)

        shadow = QGraphicsDropShadowEffect()
        shadow.setBlurRadius(60)
        shadow.setXOffset(0)
        shadow.setYOffset(12)
        shadow.setColor(QColor(0, 0, 0, 200))
        self.setGraphicsEffect(shadow)

        root = QVBoxLayout(self)
        root.setContentsMargins(40, 34, 40, 28)
        root.setSpacing(0)

        self._title = QLabel("")
        self._title.setObjectName("cardTitle")
        self._title.setWordWrap(True)
        root.addWidget(self._title)

        root.addSpacing(16)

        divider = QWidget()
        divider.setObjectName("cardDivider")
        root.addWidget(divider)

        root.addSpacing(18)

        self._text_label = QLabel("")
        self._text_label.setObjectName("cardText")
        self._text_label.setWordWrap(True)
        self._text_label.setAlignment(Qt.AlignLeft | Qt.AlignTop)

        if scrollable:
            scroll = QScrollArea()
            scroll.setObjectName("scrollArea")
            scroll.setWidgetResizable(True)
            scroll.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
            scroll.setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)
            inner = QWidget()
            inner.setObjectName("scrollContent")
            inner.setAttribute(Qt.WA_StyledBackground, True)
            inner_layout = QVBoxLayout(inner)
            inner_layout.setContentsMargins(0, 0, 8, 0)
            inner_layout.setSpacing(0)
            inner_layout.addWidget(self._text_label)
            inner_layout.addStretch()
            scroll.setWidget(inner)
            root.addWidget(scroll, stretch=1)
        else:
            root.addWidget(self._text_label, stretch=1)

        root.addSpacing(28)

        btn_row = QHBoxLayout()
        btn_row.setSpacing(12)
        btn_row.setContentsMargins(0, 0, 0, 0)
        btn_row.addStretch(1)

        self._btn_left = QPushButton("")
        self._btn_left.setObjectName("cardBtnInfo")
        self._btn_left.setCursor(Qt.PointingHandCursor)
        self._btn_left.setFixedHeight(42)
        self._btn_left.setIconSize(QSize(18, 18))

        self._btn_right = QPushButton("  Спробувати")
        self._btn_right.setObjectName("cardBtnPlay")
        self._btn_right.setCursor(Qt.PointingHandCursor)
        self._btn_right.setFixedHeight(42)
        self._btn_right.setIconSize(QSize(16, 16))
        self._btn_right.setIcon(QIcon(make_play_icon()))

        btn_row.addWidget(self._btn_left)
        btn_row.addWidget(self._btn_right)
        root.addLayout(btn_row)

    def set_content(self, title: str, text: str):
        self._title.setText(title)
        self._text_label.setText(text)

    def set_left_btn(self, label: str, icon: QPixmap, callback):
        self._btn_left.setText(f"  {label}")
        self._btn_left.setIcon(QIcon(icon))
        try:
            self._btn_left.clicked.disconnect()
        except RuntimeError:
            pass
        self._btn_left.clicked.connect(callback)

    def set_right_btn_callback(self, callback):
        try:
            self._btn_right.clicked.disconnect()
        except RuntimeError:
            pass
        self._btn_right.clicked.connect(callback)


# ──────────────────────────────────────────────
#  OVERLAY — стек карток
# ──────────────────────────────────────────────
class CardOverlay(QWidget):

    def __init__(self, parent: QWidget, on_try=None):
        super().__init__(parent)
        self.setObjectName("overlayBg")
        self.setAttribute(Qt.WA_TransparentForMouseEvents, False)
        self.hide()

        self._anim        = None
        self._card_w      = 600
        self._card_h      = 400
        self._detail_text = ""
        self._cur_title   = ""
        self._cur_text    = ""
        self._cur_meta    = None
        self._on_try      = on_try   # callable(title, text, meta) або None

        self._build_layers()

    def _build_layers(self):
        self._layer0 = QWidget(self)
        self._layer0.setObjectName("overlayBg")
        self._layer0.setAttribute(Qt.WA_StyledBackground, False)
        self._layer0.setAttribute(Qt.WA_NoSystemBackground, True)

        lay0 = QVBoxLayout(self._layer0)
        lay0.setContentsMargins(0, 0, 0, 0)
        lay0.addStretch(1)
        row0 = QHBoxLayout()
        row0.addStretch(1)
        self._card0 = BaseCard(self._layer0, scrollable=False)
        self._card0.set_left_btn("Детальніше", make_info_icon(), self._open_detail)
        self._card0.set_right_btn_callback(self._try_from_card0)
        row0.addWidget(self._card0)
        row0.addStretch(1)
        lay0.addLayout(row0)
        lay0.addStretch(1)

        self._layer1 = QWidget(self)
        self._layer1.setObjectName("overlayBg")
        self._layer1.setAttribute(Qt.WA_StyledBackground, False)
        self._layer1.setAttribute(Qt.WA_NoSystemBackground, True)
        self._layer1.hide()

        lay1 = QVBoxLayout(self._layer1)
        lay1.setContentsMargins(0, 0, 0, 0)
        lay1.addStretch(1)
        row1 = QHBoxLayout()
        row1.addStretch(1)
        self._card1 = BaseCard(self._layer1, scrollable=True)
        self._card1.set_left_btn("Повернутись", make_back_icon(), self._close_detail)
        self._card1.set_right_btn_callback(self._try_from_card1)
        row1.addWidget(self._card1)
        row1.addStretch(1)
        lay1.addLayout(row1)
        lay1.addStretch(1)

    def _try_from_card0(self):
        """Натиснуто 'Спробувати' на першій картці."""
        self.close_all()
        if self._on_try:
            self._on_try(self._cur_title, self._cur_text, self._cur_meta)

    def _try_from_card1(self):
        """Натиснуто 'Спробувати' на деталях."""
        self.close_all()
        if self._on_try:
            self._on_try(self._cur_title, self._cur_text, self._cur_meta)

    def _apply_card_size(self):
        for card in (self._card0, self._card1):
            card.setFixedWidth(self._card_w)
            card.setFixedHeight(self._card_h)

    def show_card(self, title: str, text: str, detail_text: str, meta=None):
        self._detail_text = detail_text
        self._cur_title   = title
        self._cur_text    = text
        self._cur_meta    = meta
        self._card0.set_content(title, text)
        self._card1.set_content(f"Детальніше  —  {title}", detail_text)

        self.resize(self.parent().size())
        self._layer0.resize(self.size())
        self._layer1.resize(self.size())
        self._layer1.hide()

        self._apply_card_size()
        self.raise_()
        self.show()
        self._animate_card_in(self._card0)

    def _open_detail(self):
        self._layer1.resize(self.size())
        self._layer1.raise_()
        self._layer1.show()
        self._animate_card_in(self._card1)

    def _close_detail(self):
        self._layer1.hide()

    def close_all(self):
        self._layer1.hide()
        self._card0.setMinimumHeight(0)
        self.hide()

    def _animate_card_in(self, card: BaseCard):
        card.setMinimumHeight(0)
        card.setMaximumHeight(0)
        self._anim = QPropertyAnimation(card, b"maximumHeight", self)
        self._anim.setDuration(260)
        self._anim.setStartValue(0)
        self._anim.setEndValue(self._card_h)
        self._anim.setEasingCurve(QEasingCurve.OutCubic)
        self._anim.finished.connect(lambda c=card: c.setMinimumHeight(self._card_h))
        self._anim.start()

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)
        color = QColor(COLORS["overlay_bg"])
        color.setAlpha(215)
        painter.fillRect(self.rect(), color)

    def mousePressEvent(self, event):
        pos = event.position().toPoint()
        if self._layer1.isVisible():
            card1_pos  = self._card1.mapTo(self, QPoint(0, 0))
            card1_rect = QRect(card1_pos, self._card1.size())
            if not card1_rect.contains(pos):
                self._close_detail()
            return
        card0_pos  = self._card0.mapTo(self, QPoint(0, 0))
        card0_rect = QRect(card0_pos, self._card0.size())
        if not card0_rect.contains(pos):
            self.close_all()
        super().mousePressEvent(event)

    def resizeEvent(self, event):
        if self.parent():
            new_size = self.parent().size()
            self.resize(new_size)
            self._layer0.resize(new_size)
            self._layer1.resize(new_size)
            p_w = new_size.width()
            p_h = new_size.height()
            self._card_w = max(500, int(p_w * 0.80))
            self._card_h = max(300, int(p_h * 0.72))
            self._apply_card_size()
        super().resizeEvent(event)


# ──────────────────────────────────────────────
#  ЕКРАН РІВНЯ (узагальнений)
# ──────────────────────────────────────────────
class LevelScreen(QWidget):
    """
    Один екран рівневої системи.

    :param level_title: рядок заголовка ("Easy", "Medium", …)
    :param rows0:       список кнопок верхнього ряду [(label, title, text, detail), …]
    :param rows1:       список кнопок нижнього ряду
    :param on_home:     callable — повернутись на головну (титульну) сторінку
    :param on_prev:     callable або None — перейти на попередній рівень
    :param on_next:     callable або None — перейти на наступний рівень
    """

    def __init__(self, level_title: str, rows0: list, rows1: list,
                 on_home=None, on_prev=None, on_next=None,
                 on_try=None, parent=None):
        super().__init__(parent)
        self.setObjectName("contentArea")
        self.setMouseTracking(True)

        self._rows0 = rows0
        self._rows1 = rows1
        self._on_try = on_try
        self._ref_w = 960
        self._ref_h = 520
        self._buttons_row0: list[QPushButton] = []
        self._buttons_row1: list[QPushButton] = []

        root = QVBoxLayout(self)
        root.setContentsMargins(0, 0, 0, 0)
        root.setSpacing(0)

        # ══════════════════════════════════════
        #  РЯД 1: навігаційна панель
        # ══════════════════════════════════════
        self._navbar = QWidget()
        self._navbar.setObjectName("contentArea")
        nav_layout = QHBoxLayout(self._navbar)
        nav_layout.setContentsMargins(12, 8, 12, 4)
        nav_layout.setSpacing(8)

        # Кнопка "На головну" — крайня ліва
        self._btn_home = QPushButton("  На головну")
        self._btn_home.setObjectName("levelNavBtn")
        self._btn_home.setIcon(QIcon(make_home_icon(18)))
        self._btn_home.setIconSize(QSize(18, 18))
        self._btn_home.setCursor(Qt.PointingHandCursor)
        if on_home:
            self._btn_home.clicked.connect(on_home)
        nav_layout.addWidget(self._btn_home)

        nav_layout.addStretch(1)

        # Кнопка "Попередній рівень" (якщо є)
        if on_prev is not None:
            self._btn_prev = QPushButton("Попередній рівень  ")
            self._btn_prev.setObjectName("levelNavBtn")
            self._btn_prev.setIcon(QIcon(make_back_icon(16)))
            self._btn_prev.setIconSize(QSize(16, 16))
            self._btn_prev.setCursor(Qt.PointingHandCursor)
            self._btn_prev.clicked.connect(on_prev)
            nav_layout.addWidget(self._btn_prev)
        else:
            self._btn_prev = None

        # Кнопка "Наступний рівень" (якщо є)
        if on_next is not None:
            self._btn_next = QPushButton("  Наступний рівень")
            self._btn_next.setObjectName("levelNavBtn")
            self._btn_next.setIcon(QIcon(make_next_icon(16)))
            self._btn_next.setIconSize(QSize(16, 16))
            self._btn_next.setLayoutDirection(Qt.RightToLeft)
            self._btn_next.setCursor(Qt.PointingHandCursor)
            self._btn_next.clicked.connect(on_next)
            nav_layout.addWidget(self._btn_next)
        else:
            self._btn_next = None

        root.addWidget(self._navbar)

        sep1 = QWidget()
        sep1.setObjectName("cardDivider")
        sep1.setFixedHeight(1)
        root.addWidget(sep1)

        # ══════════════════════════════════════
        #  РЯД 2: великий заголовок рівня
        # ══════════════════════════════════════
        self._title_lbl = QLabel(level_title)
        self._title_lbl.setObjectName("levelTitleBig")
        self._title_lbl.setAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
        root.addWidget(self._title_lbl)

        sep2 = QWidget()
        sep2.setObjectName("cardDivider")
        sep2.setFixedHeight(1)
        root.addWidget(sep2)

        # ══════════════════════════════════════
        #  РЯД 3: кнопки рівня
        # ══════════════════════════════════════
        self._content = QWidget()
        self._content.setObjectName("contentArea")
        self._content.setMouseTracking(True)
        root.addWidget(self._content, stretch=1)

        self._outer = QVBoxLayout(self._content)
        self._outer.setContentsMargins(0, 0, 0, 0)
        self._outer.setSpacing(0)
        self._outer.addStretch(1)

        self._row0 = QHBoxLayout()
        self._row0.setSpacing(0)
        self._row0.setContentsMargins(0, 0, 0, 0)
        self._outer.addLayout(self._row0)

        self._row_spacer = QSpacerItem(0, 18, QSizePolicy.Minimum, QSizePolicy.Fixed)
        self._outer.addItem(self._row_spacer)

        self._row1 = QHBoxLayout()
        self._row1.setSpacing(0)
        self._row1.setContentsMargins(0, 0, 0, 0)
        self._outer.addLayout(self._row1)

        self._outer.addStretch(1)

        self._build_buttons()

        # Overlay — дочірній від self, покриває весь екран рівня
        self._overlay = CardOverlay(self, on_try=self._on_try)
        self._overlay.hide()

    def _build_buttons(self):
        row0_count = len(self._rows0)
        row1_count = len(self._rows1)

        self._add_spacer(self._row0, 0)
        for col, entry in enumerate(self._rows0):
            btn_label, card_title, card_text, detail_text = entry[0], entry[1], entry[2], entry[3]
            meta = entry[4] if len(entry) > 4 else None
            btn = QPushButton(btn_label)
            btn.setObjectName("mainBtn")
            btn.setCursor(Qt.PointingHandCursor)
            btn.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Expanding)
            btn.clicked.connect(
                lambda checked=False,
                       t=card_title, tx=card_text, dt=detail_text, mt=meta:
                self._overlay.show_card(t, tx, dt, mt)
            )
            self._row0.addWidget(btn)
            self._buttons_row0.append(btn)
            if col < row0_count - 1:
                self._add_spacer(self._row0, 0)
        self._add_spacer(self._row0, 0)

        self._add_spacer(self._row1, 0)
        for col, entry in enumerate(self._rows1):
            btn_label, card_title, card_text, detail_text = entry[0], entry[1], entry[2], entry[3]
            meta = entry[4] if len(entry) > 4 else None
            btn = QPushButton(btn_label)
            btn.setObjectName("mainBtn")
            btn.setCursor(Qt.PointingHandCursor)
            btn.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Expanding)
            btn.clicked.connect(
                lambda checked=False,
                       t=card_title, tx=card_text, dt=detail_text, mt=meta:
                self._overlay.show_card(t, tx, dt, mt)
            )
            self._row1.addWidget(btn)
            self._buttons_row1.append(btn)
            if col < row1_count - 1:
                self._add_spacer(self._row1, 0)
        self._add_spacer(self._row1, 0)

    @staticmethod
    def _add_spacer(layout: QHBoxLayout, w: int) -> QSpacerItem:
        sp = QSpacerItem(w, 0, QSizePolicy.Fixed, QSizePolicy.Minimum)
        layout.addItem(sp)
        return sp

    def resizeEvent(self, event: QResizeEvent):
        w = event.size().width()
        h = event.size().height()
        row0_count = len(self._rows0)
        row1_count = len(self._rows1)

        # ── Навбар ─────────────────────────────
        nav_btn_h = max(38, int(h * 0.055 * 1.1))
        nb_h = nav_btn_h + 12
        self._navbar.setFixedHeight(nb_h)
        self._btn_home.setFixedHeight(nav_btn_h)
        home_w = max(130, int(w * 0.13 * 1.1))
        self._btn_home.setFixedWidth(home_w)
        nav_side_w = max(176, int(w * 0.176 * 1.1))
        if self._btn_prev is not None:
            self._btn_prev.setFixedHeight(nav_btn_h)
            self._btn_prev.setFixedWidth(nav_side_w)
        if self._btn_next is not None:
            self._btn_next.setFixedHeight(nav_btn_h)
            self._btn_next.setFixedWidth(nav_side_w)

        # ── Заголовок рівня ────────────────────
        title_h = max(60, int(h * 0.18))
        self._title_lbl.setFixedHeight(title_h)

        # ── Кнопки контенту ────────────────────
        content_h = max(80, h - nb_h - title_h - 4)
        btn_h = max(36, int(content_h * 0.17))

        scale_x = w / self._ref_w
        pad_h = max(8, int(32 * scale_x))
        scale_y = content_h / self._ref_h
        pad_v = max(4, int(14 * scale_y))
        self._outer.setContentsMargins(pad_h, pad_v, pad_h, pad_v)

        usable_w = max(200, w - pad_h * 2)
        gap = max(6, int(14 * scale_x))
        btn_w = max(30, int((usable_w - gap * (row0_count - 1)) / row0_count))

        total_row1_w = row1_count * btn_w + (row1_count - 1) * gap
        center_margin = max(0, (usable_w - total_row1_w) // 2)

        for btn in self._buttons_row0 + self._buttons_row1:
            btn.setFixedWidth(btn_w)
            btn.setFixedHeight(btn_h)

        self._update_spacers(self._row0, gap, left=0, right=0)
        self._update_spacers(self._row1, gap, left=center_margin, right=center_margin)

        row_gap = max(8, int(18 * scale_y))
        self._row_spacer.changeSize(0, row_gap, QSizePolicy.Minimum, QSizePolicy.Fixed)
        self._outer.invalidate()

        # Overlay покриває весь LevelScreen
        self._overlay.resize(event.size())
        super().resizeEvent(event)

    def _update_spacers(self, layout: QHBoxLayout,
                        gap: int, left: int, right: int):
        spacers: list[QSpacerItem] = []
        for i in range(layout.count()):
            item = layout.itemAt(i)
            if item and item.spacerItem():
                spacers.append(item.spacerItem())
        if len(spacers) < 2:
            return
        spacers[0].changeSize(left,  0, QSizePolicy.Fixed, QSizePolicy.Minimum)
        for sp in spacers[1:-1]:
            sp.changeSize(gap,   0, QSizePolicy.Fixed, QSizePolicy.Minimum)
        spacers[-1].changeSize(right, 0, QSizePolicy.Fixed, QSizePolicy.Minimum)
        layout.invalidate()


# ──────────────────────────────────────────────
#  ПОТІК ТЕСТУВАННЯ (компіляція тестера + запуск, без блокування UI)
# ──────────────────────────────────────────────
class _TestRunnerThread(QThread):
    """
    Виконує у фоновому потоці: компіляцію *_tester_multilang.cpp (з кешем)
    та запуск отриманого бінарника проти файлу розв'язку користувача.
    Сигналізує результат назад у головний (UI) потік через сигнали Qt.
    """
    finished_ok    = Signal(dict, bool, str)   # (result_dict, is_temp, solution_path)
    finished_error = Signal(str, bool, str)    # (message, is_temp, solution_path)

    def __init__(self, tester_cpp_path: str, solution_path: str, is_temp: bool, parent=None):
        super().__init__(parent)
        self._tester_cpp_path = tester_cpp_path
        self._solution_path   = solution_path
        self._is_temp         = is_temp

    def run(self):
        bin_path, err = compile_tester(self._tester_cpp_path)
        if bin_path is None:
            self.finished_error.emit(err, self._is_temp, self._solution_path)
            return
        result = run_tester(bin_path, self._solution_path)
        if not result.get("ok"):
            self.finished_error.emit(
                result.get("error", "Невідома помилка тестування"),
                self._is_temp, self._solution_path
            )
            return
        self.finished_ok.emit(result, self._is_temp, self._solution_path)


# ──────────────────────────────────────────────
#  ШІ-АНАЛІЗ РОЗВ'ЯЗКУ (Claude API)
# ──────────────────────────────────────────────
def _ai_config_path() -> str:
    """
    Шлях до файлу з налаштуваннями ШІ (зараз лише API-ключ).
    На Windows — %APPDATA%\\Jereb_project_1\\ai_config.json,
    на інших ОС — ~/.jereb_project_1/ai_config.json.
    Ключ зберігається ЛОКАЛЬНО на машині користувача, ніде не
    надсилається крім прямих запитів до api.anthropic.com.
    """
    if is_windows():
        base = os.environ.get("APPDATA") or os.path.expanduser("~")
        d = os.path.join(base, "Jereb_project_1")
    else:
        d = os.path.join(os.path.expanduser("~"), ".jereb_project_1")
    os.makedirs(d, exist_ok=True)
    return os.path.join(d, "ai_config.json")


def load_ai_api_key() -> str:
    """Повертає збережений API-ключ, або порожній рядок якщо ще не введений."""
    try:
        with open(_ai_config_path(), encoding="utf-8") as f:
            return json.load(f).get("api_key", "")
    except (OSError, ValueError):
        return ""


def save_ai_api_key(key: str):
    """Зберігає API-ключ локально (перезаписує попередній)."""
    try:
        with open(_ai_config_path(), "w", encoding="utf-8") as f:
            json.dump({"api_key": key}, f)
    except OSError:
        pass


def html_to_plain_text(html_path: str) -> str:
    """
    Грубо, але надійно прибирає HTML-теги з умови задачі для передачі в
    текстовий промпт ШІ — нам не потрібне форматування, лише сам текст.
    """
    if not html_path or not os.path.isfile(html_path):
        return ""
    try:
        with open(html_path, encoding="utf-8", errors="replace") as f:
            html = f.read()
    except OSError:
        return ""
    # Прибираємо <script>/<style> разом із вмістом, потім решту тегів.
    html = re.sub(r"<script\b[^>]*>.*?</script>", "", html, flags=re.DOTALL | re.IGNORECASE)
    html = re.sub(r"<style\b[^>]*>.*?</style>", "", html, flags=re.DOTALL | re.IGNORECASE)
    html = re.sub(r"<br\s*/?>", "\n", html, flags=re.IGNORECASE)
    html = re.sub(r"</p>", "\n\n", html, flags=re.IGNORECASE)
    text = re.sub(r"<[^>]+>", "", html)
    text = text.replace("&nbsp;", " ")
    text = text.replace("&le;", "≤").replace("&ge;", "≥").replace("&minus;", "-")
    text = text.replace("&middot;", "·").replace("&times;", "×").replace("&divide;", "÷")
    text = text.replace("&lt;", "<").replace("&gt;", ">").replace("&amp;", "&")
    # Згортаємо надмірну кількість порожніх рядків.
    text = re.sub(r"\n{3,}", "\n\n", text)
    return text.strip()


class AiAnalysisThread(QThread):
    """
    Виконує запит до Claude API в окремому потоці, щоб не блокувати UI
    під час очікування відповіді (зазвичай кілька секунд).
    """
    finished_ok = Signal(str)
    finished_error = Signal(str)

    def __init__(self, api_key: str, messages: list, parent=None):
        super().__init__(parent)
        self._api_key = api_key
        self._messages = messages

    def run(self):
        try:
            body = json.dumps({
                "model": "claude-sonnet-4-6",
                "max_tokens": 2000,
                "messages": self._messages,
            }).encode("utf-8")
            req = urllib.request.Request(
                "https://api.anthropic.com/v1/messages",
                data=body,
                method="POST",
                headers={
                    "Content-Type": "application/json",
                    "x-api-key": self._api_key,
                    "anthropic-version": "2023-06-01",
                },
            )
            with urllib.request.urlopen(req, timeout=60) as resp:
                data = json.loads(resp.read().decode("utf-8"))
            text_parts = [
                block.get("text", "") for block in data.get("content", [])
                if block.get("type") == "text"
            ]
            self.finished_ok.emit("\n".join(text_parts).strip() or "(порожня відповідь)")
        except urllib.error.HTTPError as e:
            try:
                err_body = json.loads(e.read().decode("utf-8"))
                msg = err_body.get("error", {}).get("message", str(e))
            except Exception:
                msg = str(e)
            if e.code == 401:
                msg = "Невірний API-ключ. Перевірте ключ у налаштуваннях ШІ."
            self.finished_error.emit(f"Помилка API ({e.code}): {msg}")
        except urllib.error.URLError as e:
            self.finished_error.emit(f"Не вдалося з'єднатися з api.anthropic.com: {e.reason}")
        except Exception as e:
            self.finished_error.emit(f"Несподівана помилка: {e}")


class ApiKeyDialog(QDialog):
    """Невеликий діалог для введення/заміни API-ключа Anthropic."""

    def __init__(self, current_key: str = "", parent=None):
        super().__init__(parent)
        self.setWindowTitle("Налаштування ШІ")
        self.setObjectName("compilerDialog")
        self.setFixedWidth(480)
        lay = QVBoxLayout(self)
        lay.setContentsMargins(20, 20, 20, 20)
        lay.setSpacing(12)

        label = QLabel(
            "Введіть API-ключ Anthropic для аналізу розв'язків через ШІ.\n"
            "Отримати ключ: console.anthropic.com → API Keys.\n"
            "Ключ зберігається лише локально на цьому комп'ютері."
        )
        label.setObjectName("compilerDialogText")
        label.setWordWrap(True)
        lay.addWidget(label)

        self._input = QLineEdit(current_key)
        self._input.setPlaceholderText("sk-ant-...")
        self._input.setEchoMode(QLineEdit.Password)
        self._input.setFixedHeight(36)
        lay.addWidget(self._input)

        show_row = QHBoxLayout()
        self._chk_show = QCheckBox("Показати ключ")
        self._chk_show.toggled.connect(
            lambda checked: self._input.setEchoMode(QLineEdit.Normal if checked else QLineEdit.Password)
        )
        show_row.addWidget(self._chk_show)
        show_row.addStretch(1)
        lay.addLayout(show_row)

        btn_row = QHBoxLayout()
        btn_cancel = QPushButton("Скасувати")
        btn_cancel.setObjectName("compilerCloseBtn")
        btn_cancel.clicked.connect(self.reject)
        btn_save = QPushButton("Зберегти")
        btn_save.setObjectName("compilerInstallBtn")
        btn_save.clicked.connect(self.accept)
        btn_row.addWidget(btn_cancel)
        btn_row.addWidget(btn_save)
        lay.addLayout(btn_row)

    def api_key(self) -> str:
        return self._input.text().strip()


class AiChatDialog(QDialog):
    """
    Невелике чат-вікно для ШІ-аналізу поточного розв'язку. Перед першим
    надсиланням користувач може (необов'язково) вставити еталонний
    розв'язок у спеціальне поле — ШІ порівняє код користувача і з умовою
    задачі, і з кодом тестера (очікувана поведінка), і, якщо заповнено,
    з еталоном. Далі можна ставити уточнюючі питання в тому самому
    контексті розмови.
    """

    def __init__(self, api_key: str, build_context_fn, parent=None):
        super().__init__(parent)
        self.setWindowTitle("ШІ-аналіз розв'язку")
        self.setObjectName("compilerDialog")
        self.resize(560, 680)
        self._api_key = api_key
        self._build_context_fn = build_context_fn  # (reference_text) -> готовий текст промпту
        self._history = []  # [{"role": "user"/"assistant", "content": str}, ...]
        self._thread = None
        self._started = False

        lay = QVBoxLayout(self)
        lay.setContentsMargins(16, 16, 16, 16)
        lay.setSpacing(10)

        # ── Початковий екран: опціональний еталон + кнопка запуску ──
        self._pre_panel = QWidget()
        pre_lay = QVBoxLayout(self._pre_panel)
        pre_lay.setContentsMargins(0, 0, 0, 0)
        pre_lay.setSpacing(8)

        pre_label = QLabel(
            "Якщо у вас є еталонний (правильний) розв'язок цієї задачі — "
            "вставте його нижче, ШІ врахує його при порівнянні. Це поле "
            "необов'язкове: аналіз спрацює і без нього, на основі умови "
            "задачі та логіки тестера."
        )
        pre_label.setObjectName("compilerDialogText")
        pre_label.setWordWrap(True)
        pre_lay.addWidget(pre_label)

        self._reference_input = QPlainTextEdit()
        self._reference_input.setObjectName("taskEditor")
        self._reference_input.setFixedHeight(160)
        self._reference_input.setPlaceholderText("(необов'язково) Еталонний розв'язок…")
        pre_lay.addWidget(self._reference_input)

        btn_start = QPushButton("Розпочати аналіз")
        btn_start.setObjectName("taskSubmitBtn")
        btn_start.setCursor(Qt.PointingHandCursor)
        btn_start.setFixedHeight(40)
        btn_start.clicked.connect(self._start_analysis)
        pre_lay.addWidget(btn_start)

        lay.addWidget(self._pre_panel)

        # ── Чат (з'являється після запуску аналізу) ──
        self._chat_view = QTextBrowser()
        self._chat_view.setObjectName("taskDescBrowser")
        self._chat_view.hide()
        lay.addWidget(self._chat_view, stretch=1)

        self._status_lbl = QLabel("")
        self._status_lbl.setObjectName("compilerDialogText")
        lay.addWidget(self._status_lbl)

        self._input_row_w = QWidget()
        input_row = QHBoxLayout(self._input_row_w)
        input_row.setContentsMargins(0, 0, 0, 0)
        self._input = QPlainTextEdit()
        self._input.setObjectName("taskEditor")
        self._input.setFixedHeight(60)
        self._input.setPlaceholderText("Поставте уточнююче питання…")
        input_row.addWidget(self._input, stretch=1)

        self._btn_send = QPushButton("Надіслати")
        self._btn_send.setObjectName("taskSubmitBtn")
        self._btn_send.setCursor(Qt.PointingHandCursor)
        self._btn_send.setFixedHeight(60)
        self._btn_send.clicked.connect(self._send_followup)
        input_row.addWidget(self._btn_send)
        self._input_row_w.hide()
        lay.addWidget(self._input_row_w)

    def _start_analysis(self):
        if self._started:
            return
        self._started = True
        reference_text = self._reference_input.toPlainText().strip()
        self._pre_panel.hide()
        self._chat_view.show()
        self._input_row_w.show()
        context_text = self._build_context_fn(reference_text)
        self._append_message("system", "Надсилаю запит на аналіз…")
        self._send_initial(context_text)

    def _append_message(self, role: str, text: str):
        safe = text.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace("\n", "<br>")
        if role == "user":
            self._chat_view.append(f"<b>Ви:</b><br>{safe}<br>")
        elif role == "assistant":
            self._chat_view.append(f"<b>ШІ:</b><br>{safe}<br>")
        else:
            self._chat_view.append(f"<i>{safe}</i><br>")

    def _send_initial(self, context_text: str):
        self._history.append({"role": "user", "content": context_text})
        self._run_request()

    def _send_followup(self):
        text = self._input.toPlainText().strip()
        if not text or self._thread is not None:
            return
        self._input.clear()
        self._append_message("user", text)
        self._history.append({"role": "user", "content": text})
        self._run_request()

    def _run_request(self):
        self._btn_send.setEnabled(False)
        self._status_lbl.setText("ШІ аналізує… (це може зайняти кілька секунд)")
        self._thread = AiAnalysisThread(self._api_key, list(self._history), self)
        self._thread.finished_ok.connect(self._on_response_ok)
        self._thread.finished_error.connect(self._on_response_error)
        self._thread.start()

    def _on_response_ok(self, text: str):
        self._thread = None
        self._status_lbl.setText("")
        self._btn_send.setEnabled(True)
        self._history.append({"role": "assistant", "content": text})
        self._append_message("assistant", text)

    def _on_response_error(self, message: str):
        self._thread = None
        self._status_lbl.setText("")
        self._btn_send.setEnabled(True)
        self._append_message("system", f"Помилка: {message}")


# ──────────────────────────────────────────────
#  СТОРІНКА ЗАВДАННЯ
# ──────────────────────────────────────────────
class TaskScreen(QWidget):
    """
    ЗГОРНУТИЙ стан (початковий, скріншот 2):
    ┌──────────────────────────────────┬─────────┐
    │  навбар [← Назад]               │         │
    ├──────────────────────────────────┤  [▼]    │
    │                                  │         │
    │  опис задачі (вся ліва ширина,   │         │
    │  висока зона, скролабель)        │         │
    │                                  │         │
    ├──────────────────────────────────┤         │
    │  [Редактор] [Завантажити] файл   │         │
    │  текстовий редактор        [ШІ]  │         │
    └──────────────────────────────────┴─────────┘
    Права колонка — тільки кнопка ▼, без списку.

    РОЗГОРНУТИЙ стан (скріншот 1):
    ┌─────────────────────────┬──────────────────┐
    │  навбар [← Назад]       │  РЕЗУЛЬТАТИ  [▲] │
    ├─────────────────────────┼──────────────────┤
    │  опис задачі (скорочена │  Результат 1 100 │
    │  висота)                │  Результат 2  85 │
    ├─────────────────────────┤  …               │
    │  [Редактор][Завантажити]│                  │
    │  текстовий редактор[ШІ] │  [Здати]         │
    └─────────────────────────┴──────────────────┘
    """

    def __init__(self, on_back, parent=None):
        super().__init__(parent)
        self.setObjectName("taskScreen")
        self.setAttribute(Qt.WA_StyledBackground, True)
        self._on_back = on_back
        self._list_expanded = False
        self._task_meta = None       # (subdir_key, load_filename) поточної задачі, або None
        self._cur_level_key = "ez"   # поточно вибраний рівень складності задачі
        self._picked_file_path = None  # шлях до завантаженого файлу розв'язку
        self._test_thread = None       # активний потік тестування (щоб не блокувати UI)
        self._last_test_data = {}      # {idx: {"input","expected","actual"}} останнього прогону
        self._last_raw_output = ""     # повний сирий вивід тестера — для діагностики
        self._viewing_test_idx = None  # номер тесту, що зараз переглядається (None = умова задачі)
        self._cur_problem_html_path = None  # шлях останньої HTML-умови (для open_task_html)

        # ══ Корінь: горизонтальний поділ ліво/право ══
        root_h = QHBoxLayout(self)
        root_h.setContentsMargins(0, 0, 0, 0)
        root_h.setSpacing(0)

        # ════════ ЛІВА КОЛОНКА (завжди видима) ════════
        left_w = QWidget()
        left_w.setObjectName("taskScreen")
        left_vlay = QVBoxLayout(left_w)
        left_vlay.setContentsMargins(0, 0, 0, 0)
        left_vlay.setSpacing(0)
        root_h.addWidget(left_w, stretch=1)

        # Навбар
        navbar = QWidget()
        navbar.setObjectName("taskScreen")
        nav_lay = QHBoxLayout(navbar)
        nav_lay.setContentsMargins(12, 8, 12, 8)
        nav_lay.setSpacing(0)

        self._btn_back = QPushButton("  Назад")
        self._btn_back.setObjectName("taskNavBtn")
        self._btn_back.setIcon(QIcon(make_back_icon(16)))
        self._btn_back.setIconSize(QSize(16, 16))
        self._btn_back.setCursor(Qt.PointingHandCursor)
        self._btn_back.setFixedHeight(34)
        self._btn_back.setFixedWidth(110)
        self._btn_back.clicked.connect(on_back)
        nav_lay.addWidget(self._btn_back)

        nav_lay.addStretch(1)

        # ── Перемикач складності задачі (по центру) ──
        # Видимий лише коли задача відкрита з можливістю вибору рівня
        # (тобто через open_task_with_levels, а не звичайний текст).
        self._level_switch = QWidget()
        self._level_switch.setObjectName("taskScreen")
        level_lay = QHBoxLayout(self._level_switch)
        level_lay.setContentsMargins(0, 0, 0, 0)
        level_lay.setSpacing(6)

        self._level_buttons = {}
        for level_key, level_label, _dir in PROBLEM_LEVELS:
            lvl_btn = QPushButton(level_label)
            lvl_btn.setObjectName("taskLevelBtn")
            lvl_btn.setCursor(Qt.PointingHandCursor)
            lvl_btn.setCheckable(True)
            lvl_btn.setFixedHeight(34)
            lvl_btn.clicked.connect(
                lambda checked=False, lk=level_key: self._switch_level(lk)
            )
            level_lay.addWidget(lvl_btn)
            self._level_buttons[level_key] = lvl_btn

        nav_lay.addWidget(self._level_switch)
        self._level_switch.hide()  # ховаємо, поки немає активної HTML-задачі з рівнями

        nav_lay.addStretch(1)

        left_vlay.addWidget(navbar)
        sep = QWidget(); sep.setObjectName("cardDivider"); sep.setFixedHeight(1)
        left_vlay.addWidget(sep)

        # Опис задачі (висока зона, змінює stretch при toggle)
        self._desc_panel = QWidget()
        self._desc_panel.setObjectName("taskPanel")
        desc_outer = QVBoxLayout(self._desc_panel)
        desc_outer.setContentsMargins(16, 10, 16, 0)  # однаковий лівий/правий з bottom_left

        # Рядок з кнопками перегляду тесту — видимий лише коли
        # _desc_stack показує дані конкретного тесту (а не умову задачі).
        self._test_view_bar = QWidget()
        self._test_view_bar.setObjectName("taskPanel")
        test_view_lay = QHBoxLayout(self._test_view_bar)
        test_view_lay.setContentsMargins(0, 0, 0, 6)
        test_view_lay.setSpacing(8)
        test_view_lay.addStretch(1)

        self._btn_copy_test = QPushButton("⧉  Копіювати вхідні дані")
        self._btn_copy_test.setObjectName("taskNavBtn")
        self._btn_copy_test.setCursor(Qt.PointingHandCursor)
        self._btn_copy_test.setFixedHeight(30)
        self._btn_copy_test.clicked.connect(self._copy_test_input)
        test_view_lay.addWidget(self._btn_copy_test)

        self._btn_close_test_view = QPushButton("✕  Закрити перегляд тесту")
        self._btn_close_test_view.setObjectName("taskNavBtn")
        self._btn_close_test_view.setCursor(Qt.PointingHandCursor)
        self._btn_close_test_view.setFixedHeight(30)
        self._btn_close_test_view.clicked.connect(self._close_test_view)
        test_view_lay.addWidget(self._btn_close_test_view)

        self._test_view_bar.hide()
        desc_outer.addWidget(self._test_view_bar)

        # Стек з двох рендерерів:
        #  - _desc_browser (QTextBrowser): простий текст (рівні, типізована система) — без JS
        #  - _desc_web (QWebEngineView): повний HTML+JS (HTML-умови задач, кнопка копіювання)
        self._desc_stack = QStackedWidget()
        desc_outer.addWidget(self._desc_stack)

        self._desc_browser = QTextBrowser()
        self._desc_browser.setObjectName("taskDescBrowser")
        self._desc_browser.setOpenExternalLinks(True)
        self._desc_browser.setFrameShape(QTextBrowser.NoFrame)
        self._desc_stack.addWidget(self._desc_browser)   # index 0

        self._desc_web = None
        if _WEBENGINE_AVAILABLE:
            self._desc_web = QWebEngineView()
            # Дозволяємо JS читати/писати в буфер обміну без діалогу підтвердження
            try:
                settings = self._desc_web.settings()
                settings.setAttribute(
                    QWebEngineSettings.WebAttribute.JavascriptCanAccessClipboard, True
                )
                settings.setAttribute(
                    QWebEngineSettings.WebAttribute.JavascriptCanPaste, True
                )
                # Автоматично дозволяти будь-які запити на доступ до буфера
                # обміну (деякі версії QtWebEngine питають дозвіл окремо
                # від налаштувань вище).
                page = self._desc_web.page()
                page.featurePermissionRequested.connect(self._grant_clipboard_permission)
            except Exception:
                pass
            self._desc_stack.addWidget(self._desc_web)   # index 1
            # Прогрів рушія одразу — щоб перше реальне відкриття не лагало
            self._desc_web.setHtml("<html><body></body></html>")

        left_vlay.addWidget(self._desc_panel, stretch=3)  # буде змінюватись

        sep2 = QWidget(); sep2.setObjectName("cardDivider"); sep2.setFixedHeight(1)
        left_vlay.addWidget(sep2)

        # Нижня ліва зона: кнопки + редактор
        bottom_left = QWidget()
        bottom_left.setObjectName("taskScreen")
        bottom_vlay = QVBoxLayout(bottom_left)
        bottom_vlay.setContentsMargins(16, 10, 16, 12)  # однаковий правий відступ з desc
        bottom_vlay.setSpacing(8)

        # Рядок: [Завантажити] файл
        upload_panel = QWidget()
        upload_panel.setObjectName("taskPanel")
        upload_lay = QHBoxLayout(upload_panel)
        upload_lay.setContentsMargins(10, 7, 10, 7)
        upload_lay.setSpacing(10)

        self._btn_upload = QPushButton("  Завантажити")
        self._btn_upload.setObjectName("taskUploadBtn")
        self._btn_upload.setCursor(Qt.PointingHandCursor)
        self._btn_upload.setFixedHeight(34)
        self._btn_upload.clicked.connect(self._pick_file)
        upload_lay.addWidget(self._btn_upload)

        self._file_name_lbl = QLabel("файл не обрано")
        self._file_name_lbl.setObjectName("taskFileNameLabel")
        self._file_name_lbl.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Preferred)
        upload_lay.addWidget(self._file_name_lbl, stretch=1)

        # ── Перемикач мови програми (праворуч) ──
        self._lang_combo = QComboBox()
        self._lang_combo.setObjectName("taskLangCombo")
        self._lang_combo.setCursor(Qt.PointingHandCursor)
        self._lang_combo.setFixedHeight(34)
        self._lang_combo.setMinimumWidth(130)
        for label, ext in SOLUTION_LANGUAGES:
            self._lang_combo.addItem(label, ext)
        default_idx = next(
            (i for i, (_, ext) in enumerate(SOLUTION_LANGUAGES) if ext == DEFAULT_LANGUAGE_EXT),
            0
        )
        self._lang_combo.setCurrentIndex(default_idx)
        self._lang_combo.currentIndexChanged.connect(self._on_language_changed)
        upload_lay.addWidget(self._lang_combo)

        bottom_vlay.addWidget(upload_panel, stretch=0)

        self._editor = QPlainTextEdit()
        self._editor.setObjectName("taskEditor")
        self._editor.setPlaceholderText("Введіть відповідь або код тут…")
        bottom_vlay.addWidget(self._editor, stretch=1)

        # Нижній рядок: [Здати] ... [ШІ] — в одному рядку
        bottom_row = QHBoxLayout()
        bottom_row.setSpacing(10)

        self._btn_submit = QPushButton("Здати")
        self._btn_submit.setObjectName("taskSubmitBtn")
        self._btn_submit.setCursor(Qt.PointingHandCursor)
        self._btn_submit.setFixedHeight(40)
        self._btn_submit.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        # Натискання "Здати" також відкриває список (якщо ще не відкритий)
        self._btn_submit.clicked.connect(self._submit_and_open)
        bottom_row.addWidget(self._btn_submit, stretch=25)

        bottom_row.addStretch(75)

        self._btn_ai = QPushButton("  ШІ")
        self._btn_ai.setObjectName("taskAiBtn")
        self._btn_ai.setCursor(Qt.PointingHandCursor)
        self._btn_ai.setFixedHeight(40)
        self._btn_ai.setFixedWidth(90)
        self._btn_ai.clicked.connect(self._on_ai_clicked)
        bottom_row.addWidget(self._btn_ai)

        bottom_vlay.addLayout(bottom_row)

        left_vlay.addWidget(bottom_left, stretch=2)

        # ════════ ПРАВА КОЛОНКА ════════
        self._right_w = QWidget()
        self._right_w.setObjectName("taskScreen")
        self._right_vlay = QVBoxLayout(self._right_w)
        self._right_vlay.setContentsMargins(0, 8, 16, 12)
        self._right_vlay.setSpacing(6)
        root_h.addWidget(self._right_w, stretch=0)

        # Кнопка toggle — помітна, зі стрілкою вліво (← відкрити)
        toggle_row = QHBoxLayout()
        toggle_row.setContentsMargins(0, 0, 0, 0)
        toggle_row.addStretch(1)
        self._btn_toggle = QPushButton()
        self._btn_toggle.setObjectName("taskToggleBtn")
        self._btn_toggle.setIcon(QIcon(make_back_icon(18)))   # стрілка вліво
        self._btn_toggle.setIconSize(QSize(18, 18))
        self._btn_toggle.setCursor(Qt.PointingHandCursor)
        self._btn_toggle.setFixedSize(42, 36)
        self._btn_toggle.clicked.connect(self._toggle_list)
        toggle_row.addWidget(self._btn_toggle)
        self._right_vlay.addLayout(toggle_row)

        # Заголовок (тільки при розгорнутому стані)
        self._lbl_results = QLabel("РЕЗУЛЬТАТИ")
        self._lbl_results.setObjectName("taskSectionLabel")
        self._right_vlay.addWidget(self._lbl_results)

        # Список — спочатку порожній, заповнюється лише після реального
        # запуску тестування (кнопка "Здати").
        self._list_widget = QListWidget()
        self._list_widget.setObjectName("taskListWidget")
        self._list_widget.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self._list_widget.itemClicked.connect(self._on_result_item_clicked)
        self._right_vlay.addWidget(self._list_widget, stretch=1)

        # Зберігаємо посилання на layout для зміни stretch
        self._left_vlay = left_vlay

        # Застосовуємо початковий стан
        self._apply_state()

    # ── Toggle ───────────────────────────────────
    def _grant_clipboard_permission(self, securityOrigin, feature):
        """Автоматично дозволяє JS-доступ до буфера обміну в умовах задач."""
        try:
            page = self._desc_web.page()
            page.setFeaturePermission(
                securityOrigin, feature,
                QWebEnginePage.PermissionPolicy.PermissionGrantedByUser
            )
        except Exception:
            pass

    def _toggle_list(self):
        self._list_expanded = not self._list_expanded
        self._apply_state()

    def _current_lang_ext(self) -> str:
        return self._lang_combo.currentData() or DEFAULT_LANGUAGE_EXT

    def _on_language_changed(self, _index: int):
        """Зміна мови — лише оновлює фільтр діалогу вибору файлу, нічого не запускає."""
        pass

    def _submit_and_open(self):
        """
        Здати:
          1. Відкриває список результатів (якщо закритий).
          2. Якщо задача має meta (конкретний алгоритм) — реально компілює й
             запускає відповідний тестер на коді з редактора (або на
             завантаженому файлі), показує результати по кожному тесту.
        """
        if not self._list_expanded:
            self._list_expanded = True
            self._apply_state()

        if self._task_meta is None:
            self._set_list_message("Тестування доступне лише для задач конкретного алгоритму.")
            return

        if self._test_thread is not None:
            return  # тестування вже триває

        solution_path, is_temp = self._prepare_solution_file()
        if solution_path is None:
            return

        subdir_key, load_filename = self._task_meta
        tester_cpp = get_tester_source_path(subdir_key, load_filename, self._cur_level_key)

        self._set_list_message("Компіляція тестера…")
        self._btn_submit.setEnabled(False)

        self._test_thread = _TestRunnerThread(tester_cpp, solution_path, is_temp, self)
        self._test_thread.finished_ok.connect(self._on_test_finished)
        self._test_thread.finished_error.connect(self._on_test_error)
        self._test_thread.start()

    def _prepare_solution_file(self):
        """
        Повертає (шлях_до_файлу_розвязку, is_temp).
        Якщо обрано файл через "Завантажити" — використовує його напряму.
        Інакше бере текст з редактора і зберігає у тимчасовий файл з
        розширенням відповідно до вибраної мови.
        Повертає (None, False) якщо немає що тестувати.
        """
        if self._picked_file_path and os.path.isfile(self._picked_file_path):
            return self._picked_file_path, False

        code = self._editor.toPlainText()
        if not code.strip():
            self._set_list_message("Спочатку введіть код у редактор або завантажте файл.")
            return None, False

        import tempfile
        ext = self._current_lang_ext()
        fd, tmp_path = tempfile.mkstemp(suffix=f".{ext}", prefix="solution_")
        with os.fdopen(fd, "w", encoding="utf-8") as f:
            f.write(code)
        return tmp_path, True

    def _read_solution_code(self) -> str:
        """
        Повертає код розв'язку як текст для ШІ-аналізу: з редактора, або
        з вибраного файлу, якщо текстове поле порожнє.
        """
        code = self._editor.toPlainText().strip()
        if code:
            return code
        if self._picked_file_path and os.path.isfile(self._picked_file_path):
            try:
                with open(self._picked_file_path, encoding="utf-8", errors="replace") as f:
                    return f.read()
            except OSError:
                return ""
        return ""

    def _on_ai_clicked(self):
        """
        Відкриває чат-вікно ШІ-аналізу для поточної задачі. Якщо API-ключ
        ще не налаштований — спершу показує діалог для його введення.
        """
        api_key = load_ai_api_key()
        if not api_key:
            dlg = ApiKeyDialog("", self)
            if dlg.exec() != QDialog.Accepted:
                return
            api_key = dlg.api_key()
            if not api_key:
                return
            save_ai_api_key(api_key)

        solution_code = self._read_solution_code()
        if not solution_code:
            self._set_list_message("Спочатку введіть код у редактор або завантажте файл.")
            return

        chat = AiChatDialog(api_key, self._build_ai_context, self)
        chat.exec()

    def _build_ai_context(self, reference_solution: str) -> str:
        """
        Формує текст першого повідомлення для ШІ: умова задачі (як plain
        text з HTML), повний код C++ тестера (щоб ШІ розумів, як саме
        перевіряється правильність — формат вводу/виводу, межі тестів
        тощо), код користувача, і, якщо заповнено, еталонний розв'язок.
        """
        lang_label = next(
            (lbl for lbl, e in SOLUTION_LANGUAGES if e == self._current_lang_ext()),
            self._current_lang_ext()
        )
        solution_code = self._read_solution_code()

        parts = [
            "Проаналізуй розв'язок задачі з програмування. Знайди помилки "
            "(логічні, синтаксичні, межові випадки) і поясни, чому розв'язок "
            "може не проходити тести. Відповідай українською мовою, стисло "
            "й по суті, з конкретними прикладами де можливо.\n"
        ]

        problem_text = ""
        if self._task_meta is not None:
            subdir_key, load_filename = self._task_meta
            html_path = get_problem_path(subdir_key, load_filename, self._cur_level_key)
            problem_text = html_to_plain_text(html_path)
        elif self._task_text:
            problem_text = self._task_text
        if problem_text:
            parts.append(f"=== УМОВА ЗАДАЧІ ===\n{problem_text}\n")

        if self._task_meta is not None:
            subdir_key, load_filename = self._task_meta
            tester_path = get_tester_source_path(subdir_key, load_filename, self._cur_level_key)
            if tester_path and os.path.isfile(tester_path):
                try:
                    with open(tester_path, encoding="utf-8", errors="replace") as f:
                        tester_code = f.read()
                    parts.append(
                        f"=== КОД ТЕСТЕРА (C++, показує точну логіку перевірки "
                        f"й формат вводу/виводу) ===\n{tester_code}\n"
                    )
                except OSError:
                    pass

        parts.append(f"=== РОЗВ'ЯЗОК КОРИСТУВАЧА (мова: {lang_label}) ===\n{solution_code}\n")

        if reference_solution:
            parts.append(f"=== ЕТАЛОННИЙ (ПРАВИЛЬНИЙ) РОЗВ'ЯЗОК ===\n{reference_solution}\n")

        return "\n".join(parts)

    def _set_list_message(self, text: str):
        """Показує одне інформаційне повідомлення замість списку результатів."""
        self._list_widget.clear()
        item = QListWidgetItem(text)
        self._list_widget.addItem(item)

    def _on_test_finished(self, result: dict, is_temp: bool, solution_path: str):
        self._test_thread = None
        self._btn_submit.setEnabled(True)
        if is_temp:
            try:
                os.remove(solution_path)
            except OSError:
                pass

        if not result.get("ok"):
            self._set_list_message(f"Помилка: {result.get('error', 'невідома помилка')}")
            return

        verdict = result.get("verdict", "ERROR")
        if verdict == "CE":
            raw = result.get("raw", "")
            self._list_widget.clear()
            self._list_widget.addItem(QListWidgetItem("[CE] Помилка компіляції розв'язку"))
            for line in raw.splitlines():
                if line.strip():
                    self._list_widget.addItem(QListWidgetItem(line))
            # Якщо це специфічно "не вдалось підготувати Python" — одразу
            # показуємо детальну діагностику (реальний тестовий запуск
            # обгортки python3 через MSYS-shell), щоб бачити факти, а не
            # повторювати гіпотези, що вже не підтвердились раніше.
            if "Помилка підготовки розв'язку (Python)" in raw and is_windows():
                self._list_widget.addItem(QListWidgetItem(""))
                self._list_widget.addItem(QListWidgetItem("── Діагностика python3 ──"))
                diag = diagnose_python3_wrapper()
                for line in diag.splitlines():
                    self._list_widget.addItem(QListWidgetItem(line))
            return

        tests = result.get("tests", [])
        self._last_test_data = result.get("test_data", {})
        self._last_raw_output = result.get("raw", "")
        self._list_widget.clear()
        header = QListWidgetItem(f"Результат: {verdict}   ({len(tests)} тестів)")
        self._list_widget.addItem(header)
        for idx, total, v, time_ms in tests:
            item = QListWidgetItem(f"Тест {idx}/{total}    [{v}]    {time_ms} мс")
            # OK — зелений, WA — червоний, решта (TL/RE/інше) — жовтий.
            if v == "OK":
                item.setForeground(QColor(COLORS_VERDICT_OK))
            elif v == "WA":
                item.setForeground(QColor(COLORS_VERDICT_WA))
            else:
                item.setForeground(QColor(COLORS_VERDICT_OTHER))
            # Зберігаємо номер тесту в самому item — використовується в
            # обробнику кліку для показу вхідних/очікуваних/фактичних даних.
            item.setData(Qt.UserRole, int(idx))
            self._list_widget.addItem(item)

    def _find_testdata_raw_context(self, idx: int) -> str:
        """
        ДІАГНОСТИКА: шукає в повному сирому виводі тестера рядок, що
        починається на "TESTDATA {idx} " (рівно цей номер тесту, не
        підрядок типу 13 всередині 130). Повертає сам рядок повністю
        (без обмеження довжини, бо саме довжина/вміст можуть пояснити
        чому regex його не підхопив), або повідомлення що рядок відсутній
        взагалі — це різні діагнози (regex не підійшов vs C++ не надрукував).
        """
        if not self._last_raw_output:
            return "(сирий вивід тестера не збережено)"
        target_prefix = f"TESTDATA {idx} "
        for line in self._last_raw_output.splitlines():
            stripped = line.rstrip("\r")
            if stripped == line:
                cr_note = ""
            else:
                cr_note = " [рядок містив символ \\r на кінці!]"
            if stripped.startswith(target_prefix):
                return f"Рядок знайдено в сирому виводі{cr_note}:\n{repr(line)}"
        return f"Рядок 'TESTDATA {idx} ...' взагалі відсутній у сирому виводі тестера."

    def _on_result_item_clicked(self, item: QListWidgetItem):
        """
        Клік по рядку результату — показує вхідні/очікувані/фактичні дані
        тесту. Працює для БУДЬ-ЯКОГО вердикту (OK, WA, TL, RE) однаково —
        перегляд вхідних даних не залежить від того, пройшов тест чи ні.
        """
        try:
            raw_idx = item.data(Qt.UserRole)
            if raw_idx is None:
                return  # клік по заголовку чи службовому рядку — ігноруємо
            idx = int(raw_idx)
            data = self._last_test_data.get(idx)
            if not data:
                # ДІАГНОСТИКА: показуємо чому саме немає даних, замість
                # мовчазного виходу — це дозволить побачити справжню
                # причину, якщо проблема повториться.
                available = sorted(self._last_test_data.keys())[:5]
                raw_around = self._find_testdata_raw_context(idx)
                self._desc_browser.setHtml(
                    f"<b>Діагностика:</b> дані для тесту {idx} не знайдено.<br>"
                    f"Тип idx: {type(raw_idx).__name__} → {type(idx).__name__}<br>"
                    f"Доступні ключі (перші 5): {available}<br>"
                    f"Всього ключів: {len(self._last_test_data)}<br><br>"
                    f"<b>Сирий текст навколо TESTDATA {idx} (якщо є):</b><br>"
                    f"<pre>{self._escape_html(raw_around)}</pre>"
                )
                self._desc_stack.setCurrentWidget(self._desc_browser)
                self._test_view_bar.show()
                return
            html = (
                f"<b>Тест {idx} — вхідні та вихідні дані</b><br><br>"
                f"<b>Вхідні дані:</b><br><pre>{self._escape_html(data['input'])}</pre><br>"
                f"<b>Очікуваний вивід:</b><br><pre>{self._escape_html(data['expected'])}</pre><br>"
                f"<b>Фактичний вивід:</b><br><pre>{self._escape_html(data['actual'])}</pre>"
            )
            self._desc_browser.setHtml(html)
            self._desc_stack.setCurrentWidget(self._desc_browser)
            self._viewing_test_idx = idx
            self._test_view_bar.show()
        except Exception as e:
            # ДІАГНОСТИКА: якщо тут впаде виняток (а не мовчазний return),
            # показуємо його прямо у вікні замість тихого нічого-не-робіть.
            import traceback
            err_text = traceback.format_exc()
            self._desc_browser.setHtml(
                f"<b>Помилка при відкритті тесту:</b><br><pre>{self._escape_html(err_text)}</pre>"
            )
            self._desc_stack.setCurrentWidget(self._desc_browser)
            self._test_view_bar.show()

    def _copy_test_input(self):
        """Копіює вхідні дані поточного переглянутого тесту в буфер обміну."""
        if self._viewing_test_idx is None:
            return
        data = self._last_test_data.get(self._viewing_test_idx)
        if not data:
            return
        QApplication.clipboard().setText(data["input"])
        # Короткий візуальний відгук — на 1.2 сек змінюємо текст кнопки.
        self._btn_copy_test.setText("✓  Скопійовано")
        QTimer.singleShot(1200, lambda: self._btn_copy_test.setText("⧉  Копіювати вхідні дані"))

    def _close_test_view(self):
        """Закриває перегляд тесту й повертає зону опису до умови задачі."""
        self._viewing_test_idx = None
        self._test_view_bar.hide()
        if self._task_meta is not None:
            self._load_level_problem()
        elif self._cur_problem_html_path is not None:
            self._load_html_into_desc(self._task_title, self._cur_problem_html_path)
        else:
            self._desc_browser.setHtml(
                f"<b>{self._task_title}</b><br><br>{self._task_text.replace(chr(10), '<br>')}"
            )
            self._desc_stack.setCurrentWidget(self._desc_browser)

    @staticmethod
    def _escape_html(text: str) -> str:
        return (
            text.replace("&", "&amp;").replace("<", "&lt;")
                .replace(">", "&gt;")
        )

    def _on_test_error(self, message: str, is_temp: bool, solution_path: str):
        self._test_thread = None
        self._btn_submit.setEnabled(True)
        if is_temp:
            try:
                os.remove(solution_path)
            except OSError:
                pass
        self._set_list_message(f"Помилка: {message}")

    def _apply_state(self):
        w = self.width() if self.width() > 0 else 1280

        if self._list_expanded:
            # Права колонка: ~28% ширини
            right_w = max(260, int(w * 0.28))
            self._right_w.setFixedWidth(right_w)
            # Прибираємо правий відступ лівої зони — він компенсований правою колонкою
            self._right_vlay.setContentsMargins(0, 8, 16, 12)
            self._list_widget.show()
            self._lbl_results.show()
            # Стрілка вправо → згорнути список
            self._btn_toggle.setIcon(QIcon(make_next_icon(18)))
            self._btn_toggle.setIconSize(QSize(18, 18))
            # Опис менший
            self._left_vlay.setStretch(
                self._left_vlay.indexOf(self._desc_panel), 2)
        else:
            # Права колонка вузька (тільки кнопка toggle)
            right_w = 58
            self._right_w.setFixedWidth(right_w)
            self._right_vlay.setContentsMargins(0, 8, 16, 12)
            self._list_widget.hide()
            self._lbl_results.hide()
            # Стрілка вліво ← відкрити список
            self._btn_toggle.setIcon(QIcon(make_back_icon(18)))
            self._btn_toggle.setIconSize(QSize(18, 18))
            # Опис більший
            self._left_vlay.setStretch(
                self._left_vlay.indexOf(self._desc_panel), 3)

        # Вирівнювання: права межа опису і редактора збігається
        # (обидва — діти bottom_left з однаковими margins, тому вже вирівняні)

    def resizeEvent(self, event: QResizeEvent):
        self._apply_state()
        super().resizeEvent(event)

    # ── Публічні методи ──────────────────────────
    def open_task(self, title: str, text: str):
        """Відкриває задачу з простим текстовим описом (рівнева/типізована системи)."""
        self._task_title = title
        self._task_text  = text
        self._task_meta  = None
        self._cur_problem_html_path = None
        self._viewing_test_idx = None
        self._test_view_bar.hide()
        self._level_switch.hide()
        self._desc_browser.setHtml(
            f"<b>{title}</b><br><br>{text.replace(chr(10), '<br>')}"
        )
        self._desc_stack.setCurrentWidget(self._desc_browser)
        self._editor.clear()
        self._file_name_lbl.setText("файл не обрано")
        self._picked_file_path = None
        self._list_expanded = False
        self._apply_state()

    def open_task_html(self, title: str, html_path: str):
        """
        Відкриває задачу, завантажуючи умову з HTML-файлу (без перемикача рівнів —
        напр. для "Випадкової задачі", де є лише один файл problem.html).

        Якщо доступний QWebEngineView — рендерить через нього (повна підтримка
        JS, тому кнопка "копіювати" в умові реально працює).
        Якщо QtWebEngine не встановлений — відкочується на QTextBrowser
        (умова відображається, але JS-кнопки бездіяльні).
        """
        self._task_title = title
        self._task_text  = ""
        self._task_meta  = None
        self._cur_problem_html_path = html_path
        self._viewing_test_idx = None
        self._test_view_bar.hide()
        self._level_switch.hide()
        self._load_html_into_desc(title, html_path)
        self._editor.clear()
        self._file_name_lbl.setText("файл не обрано")
        self._picked_file_path = None
        self._list_expanded = False
        self._apply_state()

    def open_task_with_levels(self, title: str, meta: tuple, default_level: str = "ez"):
        """
        Відкриває задачу конкретного алгоритму з перемикачем складності
        (Easy/Medium/Hard) у навбарі. meta = (subdir_key, load_filename).

        За потреби перемикання рівня користувач сам обирає складність задачі —
        це не залежить від розділу рівневої системи, з якого відкрита картка.
        """
        self._task_title = title
        self._task_text  = ""
        self._task_meta  = meta
        self._cur_level_key = default_level
        self._viewing_test_idx = None
        self._test_view_bar.hide()

        # Показуємо перемикач і виділяємо активну кнопку
        self._level_switch.show()
        for lk, btn in self._level_buttons.items():
            btn.setChecked(lk == default_level)

        self._load_level_problem()

        self._editor.clear()
        self._file_name_lbl.setText("файл не обрано")
        self._picked_file_path = None
        self._list_expanded = False
        self._apply_state()

    def _switch_level(self, level_key: str):
        """Викликається при натисканні кнопки рівня складності задачі."""
        if self._task_meta is None:
            return
        self._cur_level_key = level_key
        for lk, btn in self._level_buttons.items():
            btn.setChecked(lk == level_key)
        self._viewing_test_idx = None
        self._test_view_bar.hide()
        self._load_level_problem()

    def _load_level_problem(self):
        """Завантажує HTML-умову для self._task_meta + self._cur_level_key."""
        subdir_key, load_filename = self._task_meta
        html_path = get_problem_path(subdir_key, load_filename, self._cur_level_key)
        self._load_html_into_desc(self._task_title, html_path)

    def _load_html_into_desc(self, title: str, html_path: str):
        """Спільна логіка завантаження HTML у _desc_web/_desc_browser."""
        if not html_path or not os.path.isfile(html_path):
            self._desc_browser.setHtml(
                f"<b>{title}</b><br><br>[Файл не знайдено: {html_path}]"
            )
            self._desc_stack.setCurrentWidget(self._desc_browser)
        elif self._desc_web is not None:
            self._desc_web.setUrl(QUrl.fromLocalFile(html_path))
            self._desc_stack.setCurrentWidget(self._desc_web)
        else:
            # Фолбек без QtWebEngine
            self._desc_browser.setSource(QUrl.fromLocalFile(html_path))
            self._desc_stack.setCurrentWidget(self._desc_browser)

    def _pick_file(self):
        ext = self._current_lang_ext()
        lang_label = next((lbl for lbl, e in SOLUTION_LANGUAGES if e == ext), ext)
        filt = f"{lang_label} (*.{ext});;Всі файли (*)"
        path, _ = QFileDialog.getOpenFileName(self, "Оберіть файл розв'язку", "", filt)
        if path:
            self._picked_file_path = path
            self._file_name_lbl.setText(os.path.basename(path))


# ──────────────────────────────────────────────
#  ЕКРАН ТИПІЗОВАНОГО СОРТУВАННЯ
# ──────────────────────────────────────────────
TYPED_SORT_BUTTONS = [
    "Пошук та техніки на масивах",
    "Сортування",
    "Математика",
    "Жадібні алгоритми",
    "Графи",
    "Динамічне програмування",
    "Рядкові алгоритми",
    "Структури даних та дерева",
]

class TypedSortScreen(QWidget):
    """Сторінка 'Типізовано посортовані алгоритми' з 8 великими кнопками."""

    def __init__(self, on_home, on_algo=None, parent=None):
        super().__init__(parent)
        self.setObjectName("contentArea")
        self.setMouseTracking(True)

        self._ref_w = 960
        self._ref_h = 520
        self._buttons: list[QPushButton] = []
        self._on_algo = on_algo  # callable(title: str)

        root = QVBoxLayout(self)
        root.setContentsMargins(0, 0, 0, 0)
        root.setSpacing(0)

        # ── Навбар ────────────────────────────────
        self._navbar = QWidget()
        self._navbar.setObjectName("contentArea")
        nav_layout = QHBoxLayout(self._navbar)
        nav_layout.setContentsMargins(12, 8, 12, 4)
        nav_layout.setSpacing(8)

        btn_home = QPushButton("  На головну")
        btn_home.setObjectName("levelNavBtn")
        btn_home.setIcon(QIcon(make_home_icon(18)))
        btn_home.setIconSize(QSize(18, 18))
        btn_home.setCursor(Qt.PointingHandCursor)
        btn_home.clicked.connect(on_home)
        nav_layout.addWidget(btn_home)
        self._btn_home = btn_home

        nav_layout.addStretch(1)
        root.addWidget(self._navbar)

        sep1 = QWidget(); sep1.setObjectName("cardDivider"); sep1.setFixedHeight(1)
        root.addWidget(sep1)

        # ── Заголовок ─────────────────────────────
        self._title_lbl = QLabel("Типізовано посортовані алгоритми")
        self._title_lbl.setObjectName("levelTitleBig")
        self._title_lbl.setAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
        root.addWidget(self._title_lbl)

        sep2 = QWidget(); sep2.setObjectName("cardDivider"); sep2.setFixedHeight(1)
        root.addWidget(sep2)

        # ── Зона кнопок ───────────────────────────
        self._content = QWidget()
        self._content.setObjectName("contentArea")
        self._content.setMouseTracking(True)
        root.addWidget(self._content, stretch=1)

        self._outer = QVBoxLayout(self._content)
        self._outer.setContentsMargins(0, 0, 0, 0)
        self._outer.setSpacing(0)
        self._outer.addStretch(1)

        # Ряд 1 — 4 кнопки
        self._row0 = QHBoxLayout()
        self._row0.setSpacing(0)
        self._row0.setContentsMargins(0, 0, 0, 0)
        self._outer.addLayout(self._row0)

        self._row_spacer = QSpacerItem(0, 18, QSizePolicy.Minimum, QSizePolicy.Fixed)
        self._outer.addItem(self._row_spacer)

        # Ряд 2 — 4 кнопки
        self._row1 = QHBoxLayout()
        self._row1.setSpacing(0)
        self._row1.setContentsMargins(0, 0, 0, 0)
        self._outer.addLayout(self._row1)

        self._outer.addStretch(1)

        self._build_buttons()

    def _build_buttons(self):
        top = TYPED_SORT_BUTTONS[:4]
        bot = TYPED_SORT_BUTTONS[4:]

        self._add_sp(self._row0, 0)
        for i, label in enumerate(top):
            btn = QPushButton(label)
            btn.setObjectName("mainBtn")
            btn.setCursor(Qt.PointingHandCursor)
            btn.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Expanding)
            if self._on_algo:
                btn.clicked.connect(
                    lambda checked=False, lbl=label: self._on_algo(lbl)
                )
            self._row0.addWidget(btn)
            self._buttons.append(btn)
            if i < len(top) - 1:
                self._add_sp(self._row0, 0)
        self._add_sp(self._row0, 0)

        self._add_sp(self._row1, 0)
        for i, label in enumerate(bot):
            btn = QPushButton(label)
            btn.setObjectName("mainBtn")
            btn.setCursor(Qt.PointingHandCursor)
            btn.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Expanding)
            if self._on_algo:
                btn.clicked.connect(
                    lambda checked=False, lbl=label: self._on_algo(lbl)
                )
            self._row1.addWidget(btn)
            self._buttons.append(btn)
            if i < len(bot) - 1:
                self._add_sp(self._row1, 0)
        self._add_sp(self._row1, 0)

    @staticmethod
    def _add_sp(layout: QHBoxLayout, w: int):
        layout.addItem(QSpacerItem(w, 0, QSizePolicy.Fixed, QSizePolicy.Minimum))

    def resizeEvent(self, event: QResizeEvent):
        w = event.size().width()
        h = event.size().height()

        nav_btn_h = max(38, int(h * 0.055 * 1.1))
        nb_h = nav_btn_h + 12
        self._navbar.setFixedHeight(nb_h)
        self._btn_home.setFixedHeight(nav_btn_h)
        self._btn_home.setFixedWidth(max(130, int(w * 0.13 * 1.1)))

        title_h = max(60, int(h * 0.18))
        self._title_lbl.setFixedHeight(title_h)

        content_h = max(80, h - nb_h - title_h - 4)
        btn_h = max(36, int(content_h * 0.17))

        scale_x = w / self._ref_w
        pad_h = max(8, int(32 * scale_x))
        scale_y = content_h / self._ref_h
        pad_v = max(4, int(14 * scale_y))
        self._outer.setContentsMargins(pad_h, pad_v, pad_h, pad_v)

        usable_w = max(200, w - pad_h * 2)
        gap = max(6, int(14 * scale_x))
        btn_w = max(60, int((usable_w - gap * 3) / 4))

        for btn in self._buttons:
            btn.setFixedWidth(btn_w)
            btn.setFixedHeight(btn_h)

        # Оновити spacers в обох рядках
        for row in (self._row0, self._row1):
            spacers = []
            for i in range(row.count()):
                item = row.itemAt(i)
                if item and item.spacerItem():
                    spacers.append(item.spacerItem())
            if len(spacers) >= 2:
                spacers[0].changeSize(0, 0, QSizePolicy.Fixed, QSizePolicy.Minimum)
                for sp in spacers[1:-1]:
                    sp.changeSize(gap, 0, QSizePolicy.Fixed, QSizePolicy.Minimum)
                spacers[-1].changeSize(0, 0, QSizePolicy.Fixed, QSizePolicy.Minimum)
            row.invalidate()

        row_gap = max(8, int(18 * scale_y))
        self._row_spacer.changeSize(0, row_gap, QSizePolicy.Minimum, QSizePolicy.Fixed)
        self._outer.invalidate()

        super().resizeEvent(event)


# ──────────────────────────────────────────────

# ──────────────────────────────────────────────
#  ДАНІ КНОПОК ДЛЯ ПІДСТОРІНОК АЛГОРИТМІВ
# ──────────────────────────────────────────────
ALGO_BUTTONS = {
    "Пошук та техніки на масивах": [
        ("Лінійний пошук",           "Linear Search",
         _load("search", "linear_search"),
         _load_detail("search", "linear_search"), ("search", "linear_search")),
        ("Префіксні суми",           "Prefix Sums",
         _load("search", "prefix_sums"),
         _load_detail("search", "prefix_sums"), ("search", "prefix_sums")),
        ("Метод двох вказівників",   "Two Pointers",
         _load("search", "two_pointers"),
         _load_detail("search", "two_pointers"), ("search", "two_pointers")),
        ("Ковзне вікно",             "Sliding Window",
         _load("search", "sliding_window"),
         _load_detail("search", "sliding_window"), ("search", "sliding_window")),
        ("Бінарний пошук",           "Binary Search",
         _load("search", "binary_search"),
         _load_detail("search", "binary_search"), ("search", "binary_search")),
        ("Алгоритм Кадане",          "максимальна сума підмасиву",
         _load("search", "kadane"),
         _load_detail("search", "kadane"), ("search", "kadane")),
    ],
    "Сортування": [
        ("Сортування бульбашкою",    "Bubble Sort",
         _load("sort", "bubble_sort"),
         _load_detail("sort", "bubble_sort"), ("sort", "bubble_sort")),
        ("Сортування вибором",       "Selection Sort",
         _load("sort", "selection_sort"),
         _load_detail("sort", "selection_sort"), ("sort", "selection_sort")),
        ("Сортування вставками",     "Insertion Sort",
         _load("sort", "insertion_sort"),
         _load_detail("sort", "insertion_sort"), ("sort", "insertion_sort")),
        ("Сортування підрахунком",   "Counting Sort",
         _load("sort", "counting_sort"),
         _load_detail("sort", "counting_sort"), ("sort", "counting_sort")),
        ("Сортування злиттям",       "Merge Sort",
         _load("sort", "merge_sort"),
         _load_detail("sort", "merge_sort"), ("sort", "merge_sort")),
        ("Швидке сортування",        "Quick Sort",
         _load("sort", "quick_sort"),
         _load_detail("sort", "quick_sort"), ("sort", "quick_sort")),
        ("Пірамідальне сортування",  "Heap Sort",
         _load("sort", "heap_sort"),
         _load_detail("sort", "heap_sort"), ("sort", "heap_sort")),
    ],
    "Математика": [
        ("Алгоритм Евкліда",         "НСД, GCD",
         _load("math", "euclidean_gcd"),
         _load_detail("math", "euclidean_gcd"), ("math", "euclidean_gcd")),
        ("Швидке піднесення до степеня", "Binary Exponentiation",
         _load("math", "binary_exponentiation"),
         _load_detail("math", "binary_exponentiation"), ("math", "binary_exponentiation")),
        ("Решето Ератосфена",        "Sieve of Eratosthenes",
         _load("math", "sieve_of_eratosthenes"),
         _load_detail("math", "sieve_of_eratosthenes"), ("math", "sieve_of_eratosthenes")),
        ("Опукла оболонка",          "Convex Hull",
         _load("math", "convex_hull"),
         _load_detail("math", "convex_hull"), ("math", "convex_hull")),
    ],
    "Жадібні алгоритми": [
        ("Базова жадібність",        "розмін монет, вибір заявок",
         _load("greedy", "greedy_algorithms"),
         _load_detail("greedy", "greedy_algorithms"), ("greedy", "greedy_algorithms")),
        ("Мінімальне кістякове дерево", "Прім (Prim)",
         _load("greedy", "prim_mst"),
         _load_detail("greedy", "prim_mst"), ("greedy", "prim_mst")),
        ("Мінімальне кістякове дерево", "Краскал (Kruskal)",
         _load("greedy", "kruskal_mst"),
         _load_detail("greedy", "kruskal_mst"), ("greedy", "kruskal_mst")),
    ],
    "Графи": [
        ("Обхід у глибину",          "DFS",
         _load("graphs", "dfs"),
         _load_detail("graphs", "dfs"), ("graphs", "dfs")),
        ("Обхід у ширину",           "BFS",
         _load("graphs", "bfs"),
         _load_detail("graphs", "bfs"), ("graphs", "bfs")),
        ("Топологічне сортування",   "Topological Sort",
         _load("graphs", "topological_sort"),
         _load_detail("graphs", "topological_sort"), ("graphs", "topological_sort")),
        ("Алгоритм Дейкстри",        "",
         _load("graphs", "dijkstra"),
         _load_detail("graphs", "dijkstra"), ("graphs", "dijkstra")),
        ("Алгоритм Беллмана-Форда",  "",
         _load("graphs", "bellman_ford"),
         _load_detail("graphs", "bellman_ford"), ("graphs", "bellman_ford")),
        ("Алгоритм Флойда-Уоршелла", "",
         _load("graphs", "floyd_warshall"),
         _load_detail("graphs", "floyd_warshall"), ("graphs", "floyd_warshall")),
        ("Сильно зв'язні компоненти","Tarjan / Kosaraju",
         _load("graphs", "strongly_connected_components"),
         _load_detail("graphs", "strongly_connected_components"), ("graphs", "strongly_connected_components")),
        ("Максимальний потік",       "Форд-Фалкерсон / Едмондс-Карп",
         _load("graphs", "max_flow"),
         _load_detail("graphs", "max_flow"), ("graphs", "max_flow")),
    ],
    "Динамічне програмування": [
        ("Базове ДП",                "Фібоначчі, рюкзак",
         _load("dynamic", "dynamic_programming_basics"),
         _load_detail("dynamic", "dynamic_programming_basics"), ("dynamic", "dynamic_programming_basics")),
        ("Найдовша зростаюча підпослідовність", "LIS",
         _load("dynamic", "longest_increasing_subsequence"),
         _load_detail("dynamic", "longest_increasing_subsequence"), ("dynamic", "longest_increasing_subsequence")),
        ("Найдовша спільна підпослідовність",   "LCS",
         _load("dynamic", "longest_common_subsequence"),
         _load_detail("dynamic", "longest_common_subsequence"), ("dynamic", "longest_common_subsequence")),
        ("ДП на бітових масках",     "Bitmask DP",
         _load("dynamic", "bitmask_dp"),
         _load_detail("dynamic", "bitmask_dp"), ("dynamic", "bitmask_dp")),
    ],
    "Рядкові алгоритми": [
        ("Алгоритм Рабіна-Карпа",   "хешування",
         _load("string", "rabin_karp"),
         _load_detail("string", "rabin_karp"), ("string", "rabin_karp")),
        ("Z-функція",                "",
         _load("string", "z_function"),
         _load_detail("string", "z_function"), ("string", "z_function")),
        ("Алгоритм Кнута-Морріса-Пратта", "KMP",
         _load("string", "kmp"),
         _load_detail("string", "kmp"), ("string", "kmp")),
    ],
    "Структури даних та дерева": [
        ("Система неперетинних множин", "Union-Find / DSU",
         _load("trees", "disjoint_set_union"),
         _load_detail("trees", "disjoint_set_union"), ("trees", "disjoint_set_union")),
        ("Префіксне дерево",         "Trie",
         _load("trees", "trie"),
         _load_detail("trees", "trie"), ("trees", "trie")),
        ("Дерево Фенвіка",           "Fenwick Tree / BIT",
         _load("trees", "fenwick_tree"),
         _load_detail("trees", "fenwick_tree"), ("trees", "fenwick_tree")),
        ("Дерево відрізків",         "Segment Tree",
         _load("trees", "segment_tree"),
         _load_detail("trees", "segment_tree"), ("trees", "segment_tree")),
        ("Найменший спільний предок","LCA, binary lifting",
         _load("trees", "lowest_common_ancestor"),
         _load_detail("trees", "lowest_common_ancestor"), ("trees", "lowest_common_ancestor")),
    ],
}



# ──────────────────────────────────────────────
#  ПІДСТОРІНКА АЛГОРИТМУ (узагальнена)
# ──────────────────────────────────────────────
class AlgoScreen(QWidget):
    """
    Підсторінка для однієї категорії алгоритмів.
    Заголовок = назва категорії, кнопка ← повертає на TypedSortScreen.
    Кнопки розміщуються у рядках по MAX_PER_ROW штук.
    """
    MAX_PER_ROW = 4   # максимум кнопок в одному рядку

    def __init__(self, title: str, btn_labels: list, on_back, on_try=None, parent=None):
        super().__init__(parent)
        self.setObjectName("contentArea")
        self.setMouseTracking(True)

        self._ref_w      = 960
        self._ref_h      = 520
        self._btn_labels = btn_labels
        self._on_try     = on_try
        self._buttons:      list[QPushButton]  = []
        self._rows:         list[QHBoxLayout]  = []
        self._row_spacers:  list[QSpacerItem]  = []

        root = QVBoxLayout(self)
        root.setContentsMargins(0, 0, 0, 0)
        root.setSpacing(0)

        # ── Навбар ────────────────────────────────
        self._navbar = QWidget()
        self._navbar.setObjectName("contentArea")
        nav_layout = QHBoxLayout(self._navbar)
        nav_layout.setContentsMargins(12, 8, 12, 4)
        nav_layout.setSpacing(8)

        self._btn_back = QPushButton("  Назад")
        self._btn_back.setObjectName("levelNavBtn")
        self._btn_back.setIcon(QIcon(make_back_icon(16)))
        self._btn_back.setIconSize(QSize(16, 16))
        self._btn_back.setCursor(Qt.PointingHandCursor)
        self._btn_back.clicked.connect(on_back)
        nav_layout.addWidget(self._btn_back)
        nav_layout.addStretch(1)
        root.addWidget(self._navbar)

        sep1 = QWidget(); sep1.setObjectName("cardDivider"); sep1.setFixedHeight(1)
        root.addWidget(sep1)

        # ── Заголовок ─────────────────────────────
        self._title_lbl = QLabel(title)
        self._title_lbl.setObjectName("levelTitleBig")
        self._title_lbl.setAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
        root.addWidget(self._title_lbl)

        sep2 = QWidget(); sep2.setObjectName("cardDivider"); sep2.setFixedHeight(1)
        root.addWidget(sep2)

        # ── Зона кнопок ───────────────────────────
        self._content = QWidget()
        self._content.setObjectName("contentArea")
        root.addWidget(self._content, stretch=1)

        self._outer = QVBoxLayout(self._content)
        self._outer.setContentsMargins(0, 0, 0, 0)
        self._outer.setSpacing(0)
        self._outer.addStretch(1)

        self._build_buttons()
        self._outer.addStretch(1)

        # Overlay — як у LevelScreen, покриває весь екран
        self._overlay = CardOverlay(self, on_try=self._on_try)
        self._overlay.hide()

    def _build_buttons(self):
        """Будує кнопки з двома рядками тексту та overlay-карткою."""
        chunks = [self._btn_labels[i:i+self.MAX_PER_ROW]
                  for i in range(0, len(self._btn_labels), self.MAX_PER_ROW)]

        for r_idx, chunk in enumerate(chunks):
            row = QHBoxLayout()
            row.setSpacing(0)
            row.setContentsMargins(0, 0, 0, 0)

            if r_idx > 0:
                sp = QSpacerItem(0, 18, QSizePolicy.Minimum, QSizePolicy.Fixed)
                self._outer.addItem(sp)
                self._row_spacers.append(sp)

            self._outer.addLayout(row)
            self._rows.append(row)

            self._add_sp(row, 0)
            for b_idx, entry in enumerate(chunk):
                # entry — кортеж (main, sub, card_text, detail_text[, meta])
                meta = None
                if len(entry) == 5:
                    main_text, sub_text, card_text, detail_text, meta = entry
                elif len(entry) == 4:
                    main_text, sub_text, card_text, detail_text = entry
                elif len(entry) == 2:
                    main_text, sub_text = entry
                    card_text   = main_text
                    detail_text = ""
                else:
                    main_text   = entry[0]
                    sub_text    = ""
                    card_text   = main_text
                    detail_text = ""

                # Заголовок картки = основний текст + підпис
                card_title = f"{main_text}" + (f"\n({sub_text})" if sub_text else "")

                btn = QPushButton()
                btn.setObjectName("mainBtn")
                btn.setCursor(Qt.PointingHandCursor)
                btn.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Expanding)
                btn.clicked.connect(
                    lambda checked=False, t=card_title, tx=card_text, dt=detail_text, mt=meta:
                    self._overlay.show_card(t, tx, dt, mt)
                )

                # Вертикальний layout всередині кнопки
                btn_lay = QVBoxLayout(btn)
                btn_lay.setContentsMargins(8, 6, 8, 6)
                btn_lay.setSpacing(3)

                lbl_main = QLabel(main_text)
                lbl_main.setAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
                lbl_main.setWordWrap(True)
                lbl_main.setObjectName("algoBtnMain")
                lbl_main.setAttribute(Qt.WA_TransparentForMouseEvents, True)
                btn_lay.addWidget(lbl_main, stretch=2)

                if sub_text:
                    lbl_sub = QLabel(sub_text)
                    lbl_sub.setAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
                    lbl_sub.setWordWrap(True)
                    lbl_sub.setObjectName("algoBtnSub")
                    lbl_sub.setAttribute(Qt.WA_TransparentForMouseEvents, True)
                    btn_lay.addWidget(lbl_sub, stretch=1)

                row.addWidget(btn)
                self._buttons.append(btn)
                if b_idx < len(chunk) - 1:
                    self._add_sp(row, 0)
            self._add_sp(row, 0)

    @staticmethod
    def _add_sp(layout: QHBoxLayout, w: int):
        layout.addItem(QSpacerItem(w, 0, QSizePolicy.Fixed, QSizePolicy.Minimum))

    def resizeEvent(self, event: QResizeEvent):
        w = event.size().width()
        h = event.size().height()

        # Навбар
        nav_btn_h = max(38, int(h * 0.055 * 1.1))
        nb_h = nav_btn_h + 12
        self._navbar.setFixedHeight(nb_h)
        self._btn_back.setFixedHeight(nav_btn_h)
        self._btn_back.setFixedWidth(max(110, int(w * 0.10)))

        # Заголовок
        title_h = max(60, int(h * 0.18))
        self._title_lbl.setFixedHeight(title_h)

        # Кнопки
        content_h = max(80, h - nb_h - title_h - 4)
        btn_h = max(36, int(content_h * 0.17))

        scale_x = w / self._ref_w
        pad_h   = max(8, int(32 * scale_x))
        scale_y = content_h / self._ref_h
        pad_v   = max(4, int(14 * scale_y))
        self._outer.setContentsMargins(pad_h, pad_v, pad_h, pad_v)

        usable_w = max(200, w - pad_h * 2)
        gap = max(6, int(14 * scale_x))

        # Ширина кнопки — за найдовшим рядком (MAX_PER_ROW)
        max_in_row = self.MAX_PER_ROW
        btn_w = max(60, int((usable_w - gap * (max_in_row - 1)) / max_in_row))

        for btn in self._buttons:
            btn.setFixedWidth(btn_w)
            btn.setFixedHeight(btn_h)

        # Оновити spacers у кожному рядку
        for row in self._rows:
            spacers = [row.itemAt(i).spacerItem()
                       for i in range(row.count())
                       if row.itemAt(i) and row.itemAt(i).spacerItem()]
            if len(spacers) >= 2:
                spacers[0].changeSize(0, 0, QSizePolicy.Fixed, QSizePolicy.Minimum)
                for sp in spacers[1:-1]:
                    sp.changeSize(gap, 0, QSizePolicy.Fixed, QSizePolicy.Minimum)
                spacers[-1].changeSize(0, 0, QSizePolicy.Fixed, QSizePolicy.Minimum)
            row.invalidate()

        # Spacers між рядками
        row_gap = max(8, int(18 * scale_y))
        for sp in self._row_spacers:
            sp.changeSize(0, row_gap, QSizePolicy.Minimum, QSizePolicy.Fixed)
        self._outer.invalidate()

        self._overlay.resize(event.size())
        super().resizeEvent(event)
# ──────────────────────────────────────────────
class TitleScreen(QWidget):
    """Стартова сторінка з трьома кнопками."""

    def __init__(self, on_level_system, on_random=None, on_typed=None, parent=None):
        super().__init__(parent)
        self.setObjectName("titleScreenArea")
        self.setAttribute(Qt.WA_StyledBackground, True)

        root = QVBoxLayout(self)
        root.setContentsMargins(0, 0, 0, 0)
        root.setSpacing(0)

        root.addStretch(2)

        title_lbl = QLabel("Title")
        title_lbl.setObjectName("titleScreenLabel")
        title_lbl.setAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
        root.addWidget(title_lbl)

        root.addStretch(1)

        big_row = QHBoxLayout()
        big_row.setContentsMargins(0, 0, 0, 0)
        big_row.setSpacing(0)
        big_row.addStretch(1)

        self._btn_level = QPushButton("Рівнева система")
        self._btn_level.setObjectName("titleBtnPrimary")
        self._btn_level.setCursor(Qt.PointingHandCursor)
        self._btn_level.clicked.connect(on_level_system)
        big_row.addWidget(self._btn_level)

        big_row.addStretch(1)

        self._btn_typed = QPushButton("Типізоване сортування")
        self._btn_typed.setObjectName("titleBtnPrimary")
        self._btn_typed.setCursor(Qt.PointingHandCursor)
        if on_typed:
            self._btn_typed.clicked.connect(on_typed)
        big_row.addWidget(self._btn_typed)

        big_row.addStretch(1)
        root.addLayout(big_row)

        root.addStretch(1)

        small_row = QHBoxLayout()
        small_row.setContentsMargins(0, 0, 0, 0)
        small_row.addStretch(1)

        self._btn_random = QPushButton("Випадкова задача")
        self._btn_random.setObjectName("titleBtnSmall")
        self._btn_random.setCursor(Qt.PointingHandCursor)
        if on_random:
            self._btn_random.clicked.connect(on_random)
        small_row.addWidget(self._btn_random)

        small_row.addStretch(1)
        root.addLayout(small_row)

        root.addStretch(2)

    def resizeEvent(self, event: QResizeEvent):
        w = event.size().width()
        h = event.size().height()
        big_w = max(200, int(w * 0.28))
        big_h = max(64,  int(h * 0.12))
        self._btn_level.setFixedSize(big_w, big_h)
        self._btn_typed.setFixedSize(big_w, big_h)
        sm_w = max(160, int(w * 0.18))
        sm_h = max(42,  int(h * 0.07))
        self._btn_random.setFixedSize(sm_w, sm_h)
        super().resizeEvent(event)

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)
        grad = QLinearGradient(0, 0, 0, self.height())
        grad.setColorAt(0.0, QColor(COLORS["bg_main"]))
        grad.setColorAt(1.0, QColor("#0a1624"))
        painter.fillRect(self.rect(), grad)
        super().paintEvent(event)


# ──────────────────────────────────────────────
#  ДІАЛОГ ПЕРЕВІРКИ КОМПІЛЯТОРІВ
# ──────────────────────────────────────────────
class CompilerCheckDialog(QDialog):
    """
    З'являється при старті програми, якщо g++ (обов'язковий — сам тестер
    написаний на C++) або інші мовні інструменти відсутні в PATH.

    Показує список відсутніх мов і (на Windows) кнопку для автоматичного
    встановлення MSYS2 + g++ через winget/pacman з підвищеними правами.
    """

    def __init__(self, missing: list, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Перевірка компіляторів")
        self.setModal(True)
        self.setMinimumSize(560, 420)
        self.setObjectName("compilerDialog")
        self.setAttribute(Qt.WA_StyledBackground, True)
        self._proc = None

        root = QVBoxLayout(self)
        root.setContentsMargins(24, 22, 24, 20)
        root.setSpacing(14)

        title = QLabel("Відсутні компілятори/інтерпретатори")
        title.setObjectName("compilerDialogTitle")
        root.addWidget(title)

        missing_ext = {ext for _, ext in missing}
        # Окрема перевірка: чи здатний наявний g++ зібрати САМ ТЕСТЕР
        # (потребує fork()/waitpid() — на Windows це лише MSYS-g++,
        # не ucrt64/mingw64; звичайна перевірка PATH цього не розрізняє).
        tester_gpp_ok = is_msys_gpp_available()

        if not tester_gpp_ok:
            if is_windows():
                warn = QLabel(
                    "⚠ Не знайдено компілятор, здатний зібрати тестер.\n"
                    "Сам тестер написаний на C++ і використовує fork()/waitpid()\n"
                    "(POSIX) — на Windows це потребує САМЕ MSYS-g++\n"
                    "(C:\\msys64\\usr\\bin\\g++.exe), а не звичайний MinGW/UCRT64\n"
                    "g++, навіть якщо той вже встановлений і є в PATH."
                )
            else:
                warn = QLabel(
                    "⚠ g++ не знайдено. Це критично — сам тестер написаний\n"
                    "на C++ і не зможе скомпілюватись без g++, незалежно від того,\n"
                    "якою мовою написаний розв'язок користувача."
                )
        else:
            warn = QLabel(
                "g++ для тестера знайдено — тестування буде працювати.\n"
                "Нижче перелічені мови розв'язків, які наразі недоступні\n"
                "(їх можна додати пізніше)."
            )
        warn.setObjectName("compilerDialogText")
        warn.setWordWrap(True)
        root.addWidget(warn)

        names = ", ".join(label for label, _ in missing) if missing else "—"
        lbl_list = QLabel(f"Не знайдено: {names}")
        lbl_list.setObjectName("compilerDialogList")
        lbl_list.setWordWrap(True)
        root.addWidget(lbl_list)

        # Лог встановлення (прихований, поки не натиснута кнопка)
        self._log = QPlainTextEdit()
        self._log.setObjectName("compilerDialogLog")
        self._log.setReadOnly(True)
        self._log.hide()
        root.addWidget(self._log, stretch=1)

        root.addStretch(0)

        btn_row = QHBoxLayout()
        btn_row.setSpacing(10)

        self._btn_install = QPushButton("  Встановити g++ автоматично (потрібні права адміна)")
        self._btn_install.setObjectName("compilerInstallBtn")
        self._btn_install.setCursor(Qt.PointingHandCursor)
        self._btn_install.setFixedHeight(40)
        if not is_windows():
            self._btn_install.setEnabled(False)
            self._btn_install.setText("  Автовстановлення підтримується лише на Windows")
        else:
            self._btn_install.clicked.connect(self._start_install)
        btn_row.addWidget(self._btn_install, stretch=1)

        self._btn_close = QPushButton("Продовжити без встановлення")
        self._btn_close.setObjectName("compilerCloseBtn")
        self._btn_close.setCursor(Qt.PointingHandCursor)
        self._btn_close.setFixedHeight(40)
        self._btn_close.clicked.connect(self.accept)
        btn_row.addWidget(self._btn_close)

        root.addLayout(btn_row)

        # Кнопка "Перевірити знову" — з'являється тільки після спроби встановлення.
        self._btn_recheck = QPushButton("Перевірити ще раз (без перезапуску)")
        self._btn_recheck.setObjectName("compilerCloseBtn")
        self._btn_recheck.setCursor(Qt.PointingHandCursor)
        self._btn_recheck.setFixedHeight(36)
        self._btn_recheck.clicked.connect(self._recheck)
        self._btn_recheck.hide()
        root.addWidget(self._btn_recheck)

    def _refresh_path_from_registry(self):
        """
        Підтягує оновлений PATH користувача й системи з реєстру Windows у
        поточний процес — щоб новий g++ було видно без повного перезапуску
        програми (новий процес встановлення міняє лише реєстр, не os.environ
        вже запущеної програми).
        """
        if not is_windows():
            return
        try:
            import winreg
            def read_path(root, subkey):
                with winreg.OpenKey(root, subkey) as key:
                    value, _ = winreg.QueryValueEx(key, "Path")
                    return value

            user_path = read_path(winreg.HKEY_CURRENT_USER, "Environment")
            try:
                sys_path = read_path(
                    winreg.HKEY_LOCAL_MACHINE,
                    r"SYSTEM\CurrentControlSet\Control\Session Manager\Environment"
                )
            except OSError:
                sys_path = ""
            merged = ";".join(p for p in [sys_path, user_path] if p)
            os.environ["PATH"] = merged
        except Exception:
            pass

    def _recheck(self):
        self._refresh_path_from_registry()
        if is_msys_gpp_available():
            self._append_log("MSYS-g++ знайдено! Тестування буде працювати — можете закрити це вікно.")
            self._btn_close.setText("Продовжити")
        else:
            self._append_log(
                "MSYS-g++ (C:\\msys64\\usr\\bin\\g++.exe) й досі не знайдено. "
                "Якщо встановлення щойно завершилось, спробуйте ще раз через кілька секунд."
            )

    def _start_install(self):
        self._log.show()
        self._log.clear()
        self._btn_install.setEnabled(False)
        self._btn_install.setText("  Встановлення триває…")
        self._append_log("Запуск встановлення MSYS2 + g++ (буде запит UAC)…")

        def on_line(line: str):
            self._append_log(line)

        def on_done(success: bool, message: str):
            self._append_log(message)
            self._btn_install.setText(
                "  Готово — натисніть 'Перевірити ще раз'" if success else "  Спробувати ще раз"
            )
            self._btn_install.setEnabled(True)
            self._btn_recheck.show()

        self._proc = install_gpp_windows_async(on_line, on_done)

    def _append_log(self, text: str):
        self._log.appendPlainText(text)


# ──────────────────────────────────────────────
#  ГОЛОВНЕ ВІКНО
# ──────────────────────────────────────────────
class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowFlags(Qt.FramelessWindowHint)
        self.setMinimumSize(500, 260)
        self.setStyleSheet(STYLESHEET)
        self.setMouseTracking(True)

        self._drag_pos           = QPoint()
        self._resize_edge        = EDGE_NONE
        self._resizing           = False
        self._resize_start_geom  = QRect()
        self._resize_start_mouse = QPoint()

        self._build_ui()
        self.showFullScreen()

    def _build_ui(self):
        central = QWidget()
        central.setObjectName("centralWidget")
        central.setMouseTracking(True)
        self.setCentralWidget(central)

        root = QVBoxLayout(central)
        root.setContentsMargins(0, 0, 0, 0)
        root.setSpacing(0)
        root.addWidget(self._build_titlebar())

        # ── Стек сторінок ───────────────────────
        self._stack = QStackedWidget()
        self._stack.setObjectName("contentArea")

        self._IDX_TITLE  = 0
        self._IDX_EASY   = 1
        self._IDX_MEDIUM = 2
        self._IDX_HARD   = 3
        self._IDX_INSANE = 4
        self._IDX_TASK   = 5
        self._IDX_TYPED  = 6
        # 8 підсторінок алгоритмів: індекси 7..14
        self._IDX_ALGO   = {
            name: 7 + i for i, name in enumerate(TYPED_SORT_BUTTONS)
        }

        # Сторінка завдання (спільна для всіх рівнів і випадкової задачі)
        self._task_screen = TaskScreen(
            on_back=self._task_go_back,
            parent=self._stack
        )
        # Запам'ятовуємо звідки прийшли на сторінку завдання
        self._task_prev_idx = self._IDX_TITLE

        def go_task(title: str, text: str, from_idx: int, meta=None):
            """
            meta — (subdir_key, load_filename) або None.
            Якщо meta доступне — відкриваємо HTML-умову з перемикачем
            складності задачі (Easy/Medium/Hard). Якщо немає (старий
            текстовий формат) — відкочуємось на простий текст.
            """
            self._task_prev_idx = from_idx
            if meta is not None:
                self._task_screen.open_task_with_levels(title, meta)
            else:
                self._task_screen.open_task(title, text)
            self._stack.setCurrentIndex(self._IDX_TASK)

        def go_task_html(title: str, html_path: str, from_idx: int):
            self._task_prev_idx = from_idx
            self._task_screen.open_task_html(title, html_path)
            self._stack.setCurrentIndex(self._IDX_TASK)

        # Титульна сторінка
        self._title_screen = TitleScreen(
            on_level_system=lambda: self._stack.setCurrentIndex(self._IDX_EASY),
            on_random=lambda: go_task_html(
                "Випадкова задача",
                PROBLEM_HTML_PATH,
                self._IDX_TITLE
            ),
            on_typed=lambda: self._stack.setCurrentIndex(self._IDX_TYPED),
            parent=self._stack
        )

        # Сторінка типізованого сортування
        self._typed_screen = TypedSortScreen(
            on_home=lambda: self._stack.setCurrentIndex(self._IDX_TITLE),
            on_algo=lambda name: self._stack.setCurrentIndex(self._IDX_ALGO[name]),
            parent=self._stack
        )

        # 8 підсторінок алгоритмів
        self._algo_screens = {}
        for name in TYPED_SORT_BUTTONS:
            from_idx = self._IDX_ALGO[name]
            screen = AlgoScreen(
                title=name,
                btn_labels=ALGO_BUTTONS[name],
                on_back=lambda: self._stack.setCurrentIndex(self._IDX_TYPED),
                on_try=lambda t, tx, mt, fi=from_idx: go_task(t, tx, fi, mt),
                parent=self._stack
            )
            self._algo_screens[name] = screen

        # Чотири екрани рівнів
        self._easy_screen = LevelScreen(
            level_title="Easy",
            rows0=EASY_ROW0, rows1=EASY_ROW1,
            on_home=lambda: self._stack.setCurrentIndex(self._IDX_TITLE),
            on_prev=None,
            on_next=lambda: self._stack.setCurrentIndex(self._IDX_MEDIUM),
            on_try=lambda t, tx, mt: go_task(t, tx, self._IDX_EASY, mt),
            parent=self._stack
        )
        self._medium_screen = LevelScreen(
            level_title="Medium",
            rows0=MEDIUM_ROW0, rows1=MEDIUM_ROW1,
            on_home=lambda: self._stack.setCurrentIndex(self._IDX_TITLE),
            on_prev=lambda: self._stack.setCurrentIndex(self._IDX_EASY),
            on_next=lambda: self._stack.setCurrentIndex(self._IDX_HARD),
            on_try=lambda t, tx, mt: go_task(t, tx, self._IDX_MEDIUM, mt),
            parent=self._stack
        )
        self._hard_screen = LevelScreen(
            level_title="Hard",
            rows0=HARD_ROW0, rows1=HARD_ROW1,
            on_home=lambda: self._stack.setCurrentIndex(self._IDX_TITLE),
            on_prev=lambda: self._stack.setCurrentIndex(self._IDX_MEDIUM),
            on_next=lambda: self._stack.setCurrentIndex(self._IDX_INSANE),
            on_try=lambda t, tx, mt: go_task(t, tx, self._IDX_HARD, mt),
            parent=self._stack
        )
        self._insane_screen = LevelScreen(
            level_title="Insane",
            rows0=INSANE_ROW0, rows1=INSANE_ROW1,
            on_home=lambda: self._stack.setCurrentIndex(self._IDX_TITLE),
            on_prev=lambda: self._stack.setCurrentIndex(self._IDX_HARD),
            on_next=None,
            on_try=lambda t, tx, mt: go_task(t, tx, self._IDX_INSANE, mt),
            parent=self._stack
        )

        self._stack.addWidget(self._title_screen)    # 0
        self._stack.addWidget(self._easy_screen)     # 1
        self._stack.addWidget(self._medium_screen)   # 2
        self._stack.addWidget(self._hard_screen)     # 3
        self._stack.addWidget(self._insane_screen)   # 4
        self._stack.addWidget(self._task_screen)     # 5
        self._stack.addWidget(self._typed_screen)    # 6
        for name in TYPED_SORT_BUTTONS:              # 7–14
            self._stack.addWidget(self._algo_screens[name])
        self._stack.setCurrentIndex(self._IDX_TITLE)

        root.addWidget(self._stack, stretch=1)

    def _task_go_back(self):
        """Повернення зі сторінки завдання туди, звідки прийшли."""
        self._stack.setCurrentIndex(self._task_prev_idx)

    def _build_titlebar(self):
        bar = QWidget()
        bar.setObjectName("titleBar")
        bar.setFixedHeight(40)
        bar.setMouseTracking(True)

        layout = QHBoxLayout(bar)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(0)

        title = QLabel("  MyApp")
        title.setObjectName("appTitle")
        layout.addWidget(title)
        layout.addStretch()

        btn_min = QPushButton("_")
        btn_min.setObjectName("sysBtn")
        btn_min.setCursor(Qt.PointingHandCursor)
        btn_min.clicked.connect(self.showMinimized)
        layout.addWidget(btn_min)

        self.btn_max = QPushButton("[ ]")
        self.btn_max.setObjectName("sysBtn")
        self.btn_max.setCursor(Qt.PointingHandCursor)
        self.btn_max.clicked.connect(self._toggle_maximize)
        layout.addWidget(self.btn_max)

        btn_close = QPushButton("X")
        btn_close.setObjectName("closeBtn")
        btn_close.setCursor(Qt.PointingHandCursor)
        btn_close.clicked.connect(self.close)
        layout.addWidget(btn_close)

        bar.mousePressEvent       = self._bar_press
        bar.mouseMoveEvent        = self._bar_move
        bar.mouseDoubleClickEvent = self._bar_dbl

        return bar

    def _get_edge(self, pos: QPoint) -> int:
        x, y = pos.x(), pos.y()
        w, h = self.width(), self.height()
        m    = EDGE_MARGIN
        on_left   = x <= m
        on_right  = x >= w - m
        on_top    = y <= m
        on_bottom = y >= h - m
        if on_top    and on_left:  return EDGE_TOP_LEFT
        if on_top    and on_right: return EDGE_TOP_RIGHT
        if on_bottom and on_left:  return EDGE_BOT_LEFT
        if on_bottom and on_right: return EDGE_BOT_RIGHT
        if on_left:                return EDGE_LEFT
        if on_right:               return EDGE_RIGHT
        if on_top:                 return EDGE_TOP
        if on_bottom:              return EDGE_BOTTOM
        return EDGE_NONE

    def mouseMoveEvent(self, event):
        if self.isFullScreen() or self.isMaximized():
            return super().mouseMoveEvent(event)
        if self._resizing:
            self._do_resize(event.globalPosition().toPoint())
            return
        edge = self._get_edge(event.position().toPoint())
        self.setCursor(CURSOR_MAP.get(edge, Qt.ArrowCursor))
        super().mouseMoveEvent(event)

    def mousePressEvent(self, event):
        if self.isFullScreen() or self.isMaximized():
            return super().mousePressEvent(event)
        if event.button() == Qt.LeftButton:
            edge = self._get_edge(event.position().toPoint())
            if edge != EDGE_NONE:
                self._resizing           = True
                self._resize_edge        = edge
                self._resize_start_geom  = self.geometry()
                self._resize_start_mouse = event.globalPosition().toPoint()
                return
        super().mousePressEvent(event)

    def mouseReleaseEvent(self, event):
        if event.button() == Qt.LeftButton:
            self._resizing    = False
            self._resize_edge = EDGE_NONE
            self.setCursor(Qt.ArrowCursor)
        super().mouseReleaseEvent(event)

    def _do_resize(self, gpos: QPoint):
        dx   = gpos.x() - self._resize_start_mouse.x()
        dy   = gpos.y() - self._resize_start_mouse.y()
        g    = QRect(self._resize_start_geom)
        mn   = self.minimumSize()
        edge = self._resize_edge
        if edge in (EDGE_RIGHT,  EDGE_TOP_RIGHT, EDGE_BOT_RIGHT):
            g.setWidth(max(mn.width(),   g.width()  + dx))
        if edge in (EDGE_BOTTOM, EDGE_BOT_LEFT,  EDGE_BOT_RIGHT):
            g.setHeight(max(mn.height(), g.height() + dy))
        if edge in (EDGE_LEFT,   EDGE_TOP_LEFT,  EDGE_BOT_LEFT):
            new_w = max(mn.width(),  g.width()  - dx)
            g.setLeft(g.right() - new_w)
        if edge in (EDGE_TOP,    EDGE_TOP_LEFT,  EDGE_TOP_RIGHT):
            new_h = max(mn.height(), g.height() - dy)
            g.setTop(g.bottom() - new_h)
        self.setGeometry(g)

    def _bar_press(self, event):
        if event.button() == Qt.LeftButton:
            self._drag_pos = (
                event.globalPosition().toPoint() - self.frameGeometry().topLeft()
            )

    def _bar_move(self, event):
        if event.buttons() == Qt.LeftButton and not self.isFullScreen():
            if self.isMaximized():
                self.showNormal()
            self.move(event.globalPosition().toPoint() - self._drag_pos)

    def _bar_dbl(self, event):
        if event.button() == Qt.LeftButton:
            self._toggle_maximize()

    def _toggle_maximize(self):
        if self.isFullScreen() or self.isMaximized():
            self.showNormal()
        else:
            self.showMaximized()

    def keyPressEvent(self, event):
        # Esc більше не використовується для навігації
        super().keyPressEvent(event)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    app.setStyle("Fusion")
    app.setStyleSheet(STYLESHEET)

    # Перевірка компіляторів при старті — показуємо діалог якщо:
    #  а) чогось не вистачає для мов розв'язків користувача, АБО
    #  б) сам тестер не зможе скомпілюватись (на Windows — потрібен
    #     специфічно MSYS-g++, а не будь-який g++ з PATH).
    missing = missing_languages()
    if missing or not is_msys_gpp_available():
        dlg = CompilerCheckDialog(missing)
        dlg.exec()

    window = MainWindow()
    sys.exit(app.exec())