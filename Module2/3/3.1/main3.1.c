#include "header3.1.h"

int main() {
    char input[100];
    mode_t current_mode = 0;
    int choice;

    do {
        show_menu();
        if (scanf("%d", &choice) != 1) {
            printf("Ошибка ввода.\n");
            while (getchar() != '\n');
            continue;
        }

        while (getchar() != '\n'); // очистка буфера

        switch (choice) {
            case 1: {
                printf("Введите права (например, rwxr-xr-- или 754): ");
                safe_input(input, sizeof(input));

                if (strlen(input) == 9 && strspn(input, "rwx-") == 9) {
                    current_mode = str_to_mode(input);
                    if (current_mode == 0) {
                        printf("Некорректный формат.\n");
                        break;
                    }
                } else if (strlen(input) <= 3 && strspn(input, "01234567") == strlen(input)) {
                    current_mode = oct_to_mode(input);
                    if (current_mode == (mode_t)-1) {  // ← проверка ошибки
                        printf("Некорректный формат.\n");
                        break;
                    }
                } else {
                    printf("Неизвестный формат. Используйте 9 символов (rwx-) или 3 цифры (0-7).\n");
                    break;
                }

                printf("Буквенное представление: ");
                print_permissions(current_mode);
                printf("\nЦифровое представление: ");
                print_octal_permissions(current_mode);
                printf("\nБитовое представление: ");
                print_binary_permissions(current_mode);
                printf("\n");
                break;
            }

            case 2: {
                printf("Введите имя файла: ");
                safe_input(input, sizeof(input));

                if (!get_file_permissions(input, &current_mode)) {
                    printf("Не удалось получить права файла.\n");
                    break;
                }

                printf("Имя файла: %s\n", input);
                printf("Буквенное представление: ");
                print_permissions(current_mode);
                printf("\nЦифровое представление: ");
                print_octal_permissions(current_mode);
                printf("\nБитовое представление: ");
                print_binary_permissions(current_mode);
                printf("\n");

                printf("Сравните с командой: ls -l %s\n", input);
                break;
            }

            case 3: {
                if (current_mode == 0) {
                    printf("Сначала укажите права (пункт 1 или 2).\n");
                    break;
                }

                printf("Текущие права:\n");
                printf("  Буквенно: "); print_permissions(current_mode); printf("\n");
                printf("  Цифрово: "); print_octal_permissions(current_mode); printf("\n");
                printf("  Битово: "); print_binary_permissions(current_mode); printf("\n");

                printf("Введите команду chmod (например, +x, u=rwx,g=rx,o=r, a+x): ");
                safe_input(input, sizeof(input));

                mode_t new_mode = apply_chmod_command(current_mode, input);

                printf("Новые права:\n");
                printf("  Буквенно: "); print_permissions(new_mode); printf("\n");
                printf("  Цифрово: "); print_octal_permissions(new_mode); printf("\n");
                printf("  Битово: "); print_binary_permissions(new_mode); printf("\n");

                printf("Права обновлены в программе (но не применены к файлу на диске).\n");
                break;
            }

            case 0:
                printf("Выход.\n");
                break;

            default:
                printf("Неверный выбор.\n");
        }

    } while (choice != 0);

    return 0;
}