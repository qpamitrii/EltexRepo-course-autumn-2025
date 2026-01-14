#ifndef DEFS_H
#define DEFS_H

#include <sys/types.h>

#define MSG_KEY_FILE "mq_key"
#define PROJECT_ID 'T'
#define MAX_DRIVERS 100

// Типы команд
#define CMD_Status 1
#define CMD_Task   2
#define CMD_Exit   3

// Статусы
#define STATUS_Free 0
#define STATUS_Busy 1

// Структура сообщения
typedef struct {
    long mtype;       // Адресат (PID процесса)
    pid_t sender_pid; // Кто отправил
    int command;      // Тип команды
    int task_time;    // Время задачи (для отправки)
    int status;       // Текущий статус (для ответа)
    int time_left;    // Оставшееся время (для ответа)
} msg_t;

#define MSG_SIZE (sizeof(msg_t) - sizeof(long))

#endif