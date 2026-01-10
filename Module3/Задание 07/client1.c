#include "header.h"

int main() {
    mqd_t q1, q2;
    char buffer[MAX_SIZE];
    struct mq_attr attr;
    unsigned int prio;

    set_attr(&attr);

    // 1. Создаем/Открываем очереди
    // q1 открываем на ЗАПИСЬ (мы туда пишем)
    q1 = mq_open(QUEUE_1, O_WRONLY | O_CREAT, 0666, &attr);
    // q2 открываем на ЧТЕНИЕ (оттуда читаем ответ)
    q2 = mq_open(QUEUE_2, O_RDONLY | O_CREAT, 0666, &attr);

    if (q1 == (mqd_t)-1 || q2 == (mqd_t)-1) {
        perror("mq_open");
        exit(1);
    }

    printf("Клиент 1 запущен. Введите 'exit' для выхода.\n");

    while (1) {
        // --- ШАГ 1: ОТПРАВКА ---
        printf("Вы: ");
        if (fgets(buffer, MAX_SIZE, stdin) == NULL) break;
        
        // Убираем \n
        buffer[strcspn(buffer, "\n")] = 0;

        int priority = MSG_NORMAL;
        if (strcmp(buffer, "exit") == 0) {
            priority = MSG_STOP;
        }

        // Отправляем в q1
        if (mq_send(q1, buffer, strlen(buffer) + 1, priority) == -1) {
            perror("mq_send");
            break;
        }

        if (priority == MSG_STOP) {
            printf("Завершение работы...\n");
            break;
        }

        printf("... ожидание ответа ...\n");

        // --- ШАГ 2: ПОЛУЧЕНИЕ ОТВЕТА ---
        // Ждем сообщение из q2
        ssize_t bytes_read = mq_receive(q2, buffer, MAX_SIZE, &prio);
        if (bytes_read == -1) {
            perror("mq_receive");
            break;
        }

        printf("Собеседник: %s\n", buffer);

        // Проверяем приоритет
        if (prio == MSG_STOP) {
            printf("Собеседник завершил чат.\n");
            break;
        }
    }

    // Закрытие и удаление очередей (удаляет только инициатор)
    mq_close(q1);
    mq_close(q2);
    mq_unlink(QUEUE_1);
    mq_unlink(QUEUE_2);

    return 0;
}