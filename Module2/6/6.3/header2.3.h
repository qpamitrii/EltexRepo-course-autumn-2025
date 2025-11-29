#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
//для 6.3
#include <dlfcn.h>
#include <dirent.h>


#define MAX_OPERATIONS 100
#define LIB_DIR "libs"

//указатель на функцию
typedef double (*OperationFunc)(int count, ...);

//Прототипы функций — можно явно вынести 
/*double sum_va(int count, ...);
double subtract_va(int count, ...);
double multiply_va(int count, ...);
double divide_va(int count, ...);*/

//Определение структуры операции
typedef struct {
    char name[100];
    OperationFunc func;
    int min_args;
} Operation;

// Глобальный массив операций (extern — реализация в .c)
extern Operation operations[MAX_OPERATIONS];
extern int op_count;

void show_menu();
//Operation* select_operation(int index);


void load_libs();