//Реализовать очередь с приоритетом. Приоритет задается целым 
//числом от 0 до 255.
//Реализовать функции для работы с очередью: 1) добавление элемента и 2) 
//извлечение элемента: находящегося первым в очереди/с указанным 
//приоритетом/с приоритетом не ниже чем заданный. В тестирующей 
//программе имитировать генерацию сообщений с различным приоритетом и 
//выборку данных с различными условиями.

#include "header4.2.h"

PriorityQueue* create_priority_queue() {
    PriorityQueue* pq = malloc(sizeof(PriorityQueue));
    if (!pq) return NULL;
    pq->head = pq->tail = NULL;
    pq->size = 0;
    return pq;
}

//Добавление в конец — FIFO
bool enqueue(PriorityQueue* pq, int message_id, unsigned char priority) {
    Node* new_node = malloc(sizeof(Node));
    if (!new_node) return false;

    new_node->message_id = message_id;
    new_node->priority = priority;
    new_node->next = NULL;
    new_node->prev = pq->tail;

    if (pq->tail)
        pq->tail->next = new_node;
    else
        pq->head = new_node;

    pq->tail = new_node;
    pq->size++;
    return true;
}

//##################################################################
//Извлечение элемента по трем условиям

Node* extract_node(PriorityQueue* pq, ExtractMode mode, unsigned char param) {
    if (!pq || !pq->head) return NULL;

    Node* target = NULL;

    switch (mode) {
        case EXTRACT_FIRST:
            target = pq->head;
            break;

        case EXTRACT_BY_PRIORITY:
            for (target = pq->head; target; target = target->next) {
                if (target->priority == param)
                    break;
            }
            break;

        case EXTRACT_BY_MIN_PRIORITY:
            for (target = pq->head; target; target = target->next) {
                if (target->priority >= param)
                    break;
            }
            break;
    }

    if (!target) return NULL;

    // Удаление узла из списка
    if (target->prev)
        target->prev->next = target->next;
    else
        pq->head = target->next;

    if (target->next)
        target->next->prev = target->prev;
    else
        pq->tail = target->prev;

    pq->size--;
    return target;
}

// Обёртка: извлечь и вернуть данные
bool dequeue(PriorityQueue* pq, ExtractMode mode, unsigned char param, int* out_message_id, unsigned char* out_priority) {
    Node* node = extract_node(pq, mode, param);
    if (!node) return false;

    *out_message_id = node->message_id;
    *out_priority = node->priority;
    free(node);
    return true;
}

void print_queue(PriorityQueue* pq) {
    printf("Queue (%d elements): ", pq->size);
    for (Node* p = pq->head; p; p = p->next)
        printf("[ID=%d,P=%u] ", p->message_id, p->priority);
    printf("\n");
}

void destroy_queue(PriorityQueue* pq) {
    while (pq->head) {
        Node* tmp = pq->head;
        pq->head = pq->head->next;
        free(tmp);
    }
    free(pq);
}