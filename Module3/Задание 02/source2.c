/*Задание 02 (Процессы).
Написать программу, похожую на командный интерпретатор. При запуске выводится приглашение, и пользователь вводит имя и аргументы программы, которую желает запустить. Если программа не найдена, выводится соответствующее сообщение. После завершения запущенной программы, ваш командный интерпретатор снова выводит приглашение. Командный интерпретатор должен запускать как программы, лежащие в том же каталоге, так и системные программы. Напишите 2-3 простых программы (например, программа для вычисления суммы аргументов, «склеивания» строк, поиска наибольшего значения или наибольшей длины строки и т.д.) для тестирования.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_LINE 1024
#define MAX_ARGS 64

// Функция для разбора строки на аргументы
int parse_command(char* line, char** args) {
    int i = 0;
    char* token = strtok(line, " \t\n");
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL; // завершающий NULL
    return i;
}

// Функция поиска исполняемого файла в PATH
char* find_executable(char* cmd) {
    if (access(cmd, X_OK) == 0) { //системный вызов (POSIX), проверяющий существует ли файл по пути и есть ли у текущего пользователя право на исполнение
        return strdup(cmd);
    }

    // Ищем в PATH - переменная окружения
    char* path = getenv("PATH"); //например, PATH = /usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
    if (!path) return NULL;

    char* path_copy = strdup(path);
    char* dir = strtok(path_copy, ":"); // делит строку на подстроки-лексемы.
    
    while (dir) {
        char full_path[MAX_LINE];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, cmd); //формируем полный путь: dir + "/" + cmd         как sprintf только с ограничением длины строки
        if (access(full_path, X_OK) == 0) {
            free(path_copy);
            return strdup(full_path);
        }
        dir = strtok(NULL, ":"); //NULL означает: «Продолжи разбор той же самой строки, с того места, где остановился в прошлый раз
    }

    free(path_copy);
    return NULL;
}

int main() {
    char line[MAX_LINE];
    char* args[MAX_ARGS];

    printf("Это myshell!\n");

    while (1) {
        printf("> "); // приглашение
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            break; // Ctrl+D
        }

        // Удаляем символ новой строки
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        if (strlen(line) == 0) continue; // пустая строка

        // Разбираем команду
        int argc = parse_command(line, args);
        if (argc == 0) continue;

        // Команда "exit" — выход
        if (strcmp(args[0], "exit") == 0) {
            printf("Выход из myshell.\n");
            break;
        }

        // Создаем дочерний процесс
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            continue;
        } else if (pid == 0) {
            // Дочерний процесс: запускается программа
            char* executable = find_executable(args[0]);
            if (!executable) {
                fprintf(stderr, "Ошибка: команда '%s' не найдена.\n", args[0]);
                exit(1);
            }

            execv(executable, args); // запуск программы, заменяет весь дочерний процесс на целевую программу
            perror("execv"); // если execv вернулся — ошибка
            exit(1);
        } else {
            // Родительский процесс: ждём завершения
            wait(NULL);
        }
    }

    return 0;
}