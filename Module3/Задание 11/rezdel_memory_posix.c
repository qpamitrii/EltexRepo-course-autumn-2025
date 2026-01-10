#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>           // Для O_CREAT, O_RDWR
#include <sys/mman.h>        // POSIX shared memory
#include <sys/stat.h>        // Для mode constants
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <limits.h>
#include <string.h>

#define SHM_NAME "/posix_shm_example" // Имя объекта памяти (должно начинаться с /)
#define MAX_NUMS 50                   // Максимальный размер массива

// Структура, которая будет лежать в общей памяти
struct shared_data {
    volatile int status; // 0=Пусто, 1=Данные готовы, 2=Результат готов
    volatile int stop;   // Флаг остановки
    
    int array[MAX_NUMS];
    int count;
    
    int min;
    int max;
};

// Глобальные переменные для корректного выхода
int keep_running = 1;

// Обработчик Ctrl+C
void handle_sigint(int sig) {
    keep_running = 0;
}

int main() {
    // 1. Создаем/Открываем объект разделяемой памяти
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    // 2. Задаем размер памяти (обязательно для POSIX)
    if (ftruncate(shm_fd, sizeof(struct shared_data)) == -1) {
        perror("ftruncate");
        exit(1);
    }

    // 3. Отображаем память в адресное пространство (mmap)
    struct shared_data *shm = mmap(NULL, sizeof(struct shared_data),
                                   PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // Инициализация
    shm->status = 0;
    shm->stop = 0;

    signal(SIGINT, handle_sigint);
    srand(time(NULL));
    int processed_count = 0;

    printf("POSIX Shared Memory запущена. Имя: %s\n", SHM_NAME);
    printf("Нажмите Ctrl+C для завершения.\n");

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        // --- ДОЧЕРНИЙ ПРОЦЕСС ---
        while (1) {
            // Ждем появления данных (статус 1)
            while (shm->status != 1) {
                if (shm->stop) goto child_exit; // Выход по флагу
                usleep(1000);
            }

            if (shm->stop) break;

            // Поиск Min/Max
            int loc_min = INT_MAX;
            int loc_max = INT_MIN;
            
            for (int i = 0; i < shm->count; i++) {
                if (shm->array[i] < loc_min) loc_min = shm->array[i];
                if (shm->array[i] > loc_max) loc_max = shm->array[i];
            }

            // Запись результата
            shm->min = loc_min;
            shm->max = loc_max;

            // Уведомляем родителя (статус 2)
            shm->status = 2;
        }

    child_exit:
        printf("[Дочерний] Завершение работы.\n");
        munmap(shm, sizeof(struct shared_data)); // Отключаемся от памяти
        exit(0);

    } else {
        // --- РОДИТЕЛЬСКИЙ ПРОЦЕСС ---
        while (keep_running) {
            // Ждем готовности к записи (статус 0)
            while (shm->status != 0 && keep_running) {
                usleep(1000);
            }

            if (!keep_running) break;

            // Генерация данных
            shm->count = 5 + rand() % (MAX_NUMS - 5);
            for (int i = 0; i < shm->count; i++) {
                shm->array[i] = rand() % 1000;
            }

            // Отправляем данные ребенку
            shm->status = 1;

            // Ждем результат (статус 2)
            while (shm->status != 2 && keep_running) {
                usleep(1000);
            }

            if (!keep_running) break;

            // Вывод
            printf("Набор #%d (%d чисел): Min=%d, Max=%d\n", 
                   processed_count + 1, shm->count, shm->min, shm->max);
            
            processed_count++;
            
            // Сброс цикла
            shm->status = 0;
            sleep(1);
        }

        // --- ЗАВЕРШЕНИЕ ---
        printf("\nСигнал получен. Всего обработано наборов: %d\n", processed_count);
        
        // Останавливаем ребенка
        shm->stop = 1;
        shm->status = 1; // Будим ребенка, если он завис в ожидании
        
        wait(NULL); // Ждем выхода ребенка

        // Очистка ресурсов
        munmap(shm, sizeof(struct shared_data)); // Отключить отображение
        shm_unlink(SHM_NAME); // Удалить объект памяти из системы
        
        printf("Память удалена. Выход.\n");
    }

    return 0;
}