/*Задание 01 (Процессы).
Написать программу, которая принимает аргументы запуска и обрабатывает их по следующим правилам:
1) если аргумент является целым или вещественным числом, то выводится это значение и значение, умноженное на 2;
2) если аргумент не является числом, то он выводится в исходном виде.Обработку делают родительский и дочерний процессы, разделяя задачи примерно поровну.*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// Проверяет, является ли строка числом (целым или вещественным)
int is_number(const char* str) {
    if (!str || *str == '\0') return 0;

    char* endptr;
    strtod(str, &endptr);  //Преобразует строку в дабл, ук-ль будет установлен на первый символ не принадлежащий числу.  не сохраняем значение — только проверяем формат

    // Вся строка должна быть числом (пробелы в начале/конце — не допускаются!)
    return endptr != str && *endptr == '\0';
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <аргумент1> <аргумент2> ... \n", argv[0]);
        return 1;
    }

    // Общее число аргументов (без имени программы)
    int n = argc - 1;

    // Разделение: первая половина — родитель, вторая — дочерний
    int mid = n / 2; // родитель обрабатывает argv[1] ... argv[mid]
                     // дочерний — argv[mid+1] ... argv[n]

    pid_t pid = fork();

    if (pid < 0) {
        //Ошибка создания процесса
        perror("fork failed");
        return 1;

    } else if (pid == 0) {
        //Дочерний процесс
        pid_t my_pid = getpid();
        printf("[PID: %d] Дочерний процесс обрабатывает:\n", (int)my_pid);

        for (int i = mid; i < n; i++) {  // вторая половина: от mid до n-1
            char* arg = argv[i + 1];      // argv[1] — первый аргумент

            if (is_number(arg)) {
                double val = strtod(arg, NULL);
                printf("  %s → %.2f\n", arg, val * 2.0);
            } else {
                printf("  %s → %s\n", arg, arg);
            }
        }
        exit(0); // обязательно завершаем дочерний!

    } else {
        //Родительский процесс
        printf("[PID: %d] Родительский процесс обрабатывает:\n", (int)getpid());

        for (int i = 0; i < mid; i++) {  // первая половина: 0 → mid-1
            char* arg = argv[i + 1];

            if (is_number(arg)) {
                double val = strtod(arg, NULL);
                printf("  %s → %.2f\n", arg, val * 2.0);
                //printf("  %s → %.2f - pid: %d\n", arg, val * 2.0, (int)getpid());
            } else {
                printf("  %s → %s\n", arg, arg);
                //printf("  %s → %s - pid: %d\n", arg, arg, (int)getpid());
            }
        }

        // Ждём дочерний, чтобы не было зомби и вывод не "наезжал"
        wait(NULL);
        printf("\nРодитель (PID: %d) завершил работу.\n", (int)getpid());
    }

    return 0;
}