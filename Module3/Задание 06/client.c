#include "header.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <client_id (20, 30...)>\n", argv[0]);
        exit(1);
    }

    long my_id = atol(argv[1]);
    if (my_id <= 10) {
        fprintf(stderr, "ID должен быть больше 10 (10 занят сервером).\n");
        exit(1);
    }

    key_t key;
    int msgid;
    struct msg_buf message;

    // Генерация того же ключа
    if ((key = ftok(KEY_FILE, 'A')) == -1) {
        perror("ftok");
        exit(1);
    }

    // Подключение к очереди
    if ((msgid = msgget(key, 0666)) == -1) {
        perror("msgget (очередь не найдена, запустите сервер!)");
        exit(1);
    }

    printf("Клиент %ld запущен. Пишите сообщения (или 'shutdown' для выхода).\n", my_id);

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        // --- ДОЧЕРНИЙ ПРОЦЕСС (ПРИЕМ СООБЩЕНИЙ) ---
        while (1) {
            // Ждем сообщения, адресованного МНЕ (mtype == my_id)
            if (msgrcv(msgid, &message, MSG_SIZE, my_id, 0) == -1) {
                perror("msgrcv");
                exit(1);
            }
            printf("\n[Клиент %ld]: %s> ", message.sender_id, message.mtext);
            fflush(stdout);
        }
    } else {
        // --- РОДИТЕЛЬСКИЙ ПРОЦЕСС (ОТПРАВКА СООБЩЕНИЙ) ---
        while (1) {
            printf("> ");
            if (fgets(message.mtext, MAX_TEXT, stdin) == NULL) break;

            message.mtype = SERVER_TYPE; // Шлем серверу (10)
            message.sender_id = my_id;   // Подписываемся своим ID

            // Отправка
            if (msgsnd(msgid, &message, MSG_SIZE, 0) == -1) {
                perror("msgsnd");
                break;
            }

            // Если ввели shutdown - завершаем работу
            if (strncmp(message.mtext, "shutdown", 8) == 0) {
                printf("Отключение...\n");
                kill(pid, SIGKILL); // Убиваем слушающий процесс
                wait(NULL);
                exit(0);
            }
        }
    }

    return 0;
}