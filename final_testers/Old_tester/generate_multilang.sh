#!/usr/bin/env bash
# generate_multilang.sh
# Для кожного existing_tester.cpp створює existing_tester_multilang.cpp поряд,
# замінюючи:
#   #include "../../common/tester_common.hpp"  ->  #include "../../common/language.hpp"
#   #include "../../common/tester_main.inc"    ->  #include "../../common/tester_main_multilang.inc"
# Решта файлу (генератор тестів, еталонний розв'язок) лишається незмінною,
# бо вона мовно-незалежна - стосується лише того, ЯК генеруються тести,
# а не якою мовою написано розв'язок користувача.

set -e
cd "$(dirname "$0")"

count=0
for f in $(find . -name "*_tester.cpp" ! -name "*_multilang.cpp"); do
    out="${f%.cpp}_multilang.cpp"
    sed -e 's#common/tester_common\.hpp#common/language.hpp#' \
        -e 's#common/tester_main\.inc#common/tester_main_multilang.inc#' \
        "$f" > "$out"
    count=$((count+1))
done

echo "Створено $count мультимовних тестерів (*_tester_multilang.cpp)"
