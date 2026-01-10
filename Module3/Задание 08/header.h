#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

// Индексы в массиве семафоров
#define SEM_MUTEX 0 
#define SEM_COUNT 1 

// Для совместимости (в некоторых системах это нужно объявлять вручную)
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

// --- ФУНКЦИИ-ОБЕРТКИ ДЛЯ SEMOP ---

// P-операция (Wait/Lock): Ждет и уменьшает
void p_op(int semid, int sem_num) {
    struct sembuf op = {sem_num, -1, 0};
    if (semop(semid, &op, 1) == -1) {
        perror("p_op error");
        exit(1);
    }
}

// V-операция (Signal/Unlock): Увеличивает
void v_op(int semid, int sem_num) {
    struct sembuf op = {sem_num, 1, 0};
    if (semop(semid, &op, 1) == -1) {
        perror("v_op error");
        exit(1);
    }
}

// Инициализация семафора (только если мы его создали)
void init_sem(int semid, int sem_num, int val) {
    union semun arg;
    arg.val = val;
    if (semctl(semid, sem_num, SETVAL, arg) == -1) {
        perror("semctl init");
        exit(1);
    }
}

#endif