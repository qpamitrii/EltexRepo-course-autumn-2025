#include "header.h"
#include <limits.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <имя_файла>\n", argv[0]);
        exit(1);
    }

    char *filename = argv[1];
    char pos_filename[256];
    sprintf(pos_filename, "%s.pos", filename);

    // 1. Получаем ключ (файл должен уже существовать, его создает producer)
    key_t key = ftok(filename, 'A');
    if (key == -1) { 
        fprintf(stderr, "Ошибка ftok. Возможно, производитель еще не запущен?\n");
        exit(1); 
    }

    // 2. Подключаемся к семафорам
    int semid = semget(key, 2, 0666);
    if (semid == -1) {
        perror("semget (запустите сначала producer)");
        exit(1);
    }

    printf("Потребитель запущен (PID %d). Жду данных...\n", getpid());

    while (1) {
        // Ждем, пока появится строка (Count > 0)
        p_op(semid, SEM_COUNT); 

        // --- КРИТИЧЕСКАЯ СЕКЦИЯ ---
        p_op(semid, SEM_MUTEX); // Блокируем доступ к файлам

        // 1. Читаем текущую позицию из .pos файла
        long offset = 0;
        FILE *fp_pos = fopen(pos_filename, "r");
        if (fp_pos) {
            fscanf(fp_pos, "%ld", &offset);
            fclose(fp_pos);
        }

        // 2. Читаем строку данных с этой позиции
        char buffer[256];
        FILE *fp_data = fopen(filename, "r");
        if (!fp_data) {
            perror("Ошибка открытия файла данных");
            v_op(semid, SEM_MUTEX); // Не забываем разлочить при ошибке
            continue;
        }

        fseek(fp_data, offset, SEEK_SET);
        if (fgets(buffer, sizeof(buffer), fp_data) == NULL) {
            // Если вдруг пусто (хотя семафор сказал ок), просто выходим
            fclose(fp_data);
            v_op(semid, SEM_MUTEX);
            continue;
        }

        // 3. Обновляем позицию для следующего потребителя
        long new_offset = ftell(fp_data);
        fclose(fp_data);

        fp_pos = fopen(pos_filename, "w");
        if (fp_pos) {
            fprintf(fp_pos, "%ld", new_offset);
            fclose(fp_pos);
        }

        v_op(semid, SEM_MUTEX); // Разблокируем доступ
        // ---------------------------

        // Анализ строки (поиск мин/макс)
        int min_val = INT_MAX;
        int max_val = INT_MIN;
        int val;
        int found = 0;
        
        char *token = strtok(buffer, " \n");
        while (token != NULL) {
            val = atoi(token);
            if (val < min_val) min_val = val;
            if (val > max_val) max_val = val;
            found = 1;
            token = strtok(NULL, " \n");
        }

        if (found) {
            printf("[Потребитель %d]: Мин=%d, Макс=%d (из строки: %ld байт)\n", 
                   getpid(), min_val, max_val, offset);
        }
    }

    return 0;
}