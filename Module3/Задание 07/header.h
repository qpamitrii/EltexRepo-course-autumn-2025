#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#define QUEUE_1 "/chat_queue_1" // Очередь: Клиент 1 -> Клиент 2
#define QUEUE_2 "/chat_queue_2" // Очередь: Клиент 2 -> Клиент 1

#define MAX_SIZE 256
#define MSG_STOP 2  // Приоритет для остановки
#define MSG_NORMAL 1 // Обычный приоритет

// Функция для инициализации атрибутов очереди
void set_attr(struct mq_attr *attr) {
    attr->mq_flags = 0;
    attr->mq_maxmsg = 10;      // Макс. кол-во сообщений в очереди
    attr->mq_msgsize = MAX_SIZE; // Макс. размер одного сообщения
    attr->mq_curmsgs = 0;
}

#endif