#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

// Глобальные переменные нужны, чтобы они были видны внутри обработчика
FILE *fp;
int sigint_count = 0; // Счетчик нажатий Ctrl+C

// Функция-обработчик сигналов
void handle_signal(int sig) {
    if (sig == SIGINT) {
        sigint_count++;
        fprintf(fp, "[SIGNAL] Получен SIGINT (%d/3)\n", sigint_count);
        printf("Получен SIGINT (%d/3)\n", sigint_count); // Вывод в консоль для наглядности
        
        if (sigint_count >= 3) {
            fprintf(fp, "[EXIT] Получен третий SIGINT. Завершение работы.\n");
            printf("Получен третий SIGINT. Программа завершается.\n");
            
            fclose(fp); // Обязательно закрываем файл
            exit(0);
        }
    } else if (sig == SIGQUIT) {
        fprintf(fp, "[SIGNAL] Получен SIGQUIT\n");
        printf("Получен SIGQUIT\n"); // Вывод в консоль
    }
    
    // Сбрасываем буфер, чтобы запись сразу попала в файл физически
    fflush(fp);
}

int main() {
    int counter = 1;

    // Открываем файл для записи (w - перезапись, a - дозапись)
    fp = fopen("log.txt", "w");
    if (fp == NULL) {
        perror("Ошибка открытия файла");
        return 1;
    }

    // Регистрация обработчика для SIGINT и SIGQUIT
    if (signal(SIGINT, handle_signal) == SIG_ERR) {
        perror("Ошибка установки обработчика SIGINT");
        return 1;
    }
    if (signal(SIGQUIT, handle_signal) == SIG_ERR) {
        perror("Ошибка установки обработчика SIGQUIT");
        return 1;
    }

    printf("Программа запущена. PID: %d\n", getpid());
    printf("Пишем логи в файл log.txt...\n");

    while (1) {
        // Вывод счетчика в файл
        fprintf(fp, "%d\n", counter++);
        fflush(fp); // Принудительная запись на диск

        sleep(1); // Пауза 1 секунда
    }

    // Эта часть кода недостижима из-за бесконечного цикла,
    // но файл закрывается в обработчике сигнала.
    fclose(fp);
    return 0;
}