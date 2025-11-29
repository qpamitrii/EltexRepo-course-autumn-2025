#include "header2.3.h"

int main() {
    int choice;
    do {
        show_menu();
        if (scanf("%d", &choice) != 1) {
            printf("Ошибка ввода.\n");
            while (getchar() != '\n');
            continue;
        }

        if (choice == 0) {
            printf("Выход.\n");
            break;
        }

        if (choice < 1 || choice > OP_COUNT) {
            printf("Неверный выбор.\n");
            continue;
        }

        Operation* op = select_operation(choice - 1);
        if (!op) continue;

        int n;
        // Всегда спрашиваем количество чисел, но с учётом min_args
        printf("Введите количество чисел (%d-10): ", op->min_args);
        if (scanf("%d", &n) != 1 || n < op->min_args || n > 10) {
            printf("Ошибка: требуется от %d до 10 чисел.\n", op->min_args);
            while (getchar() != '\n');
            continue;
        }


        double args[10];
        printf("Введите %d число%s:\n", n, (n == 1 ? "" : "а/ов"));
        int input_error = 0;
        for (int i = 0; i < n; i++) {
            printf("  [%d]: ", i + 1);
            if (scanf("%lf", &args[i]) != 1) {
                printf("Ошибка ввода числа. Операция отменена.\n");
                while (getchar() != '\n');
                input_error = 1;
                break;
            }
        }

        if (input_error) {
            continue;
        }

        double result;
        switch (n) {
            case 1:  result = op->func(1, args[0]); break;
            case 2:  result = op->func(2, args[0], args[1]); break;
            case 3:  result = op->func(3, args[0], args[1], args[2]); break;
            case 4:  result = op->func(4, args[0], args[1], args[2], args[3]); break;
            case 5:  result = op->func(5, args[0], args[1], args[2], args[3], args[4]); break;
            case 6:  result = op->func(6, args[0], args[1], args[2], args[3], args[4], args[5]); break;
            case 7:  result = op->func(7, args[0], args[1], args[2], args[3], args[4], args[5], args[6]); break;
            case 8:  result = op->func(8, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]); break;
            case 9:  result = op->func(9, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8]); break;
            case 10: result = op->func(10, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9]); break;
            default:
                printf("Ошибка: поддерживается только до 10 чисел.\n");
                continue;
        }

        if (isnan(result) || isinf(result)) {
            printf("Ошибка: недопустимая операция или деление на ноль.\n");
        } else {
            printf("Результат: %.3f\n", result);
        }

    } while (choice != 0);

    return 0;
}