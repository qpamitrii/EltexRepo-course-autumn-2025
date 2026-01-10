#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <signal.h>

#define MAX_TEXT 256
#define SERVER_TYPE 10
#define KEY_FILE "queue_key" // Файл для генерации ключа ftok

// Структура сообщения
struct msg_buf {
    long mtype;             // Тип сообщения (адресат): 10 - серверу, 20/30... - клиенту
    long sender_id;         // ID отправителя (чтобы знать, от кого пришло)
    char mtext[MAX_TEXT];   // Текст сообщения
};

// Размер данных сообщения (без mtype)
const size_t MSG_SIZE = sizeof(struct msg_buf) - sizeof(long);

#endif