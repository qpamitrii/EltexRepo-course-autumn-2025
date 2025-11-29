// libs/divide.c

#include "../header2.3.h"

double divide_va(int count, ...) {
    if (count < 1) return NAN;

    va_list args;
    va_start(args, count);

    double total = va_arg(args, double); // делимое
    for (int i = 1; i < count; i++) {
        double divisor = va_arg(args, double);
        if (divisor == 0.0) {
            va_end(args);
            return NAN; // деление на ноль
        }
        total /= divisor;
    }

    va_end(args);
    return total;
}

/*char plugin_name[] = "Деление N чисел";
const int plugin_min_args = 2;
OperationFunc plugin_func = divide_va;*/

// 1. ИСПРАВЛЕНИЕ: Это должна быть функция, возвращающая const char*
const char* plugin_name() {
    return "Деление N чисел";
}

// 2. ИСПРАВЛЕНИЕ: Это должна быть функция, возвращающая int
int plugin_min_args() {
    return 2;
}

// 3. ИСПРАВЛЕНИЕ: Это должна быть функция, возвращающая указатель на функцию
OperationFunc plugin_func() {
    return divide_va;
}