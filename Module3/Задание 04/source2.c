#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h> // Необходимо для open, O_RDONLY и т.д.

#define MAX_LINE 1024
#define MAX_ARGS 64
#define MAX_COMMANDS 16 // Максимальное количество команд в конвейере

// Функция поиска исполняемого файла (без изменений)
char* find_executable(char* cmd) {
    if (access(cmd, X_OK) == 0) {
        return strdup(cmd);
    }
    char* path = getenv("PATH");
    if (!path) return NULL;
    char* path_copy = strdup(path);
    char* dir = strtok(path_copy, ":");
    while (dir) {
        char full_path[MAX_LINE];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, cmd);
        if (access(full_path, X_OK) == 0) {
            free(path_copy);
            return strdup(full_path);
        }
        dir = strtok(NULL, ":");
    }
    free(path_copy);
    return NULL;
}

// Функция для разбора одной команды на аргументы (по пробелам)
int parse_args(char* command, char** args) {
    int i = 0;
    // Используем strtok_r, так как strtok может конфликтовать, если вызывается во вложенных циклах
    char *saveptr;
    char* token = strtok_r(command, " \t\n", &saveptr);
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok_r(NULL, " \t\n", &saveptr);
    }
    args[i] = NULL;
    return i;
}

// Функция обработки перенаправления ввода/вывода (<, >)
// Возвращает 0 при успехе, -1 при ошибке открытия файла
int handle_redirection(char** args) {
    for (int i = 0; args[i] != NULL; i++) {
        // Перенаправление ввода: < filename
        if (strcmp(args[i], "<") == 0) {
            if (args[i+1] == NULL) {
                fprintf(stderr, "Ошибка: не указан файл для ввода.\n");
                return -1;
            }
            int fd = open(args[i+1], O_RDONLY);
            if (fd < 0) {
                perror("Ошибка открытия файла ввода");
                return -1;
            }
            // Подменяем стандартный ввод (0) на fd
            dup2(fd, STDIN_FILENO);
            close(fd);
            
            // Удаляем "<" и имя файла из аргументов, чтобы execv их не увидел
            args[i] = NULL; 
            // Мы "обрезали" список аргументов. Остальное (имя файла) игнорируем.
            // Примечание: Это упрощенная логика, предполагающая, что редирект в конце.
        } 
        // Перенаправление вывода: > filename
        else if (strcmp(args[i], ">") == 0) {
            if (args[i+1] == NULL) {
                fprintf(stderr, "Ошибка: не указан файл для вывода.\n");
                return -1;
            }
            // Открываем на запись, создаем если нет, обрезаем если есть. Права 644.
            int fd = open(args[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("Ошибка открытия файла вывода");
                return -1;
            }
            // Подменяем стандартный вывод (1) на fd
            dup2(fd, STDOUT_FILENO);
            close(fd);
            
            args[i] = NULL; // Обрезаем аргументы
        }
    }
    return 0;
}

int main() {
    char line[MAX_LINE];
    char* commands[MAX_COMMANDS]; // Массив строк с командами (разделенными |)
    
    printf("Это myshell v2.0 (с поддержкой pipes и редиректов)!\n");

    while (1) {
        printf("> ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            break;
        }

        // Удаление \n на конце
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';
        if (len == 0) continue;

        // 1. Разбиваем строку по символу '|'
        int num_cmds = 0;
        char *saveptr_pipe; // Для strtok_r
        char *cmd_str = strtok_r(line, "|", &saveptr_pipe);
        
        while (cmd_str != NULL && num_cmds < MAX_COMMANDS) {
            commands[num_cmds++] = cmd_str;
            cmd_str = strtok_r(NULL, "|", &saveptr_pipe);
        }

        // Если команд нет
        if (num_cmds == 0) continue;

        // Проверка на exit (только если это одна команда)
        if (num_cmds == 1 && strncmp(commands[0], "exit", 4) == 0) {
             printf("Выход.\n");
             break;
        }

        int i;
        int pipefd[2];
        int prev_read_fd = -1; // Дескриптор для чтения из предыдущего канала

        // Цикл по всем командам конвейера
        for (i = 0; i < num_cmds; i++) {
            // Если команда не последняя, создаем канал
            if (i < num_cmds - 1) {
                if (pipe(pipefd) < 0) {
                    perror("pipe");
                    break;
                }
            }

            pid_t pid = fork();

            if (pid < 0) {
                perror("fork");
                break;
            } else if (pid == 0) {
                // --- ДОЧЕРНИЙ ПРОЦЕСС ---

                // 1. Настройка ввода (от предыдущей команды)
                if (prev_read_fd != -1) {
                    dup2(prev_read_fd, STDIN_FILENO); // STDIN читает из предыдущего канала
                    close(prev_read_fd);
                }

                // 2. Настройка вывода (к следующей команде)
                if (i < num_cmds - 1) {
                    dup2(pipefd[1], STDOUT_FILENO); // STDOUT пишет в текущий канал
                    close(pipefd[1]);
                    close(pipefd[0]); // Читающий конец текущего канала ребенку не нужен
                }

                // 3. Парсинг аргументов и обработка редиректов (<, >)
                char* args[MAX_ARGS];
                parse_args(commands[i], args);

                if (args[0] == NULL) exit(0); // Пустая команда

                if (handle_redirection(args) < 0) {
                    exit(1);
                }

                // 4. Поиск и запуск
                char* executable = find_executable(args[0]);
                if (!executable) {
                    fprintf(stderr, "Команда '%s' не найдена.\n", args[0]);
                    exit(1);
                }
                
                execv(executable, args);
                perror("execv");
                exit(1);
            } else {
                // --- РОДИТЕЛЬСКИЙ ПРОЦЕСС ---
                
                // Закрываем дескриптор чтения предыдущего канала (он уже передан ребенку)
                if (prev_read_fd != -1) {
                    close(prev_read_fd);
                }

                // Если это не последняя команда, сохраняем читающий конец текущего канала для следующего ребенка
                if (i < num_cmds - 1) {
                    close(pipefd[1]); // Родитель не пишет в канал
                    prev_read_fd = pipefd[0]; // Сохраняем для следующей итерации
                }
            }
        }

        // Ждем завершения всех детей
        for (i = 0; i < num_cmds; i++) {
            wait(NULL);
        }
    }

    return 0;
}