//указатели на функции + переменное число параметров
#include "header2.3.h"

// Массив операций (статически инициализирован)
Operation operations[] = {
    {"Сумма N чисел", sum_va, 1},
    {"Вычитание N чисел", subtract_va, 2},
    {"Умножение N чисел", multiply_va, 2},
    {"Деление N чисел", divide_va, 2},
};
const int OP_COUNT = sizeof(operations) / sizeof(operations[0]);  // константа вместо вычисления каждый раз

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

double subtract_va(int count, ...) {
    if (count < 1) return NAN;

    va_list args;
    va_start(args, count);

    double total = va_arg(args, double); // первое число
    for (int i = 1; i < count; i++) {
        total -= va_arg(args, double);
    }

    va_end(args);
    return total;
}

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


Operation* select_operation(int index) {
    if (index < 0 || index >= OP_COUNT) return NULL;
    return &operations[index];
}

void show_menu() {
    printf("\n=== Динамический Калькулятор ===\n");
    for (int i = 0; i < OP_COUNT; i++) {
        printf("%d. %s\n", i + 1, operations[i].name);
    }
    printf("0. Выход\nВыберите действие: ");
}