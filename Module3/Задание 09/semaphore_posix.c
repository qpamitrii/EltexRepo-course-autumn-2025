#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>           // Для констант O_CREAT
#include <sys/stat.h>        // Для прав доступа 0644
#include <semaphore.h>       // POSIX семафоры
#include <time.h>
#include <string.h>
#include <limits.h>
#include <sys/wait.h>

#define FILE_NAME "data.txt"
#define POS_FILE "data.pos"
#define SEM_MUTEX_NAME "/sem_mutex_Example" // Имя мьютекса
#define SEM_COUNT_NAME "/sem_count_Example" // Имя счетчика

// Функция для очистки при завершении (Ctrl+C обработать сложно в fork, но для примера)
void cleanup() {
    sem_unlink(SEM_MUTEX_NAME);
    sem_unlink(SEM_COUNT_NAME);
}

int main() {
    // 0. Предварительная очистка (на случай если прошлый запуск упал)
    cleanup();

    // 1. Создание семафоров
    // O_CREAT - создать, 0644 - права доступа
    // 1 - начальное значение для Mutex (открыт)
    sem_t *mutex = sem_open(SEM_MUTEX_NAME, O_CREAT, 0644, 1);
    // 0 - начальное значение для Count (пусто)
    sem_t *count = sem_open(SEM_COUNT_NAME, O_CREAT, 0644, 0);

    if (mutex == SEM_FAILED || count == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    // Создаем чистые файлы
    FILE *fp = fopen(FILE_NAME, "w"); fclose(fp);
    fp = fopen(POS_FILE, "w"); fclose(fp);

    srand(time(NULL));

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid > 0) {
        // --- РОДИТЕЛЬСКИЙ ПРОЦЕСС (ПРОИЗВОДИТЕЛЬ) ---
        printf("[Родитель PID %d] Запущен. Пишу данные...\n", getpid());

        while (1) {
            // Генерация данных
            int nums_count = 3 + rand() % 5;
            char buffer[256] = "";
            char temp[16];
            for (int i = 0; i < nums_count; i++) {
                sprintf(temp, "%d ", rand() % 100);
                strcat(buffer, temp);
            }
            strcat(buffer, "\n");

            // Вход в критическую секцию
            sem_wait(mutex); 
            
            FILE *f = fopen(FILE_NAME, "a");
            if (f) {
                fputs(buffer, f);
                fclose(f);
                printf("[Родитель]: Записал -> %s", buffer);
            }

            // Выход из критической секции
            sem_post(mutex);

            // Сигнал, что появилась новая строка
            sem_post(count);

            sleep(1); // Пауза 1 сек
        }

    } else {
        // --- ДОЧЕРНИЙ ПРОЦЕСС (ПОТРЕБИТЕЛЬ) ---
        printf("[Ребенок PID %d] Запущен. Жду данные...\n", getpid());
        srand(time(NULL) ^ getpid()); // Чтобы рандом отличался, если понадобится

        long offset = 0;

        while (1) {
            // Ждем появления данных (уменьшаем count, если 0 - ждем)
            sem_wait(count);

            // Блокируем файл для чтения
            sem_wait(mutex);

            // Логика чтения: читаем позицию -> читаем строку -> пишем позицию
            char line[256];
            int read_success = 0;

            FILE *f_data = fopen(FILE_NAME, "r");
            if (f_data) {
                fseek(f_data, offset, SEEK_SET); // Прыгаем на нужную позицию
                if (fgets(line, sizeof(line), f_data) != NULL) {
                    offset = ftell(f_data); // Запоминаем, где остановились
                    read_success = 1;
                }
                fclose(f_data);
            }

            sem_post(mutex); // Разблокируем файл как можно скорее

            if (read_success) {
                // Поиск min/max (уже без блокировки файла)
                int min_val = INT_MAX;
                int max_val = INT_MIN;
                int val;
                
                // Копия строки для strtok
                char *token = strtok(line, " \n");
                while (token != NULL) {
                    val = atoi(token);
                    if (val < min_val) min_val = val;
                    if (val > max_val) max_val = val;
                    token = strtok(NULL, " \n");
                }

                printf("   >>> [Ребенок]: Min=%d, Max=%d\n", min_val, max_val);
            }
        }
    }

    // Закрытие семафоров (до этого кода дойдет только если прервать циклы)
    sem_close(mutex);
    sem_close(count);
    cleanup();

    return 0;
}