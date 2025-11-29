// libs/sum.c

#include "../header2.3.h"

double sum_va(int count, ...) {
    if (count <= 0) return 0.0;

    va_list args;
    va_start(args, count);

    double total = 0.0;
    for (int i = 0; i < count; i++) {
        total += va_arg(args, double);
    }

    va_end(args);
    return total;
}

// Экспортируемые символы для плагина
/*const char* plugin_name = "Сумма N чисел";
const int plugin_min_args = 1;
OperationFunc plugin_func = sum_va;*/

// 1. ИСПРАВЛЕНИЕ: Это должна быть функция, возвращающая const char*
const char* plugin_name() {
    return "Сумма N чисел";
}

// 2. ИСПРАВЛЕНИЕ: Это должна быть функция, возвращающая int
int plugin_min_args() {
    return 1;
}

// 3. ИСПРАВЛЕНИЕ: Это должна быть функция, возвращающая указатель на функцию
OperationFunc plugin_func() {
    return sum_va;
}