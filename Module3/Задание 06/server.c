#include "header.h"

#define MAX_CLIENTS 10

int msgid;
long clients[MAX_CLIENTS]; // Массив ID клиентов
int client_count = 0;

// Очистка очереди при выходе (Ctrl+C)
void cleanup(int sig) {
    msgctl(msgid, IPC_RMID, NULL);
    printf("\nСервер остановлен, очередь удалена.\n");
    exit(0);
}

// Проверка, есть ли клиент в базе
int is_client_known(long id) {
    for (int i = 0; i < client_count; i++) {
        if (clients[i] == id) return 1;
    }
    return 0;
}

// Удаление клиента (при shutdown)
void remove_client(long id) {
    for (int i = 0; i < client_count; i++) {
        if (clients[i] == id) {
            // Сдвигаем массив
            for (int j = i; j < client_count - 1; j++) {
                clients[j] = clients[j+1];
            }
            client_count--;
            printf("Клиент %ld удален из рассылки.\n", id);
            return;
        }
    }
}

int main() {
    key_t key;
    struct msg_buf message;

    // Создаем пустой файл для ключа, если его нет
    FILE *fp = fopen(KEY_FILE, "a");
    fclose(fp);

    // Генерация ключа
    if ((key = ftok(KEY_FILE, 'A')) == -1) {
        perror("ftok");
        exit(1);
    }

    // Создание очереди
    if ((msgid = msgget(key, IPC_CREAT | 0666)) == -1) {
        perror("msgget");
        exit(1);
    }

    signal(SIGINT, cleanup); // Обработка Ctrl+C

    printf("Сервер запущен. Ожидание сообщений (тип %d)...\n", SERVER_TYPE);

    while (1) {
        // Читаем сообщение типа 10 (от любого клиента)
        if (msgrcv(msgid, &message, MSG_SIZE, SERVER_TYPE, 0) == -1) {
            perror("msgrcv");
            break;
        }

        // Если клиента нет в списке и это не shutdown, добавляем его
        if (!is_client_known(message.sender_id) && strcmp(message.mtext, "shutdown\n") != 0) {
            if (client_count < MAX_CLIENTS) {
                clients[client_count++] = message.sender_id;
                printf("Новый клиент зарегистрирован: %ld\n", message.sender_id);
            } else {
                printf("Слишком много клиентов, %ld пропущен.\n", message.sender_id);
            }
        }

        // Обработка shutdown
        if (strcmp(message.mtext, "shutdown\n") == 0 || strcmp(message.mtext, "shutdown") == 0) {
            remove_client(message.sender_id);
            continue; // Не пересылаем это сообщение
        }

        printf("Получено от %ld: %s", message.sender_id, message.mtext);

        // Рассылка всем, кроме отправителя
        for (int i = 0; i < client_count; i++) {
            if (clients[i] != message.sender_id) {
                message.mtype = clients[i]; // Меняем тип на ID получателя
                
                // sender_id оставляем оригинальным, чтобы получатель знал, от кого это
                if (msgsnd(msgid, &message, MSG_SIZE, 0) == -1) {
                    perror("msgsnd forwarding");
                }
            }
        }
    }

    return 0;
}