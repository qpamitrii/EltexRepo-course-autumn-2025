#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

// Макросы для удобства работы с битами
#define RBIT 0444
#define WBIT 0222
#define XBIT 0111
#define UMASK 0700
#define GMASK 0070
#define OMASK 0007

// Функция: вывод прав в буквенном виде
void print_permissions(mode_t mode);

// Функция: вывод прав в цифровом виде (восемеричном)
void print_octal_permissions(mode_t mode);

// Функция: вывод прав в битовом виде (32 бита, только младшие 9 значимых)
void print_binary_permissions(mode_t mode);

// Функция: преобразование буквенного представления в mode_t
mode_t str_to_mode(const char* str);

// Функция: преобразование восьмеричной строки в mode_t
mode_t oct_to_mode(const char* str);

// Функция: применение команды chmod (без изменения файла!)
mode_t apply_chmod_command(mode_t current, const char* cmd);

// Функция: чтение прав из файла через stat
int get_file_permissions(const char* filename, mode_t* mode);

// Функция: ввод строки с очисткой буфера
char* safe_input(char* buffer, size_t size);

// Главное меню
void show_menu();