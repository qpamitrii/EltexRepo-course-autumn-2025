// libs/multiply.c

#include "../header2.3.h"

double multiply_va(int count, ...) {
    if (count < 1) return NAN;

    va_list args;
    va_start(args, count);

    double total = va_arg(args, double); // первое число
    for (int i = 1; i < count; i++) {
        total *= va_arg(args, double);
    }

    va_end(args);
    return total;
}

/*const char* plugin_name = "Умножение N чисел";
const int plugin_min_args = 2;
OperationFunc plugin_func = multiply_va;*/

// 1. ИСПРАВЛЕНИЕ: Это должна быть функция, возвращающая const char*
const char* plugin_name() {
    return "Умножение N чисел";
}

// 2. ИСПРАВЛЕНИЕ: Это должна быть функция, возвращающая int
int plugin_min_args() {
    return 2;
}

// 3. ИСПРАВЛЕНИЕ: Это должна быть функция, возвращающая указатель на функцию
OperationFunc plugin_func() {
    return multiply_va;
}