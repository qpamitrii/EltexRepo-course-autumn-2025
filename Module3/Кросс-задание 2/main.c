#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/timerfd.h>
#include <poll.h>
#include <errno.h>
#include <time.h>
#include "defs.h"

// Глобальные переменные
int msgid;
pid_t drivers[MAX_DRIVERS];
int driver_count = 0;

// --- Функция Водителя (Child Process) ---
void driver_loop() {
    pid_t my_pid = getpid();
    int state = STATUS_Free;
    
    // Создаем таймер (CLOCK_MONOTONIC)
    int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (tfd == -1) {
        perror("timerfd_create");
        exit(1);
    }

    struct pollfd pfd[1];
    pfd[0].fd = tfd;
    pfd[0].events = POLLIN;

    msg_t msg;

    while (1) {
        // 1. Проверяем очередь сообщений (неблокирующе)
        // System V MQ не поддерживает poll напрямую, поэтому используем IPC_NOWAIT
        // и короткий таймаут в poll ниже.
        if (msgrcv(msgid, &msg, MSG_SIZE, my_pid, IPC_NOWAIT) != -1) {
            
            msg_t reply;
            reply.mtype = msg.sender_pid; // Отвечаем тому, кто спросил
            reply.sender_pid = my_pid;

            if (msg.command == CMD_Status) {
                reply.status = state;
                if (state == STATUS_Busy) {
                    struct itimerspec curr_val;
                    timerfd_gettime(tfd, &curr_val);
                    reply.time_left = curr_val.it_value.tv_sec;
                    if (curr_val.it_value.tv_nsec > 0) reply.time_left++; // Округление вверх
                } else {
                    reply.time_left = 0;
                }
                msgsnd(msgid, &reply, MSG_SIZE, 0);
            } 
            else if (msg.command == CMD_Task) {
                if (state == STATUS_Busy) {
                    // Уже занят
                    reply.status = STATUS_Busy;
                    struct itimerspec curr_val;
                    timerfd_gettime(tfd, &curr_val);
                    reply.time_left = curr_val.it_value.tv_sec;
                    msgsnd(msgid, &reply, MSG_SIZE, 0);
                } else {
                    // Свободен - берем задачу
                    struct itimerspec new_value;
                    new_value.it_value.tv_sec = msg.task_time;
                    new_value.it_value.tv_nsec = 0;
                    new_value.it_interval.tv_sec = 0; // Одноразовый таймер
                    new_value.it_interval.tv_nsec = 0;

                    if (timerfd_settime(tfd, 0, &new_value, NULL) == -1) {
                        perror("timerfd_settime");
                    }
                    
                    state = STATUS_Busy;
                    reply.status = STATUS_Free; // Сигнализируем, что взяли успешно
                    reply.time_left = msg.task_time;
                    msgsnd(msgid, &reply, MSG_SIZE, 0);
                }
            }
            else if (msg.command == CMD_Exit) {
                close(tfd);
                exit(0);
            }
        }

        // 2. Ждем событий от таймера или небольшой таймаут для проверки очереди
        // Таймаут 100мс позволяет быстро реагировать на сообщения
        int ret = poll(pfd, 1, 100); 

        if (ret > 0) {
            if (pfd[0].revents & POLLIN) {
                // Таймер сработал
                uint64_t expirations;
                read(tfd, &expirations, sizeof(expirations));
                state = STATUS_Free;
                // printf("[Driver %d] Finished task, now Available\n", my_pid);
            }
        }
    }
}


// Проверка, существует ли водитель с таким PID в нашем списке
int is_valid_driver(pid_t pid) {
    for (int i = 0; i < driver_count; i++) {
        if (drivers[i] == pid) {
            return 1; // Нашли
        }
    }
    return 0; // Не нашли
}


// --- Функции CLI (Parent Process) ---

void create_driver() {
    if (driver_count >= MAX_DRIVERS) {
        printf("Error: Max drivers limit reached.\n");
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        // Код потомка
        driver_loop();
        exit(0); // Никогда не должен достигнуться
    } else {
        // Код родителя
        drivers[driver_count++] = pid;
        printf("Driver created with PID: %d\n", pid);
    }
}

void send_task(pid_t target_pid, int duration) {
    // --- ЗАЩИТА ОТ НЕВЕРНОГО PID ---
    if (!is_valid_driver(target_pid)) {
        printf("Error: Driver with PID %d does not exist.\n", target_pid);
        return;
    }
    
    msg_t msg;
    msg.mtype = target_pid;
    msg.sender_pid = getpid();
    msg.command = CMD_Task;
    msg.task_time = duration;

    if (msgsnd(msgid, &msg, MSG_SIZE, 0) == -1) {
        perror("msgsnd");
        return;
    }

    // Ждем ответ
    msg_t reply;
    if (msgrcv(msgid, &reply, MSG_SIZE, getpid(), 0) != -1) {
        if (reply.status == STATUS_Busy) {
            printf("Error: Driver %d is Busy. Time left: %d sec\n", reply.sender_pid, reply.time_left);
        } else {
            printf("Task assigned to Driver %d for %d sec\n", reply.sender_pid, duration);
        }
    }
}

void get_status(pid_t target_pid) {
    // --- ЗАЩИТА ОТ НЕВЕРНОГО PID ---
    if (!is_valid_driver(target_pid)) {
        printf("Error: Driver with PID %d does not exist.\n", target_pid);
        return;
    }
    
    msg_t msg;
    msg.mtype = target_pid;
    msg.sender_pid = getpid();
    msg.command = CMD_Status;

    if (msgsnd(msgid, &msg, MSG_SIZE, 0) == -1) {
        perror("msgsnd");
        return;
    }

    msg_t reply;
    if (msgrcv(msgid, &reply, MSG_SIZE, getpid(), 0) != -1) {
        if (reply.status == STATUS_Busy) {
            printf("Driver %d: Busy <%d sec>\n", reply.sender_pid, reply.time_left);
        } else {
            printf("Driver %d: Available\n", reply.sender_pid);
        }
    }
}

void get_all_drivers() {
    printf("--- Drivers List ---\n");
    for (int i = 0; i < driver_count; i++) {
        get_status(drivers[i]);
    }
    printf("--------------------\n");
}

void cleanup() {
    // Отправляем сигнал завершения всем водителям
    msg_t msg;
    msg.sender_pid = getpid();
    msg.command = CMD_Exit;
    
    for (int i = 0; i < driver_count; i++) {
        msg.mtype = drivers[i];
        msgsnd(msgid, &msg, MSG_SIZE, 0);
        waitpid(drivers[i], NULL, 0); // Ждем завершения
    }
    
    msgctl(msgid, IPC_RMID, NULL); // Удаляем очередь
    remove(MSG_KEY_FILE);
}

int main() {
    // Создаем файл ключа для ftok
    FILE *fp = fopen(MSG_KEY_FILE, "w");
    if (!fp) { perror("fopen key"); return 1; }
    fclose(fp);

    key_t key = ftok(MSG_KEY_FILE, PROJECT_ID);
    if (key == -1) { perror("ftok"); return 1; }

    // Создаем очередь
    msgid = msgget(key, IPC_CREAT | 0666);
    if (msgid == -1) { perror("msgget"); return 1; }

    printf("Taxi CLI started. Commands:\n");
    printf("  create_driver\n");
    printf("  send_task <pid> <time>\n");
    printf("  get_status <pid>\n");
    printf("  get_drivers\n");
    printf("  exit\n");

    char line[100];
    char cmd[50];
    int arg1, arg2;

    while (1) {
        printf("> ");
        if (fgets(line, sizeof(line), stdin) == NULL) break;

        // Парсинг
        int n = sscanf(line, "%s %d %d", cmd, &arg1, &arg2);

        if (strcmp(cmd, "create_driver") == 0) {
            create_driver();
        } else if (strcmp(cmd, "send_task") == 0 && n == 3) {
            send_task((pid_t)arg1, arg2);
        } else if (strcmp(cmd, "get_status") == 0 && n == 2) {
            get_status((pid_t)arg1);
        } else if (strcmp(cmd, "get_drivers") == 0) {
            get_all_drivers();
        } else if (strcmp(cmd, "exit") == 0) {
            break;
        } else {
            printf("Unknown command or wrong arguments.\n");
        }
    }

    cleanup();
    printf("Exiting...\n");
    return 0;
}