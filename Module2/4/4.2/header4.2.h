#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

typedef struct PriorityQueueNode {
    int message_id;           // данные 
    unsigned char priority;   // 0..255
    struct PriorityQueueNode* next;
    struct PriorityQueueNode* prev;
} Node;

typedef struct {
    Node* head;
    Node* tail;
    int size;
} PriorityQueue;

PriorityQueue* create_priority_queue();

//Добавление в конец — FIFO
bool enqueue(PriorityQueue* pq, int message_id, unsigned char priority);

//Извлечение элемента по трем условиям
// Условия извлечения
typedef enum {
    EXTRACT_FIRST,           // первый в очереди (FIFO)
    EXTRACT_BY_PRIORITY,     // с указанным приоритетом
    EXTRACT_BY_MIN_PRIORITY  // с приоритетом >= заданного
} ExtractMode;
Node* extract_node(PriorityQueue* pq, ExtractMode mode, unsigned char param);

// Обёртка: извлечь и вернуть данные
bool dequeue(PriorityQueue* pq, ExtractMode mode, unsigned char param, int* out_message_id, unsigned char* out_priority);

void print_queue(PriorityQueue* pq);
void destroy_queue(PriorityQueue* pq);