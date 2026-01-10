#include "header.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <имя_файла>\n", argv[0]);
        exit(1);
    }

    char *filename = argv[1];
    char pos_filename[256];
    sprintf(pos_filename, "%s.pos", filename);

    srand(time(NULL) ^ getpid());

    // 1. Создаем файлы, чтобы ftok сработал
    FILE *fp = fopen(filename, "a"); fclose(fp);
    fp = fopen(pos_filename, "a"); fclose(fp); // Файл позиции

    // 2. Генерируем ключ
    key_t key = ftok(filename, 'A');
    if (key == -1) { perror("ftok"); exit(1); }

    // 3. Создаем семафоры (2 штуки в наборе)
    // IPC_CREAT | IPC_EXCL позволяет узнать, создали мы их первыми или они уже были
    int semid = semget(key, 2, IPC_CREAT | IPC_EXCL | 0666);
    
    if (semid != -1) {
        // Мы создатели -> инициализируем значения
        printf("Инициализация семафоров...\n");
        init_sem(semid, SEM_MUTEX, 1); // 1 = файл свободен
        init_sem(semid, SEM_COUNT, 0); // 0 = данных пока нет
    } else if (errno == EEXIST) {
        // Семафоры уже есть -> просто подключаемся
        semid = semget(key, 2, 0666);
    } else {
        perror("semget");
        exit(1);
    }

    printf("Производитель запущен. Файл: %s\n", filename);

    while (1) {
        // Генерация строки (3-7 случайных чисел)
        int count = 3 + rand() % 5;
        char buffer[256] = "";
        char temp[16];
        
        for (int i = 0; i < count; i++) {
            sprintf(temp, "%d ", rand() % 100);
            strcat(buffer, temp);
        }
        strcat(buffer, "\n");

        // --- КРИТИЧЕСКАЯ СЕКЦИЯ ---
        p_op(semid, SEM_MUTEX); // Блокируем доступ (Lock)

        fp = fopen(filename, "a");
        if (fp) {
            fputs(buffer, fp);
            fclose(fp);
            printf("Записано: %s", buffer);
        } else {
            perror("fopen");
        }

        v_op(semid, SEM_MUTEX); // Разблокируем доступ (Unlock)
        // ---------------------------

        v_op(semid, SEM_COUNT); // Сигнализируем, что появилась новая строка (+1)

        sleep(1); // Имитация бурной деятельности
    }

    return 0;
}