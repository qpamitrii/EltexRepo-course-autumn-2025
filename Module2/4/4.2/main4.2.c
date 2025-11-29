#include "header4.2.h"

int main() {
    srand(time(NULL));

    PriorityQueue* pq = create_priority_queue();
    if (!pq) {
        printf("Failed to create queue\n");
        return 1;
    }

    printf("=== Генерация сообщений ===\n");
    for (int i = 1; i <= 10; i++) {
        unsigned char prio = rand() % 256; // 0..255
        enqueue(pq, i, prio);
        printf("Добавлено сообщение %d с приоритетом %u\n", i, prio);
    }

    print_queue(pq);

    printf("\n=== Извлечение по разным условиям ===\n");

    int msg_id;
    unsigned char prio;

    // 1. Извлечь первый (FIFO)
    if (dequeue(pq, EXTRACT_FIRST, 0, &msg_id, &prio)) {
        printf("Извлечено первое сообщение: ID=%d, P=%u\n", msg_id, prio);
    }

    // 2. Извлечь с приоритетом 100 (если есть)
    if (dequeue(pq, EXTRACT_BY_PRIORITY, 167, &msg_id, &prio)) {
        printf("Извлечено сообщение с приоритетом 167: ID=%d, P=%u\n", msg_id, prio);
    } else {
        printf("Не найдено сообщение с приоритетом 167\n");
    }

    // 3. Извлечь с приоритетом >= 200
    if (dequeue(pq, EXTRACT_BY_MIN_PRIORITY, 200, &msg_id, &prio)) {
        printf("Извлечено сообщение с приоритетом >= 200: ID=%d, P=%u\n", msg_id, prio);
    } else {
        printf("Нет сообщений с приоритетом >= 200\n");
    }

    print_queue(pq);

    destroy_queue(pq);
    return 0;
}