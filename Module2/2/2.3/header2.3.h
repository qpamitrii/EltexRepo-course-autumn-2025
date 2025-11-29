#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdarg.h>

//указатель на функцию
typedef double (*VariadicOperation)(int count, ...);

//Прототипы функций — можно явно вынести 
double sum_va(int count, ...);
double subtract_va(int count, ...);
double multiply_va(int count, ...);
double divide_va(int count, ...);

//Определение структуры операции
typedef struct {
    const char* name;
    VariadicOperation func;
    int min_args;
} Operation;

// Глобальный массив операций (extern — реализация в .c)
extern Operation operations[];
extern const int OP_COUNT;

void show_menu();
Operation* select_operation(int index);