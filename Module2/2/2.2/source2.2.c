#include "header2.2.h"

// Функции калькулятора
double add(double a, double b) {
    return a + b;
}

double subtract(double a, double b) {
    return a - b;
}

double multiply(double a, double b) {
    return a * b;
}

double divide(double a, double b) {
    if (b == 0.0) {
        return NAN; // Возвращаем NaN при делении на ноль
    }
    return a / b;
}



// Главное меню
void show_menu() {
    printf("\n=== Калькулятор ===\n");
    printf("1. Сложение (+)\n");
    printf("2. Вычитание (-)\n");
    printf("3. Умножение (*)\n");
    printf("4. Деление (/)\n");
    printf("0. Выход\n");
    printf("Выберите действие: ");
}

// Реализация show_operation
int show_operation(int choice) {
    double a, b, result;

    printf("Введите первое число: ");
    if (scanf("%lf", &a) != 1) {
        printf("Ошибка ввода числа.\n");
        while (getchar() != '\n');
        return -1;  // ошибка ввода
    }

    printf("Введите второе число: ");
    if (scanf("%lf", &b) != 1) {
        printf("Ошибка ввода числа.\n");
        while (getchar() != '\n');
        return -1;
    }

    switch (choice) {
        case 1:
            result = add(a, b);
            printf("%.2f + %.2f = %.2f\n", a, b, result);
            break;
        case 2:
            result = subtract(a, b);
            printf("%.2f - %.2f = %.2f\n", a, b, result);
            break;
        case 3:
            result = multiply(a, b);
            printf("%.2f * %.2f = %.2f\n", a, b, result);
            break;
        case 4:
            result = divide(a, b);
            if (isnan(result) || isinf(result)) {
                printf("Ошибка: деление на ноль!\n");
            } else {
                printf("%.2f / %.2f = %.2f\n", a, b, result);
            }
            break;
        default:
            // Недостижимо при корректной проверке в main, но на всякий случай:
            printf("Неизвестная операция.\n");
            return -1;
    }

    return 0;  // успешно
}