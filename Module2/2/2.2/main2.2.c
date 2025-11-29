#include "header2.2.h"

int main() {
    int choice;

    do {
        show_menu();
        if (scanf("%d", &choice) != 1) {
            printf("Ошибка ввода. Попробуйте снова.\n");
            while (getchar() != '\n'); // очистка буфера
            continue;
        }

        if (choice == 0) {
            printf("Выход из программы.\n");
            break;
        }

        if (choice < 1 || choice > 4) {
            printf("Неверный выбор. Попробуйте снова.\n");
            continue;
        }

        //вызов меню операций
        if (show_operation(choice) != 0) {
            printf("Неверный вызов операций.\n");
        }

    } while (choice != 0);

    return 0;
}