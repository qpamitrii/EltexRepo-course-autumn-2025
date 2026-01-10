#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <limits.h>

#define MAX_NUMS 20      // Максимальное кол-во чисел в наборе
#define SHM_KEY_FILE "shmfile" // Файл для генерации ключа

// Структура данных в разделяемой памяти
struct shared_data {
    int status;          // 0 - ждем записи, 1 - данные готовы, 2 - результат готов
    int stop_flag;       // Флаг остановки (1 - пора выходить)
    
    int array[MAX_NUMS]; // Массив чисел
    int count;           // Реальное количество чисел в массиве
    
    int min;             // Результат: минимум
    int max;             // Результат: максимум
};

// Глобальные переменные для обработчика сигналов
int shmid = -1;
int processed_count = 0;
int keep_running = 1;

// Обработчик SIGINT (Ctrl+C)
void handle_sigint(int sig) {
    keep_running = 0; // Сообщаем циклу в main, что пора закругляться
}

int main() {
    // 1. Создаем файл для ключа (если нет)
    FILE *fp = fopen(SHM_KEY_FILE, "a");
    fclose(fp);

    // 2. Генерация ключа
    key_t key = ftok(SHM_KEY_FILE, 'R');
    if (key == -1) { perror("ftok"); exit(1); }

    // 3. Создание разделяемой памяти
    // IPC_CREAT | 0666 - создать с правами чтения/записи
    shmid = shmget(key, sizeof(struct shared_data), IPC_CREAT | 0666);
    if (shmid == -1) { perror("shmget"); exit(1); }

    // 4. Подключение (attach) памяти к процессу
    struct shared_data *shm = (struct shared_data *)shmat(shmid, NULL, 0);
    if (shm == (void *)-1) { perror("shmat"); exit(1); }

    // Инициализация
    shm->status = 0;
    shm->stop_flag = 0;

    // Установка обработчика сигнала
    signal(SIGINT, handle_sigint);

    srand(time(NULL));

    printf("Программа запущена. Нажмите Ctrl+C для выхода и отчета.\n");

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        // --- ДОЧЕРНИЙ ПРОЦЕСС (Обработчик) ---
        while (1) {
            // Ждем, пока статус не станет 1 (данные готовы)
            while (shm->status != 1) {
                if (shm->stop_flag) break; // Проверка на выход
                usleep(1000); // Спим 1мс, чтобы не грузить CPU
            }

            if (shm->stop_flag) break;

            // Обработка данных
            int local_min = INT_MAX;
            int local_max = INT_MIN;

            for (int i = 0; i < shm->count; i++) {
                if (shm->array[i] < local_min) local_min = shm->array[i];
                if (shm->array[i] > local_max) local_max = shm->array[i];
            }

            // Запись результата
            shm->min = local_min;
            shm->max = local_max;

            // Меняем статус на 2 (результат готов)
            shm->status = 2;
        }

        // Отключение от памяти перед выходом
        shmdt(shm);
        exit(0);

    } else {
        // --- РОДИТЕЛЬСКИЙ ПРОЦЕСС (Генератор + Вывод) ---
        while (keep_running) {
            // Ждем, пока статус не станет 0 (ребенок прочитал прошлое или начало)
            while (shm->status != 0 && keep_running) {
                usleep(1000);
            }
            
            if (!keep_running) break;

            // Генерация данных
            shm->count = 5 + rand() % (MAX_NUMS - 5); // От 5 до MAX чисел
            // printf("\n[Родитель] Сгенерировал %d чисел: ", shm->count);
            for (int i = 0; i < shm->count; i++) {
                shm->array[i] = rand() % 100;
                // printf("%d ", shm->array[i]);
            }
            // printf("\n");

            // Даем отмашку ребенку (Статус 1)
            shm->status = 1;

            // Ждем ответа от ребенка (Статус 2)
            while (shm->status != 2 && keep_running) {
                usleep(1000);
            }

            if (!keep_running) break;

            // Вывод результата
            printf("Набор #%d: Min = %d, Max = %d\n", 
                   processed_count + 1, shm->min, shm->max);
            
            processed_count++;

            // Сброс статуса на 0 (готовы к новому кругу)
            shm->status = 0;
            
            sleep(1); // Пауза для наглядности
        }

        // --- ЗАВЕРШЕНИЕ РАБОТЫ ---
        printf("\n\nПолучен сигнал SIGINT.\n");
        printf("Итого обработано наборов данных: %d\n", processed_count);

        // Сообщаем ребенку о выходе
        shm->stop_flag = 1;
        shm->status = 1; // "Пинаем" ребенка, если он спит в ожидании статуса 1

        wait(NULL); // Ждем завершения ребенка

        // Удаление разделяемой памяти
        shmdt(shm);
        shmctl(shmid, IPC_RMID, NULL);
        printf("Память очищена. Выход.\n");
    }

    return 0;
}