//указатели на функции + переменное число параметров
#include "header2.3.h"

// Массив операций (статически инициализирован)
/*Operation operations[] = {
    {"Сумма N чисел", sum_va, 1},
    {"Вычитание N чисел", subtract_va, 2},
    {"Умножение N чисел", multiply_va, 2},
    {"Деление N чисел", divide_va, 2},
};

const int OP_COUNT = sizeof(operations) / sizeof(operations[0]);  // константа вместо вычисления каждый раз


Operation* select_operation(int index) {
    if (index < 0 || index >= OP_COUNT) return NULL;
    return &operations[index];
}*/

// Глобальные переменные для динамических операций
Operation operations[MAX_OPERATIONS];
int op_count = 0;

void show_menu() {
    printf("\n=== Динамический Калькулятор ===\n");
    if (op_count == 0) {
        printf("Нет доступных операций.\n");
        return;
    }
    for (int i = 0; i < op_count; i++) {
        printf("%d. %s\n", i + 1, operations[i].name);
    }
    printf("0. Выход\nВыберите действие: ");
}


// Загрузка одной библиотеки
int load_library(const char* filename) {
    char fullpath[256];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", LIB_DIR, filename);

    void* handle = dlopen(fullpath, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Ошибка загрузки %s: %s\n", filename, dlerror());
        return 0;
    }

    // Получаем указатели на символы (теперь это УКАЗАТЕЛИ НА ФУНКЦИИ)
    // 1. Указатель на функцию, возвращающую const char*
    const char* (*get_name)() = dlsym(handle, "plugin_name"); 
    
    // 2. Указатель на функцию, возвращающую int
    int (*get_min_args)() = dlsym(handle, "plugin_min_args"); 
    
    // 3. Указатель на функцию, которая возвращает OperationFunc (нашу нужную функцию)
    OperationFunc (*get_plugin_func_ptr)() = dlsym(handle, "plugin_func"); // <-- ИСПРАВЛЕНО
    
    // Проверка, что все символы найдены
    if (!get_name || !get_min_args || !get_plugin_func_ptr) { // <-- ИСПРАВЛЕНО
        fprintf(stderr, "Ошибка: не найдены символы в %s\n", filename);
        dlclose(handle);
        return 0;
    }

    if (op_count >= MAX_OPERATIONS) {
        fprintf(stderr, "Ошибка: слишком много библиотек.\n");
        dlclose(handle);
        return 0;
    }

    // Вызываем функции-обертки, чтобы получить фактические значения
    const char* name_str = get_name(); // ВЫЗОВ ФУНКЦИИ
    int min_args_val = get_min_args(); // ВЫЗОВ ФУНКЦИИ
    OperationFunc actual_func = get_plugin_func_ptr(); // ВЫЗОВ ФУНКЦИИ, КОТОРАЯ ВОЗВРАЩАЕТ УКАЗАТЕЛЬ НА ФУНКЦИЮ

    // Копируем данные в глобальный массив
    strncpy(operations[op_count].name, name_str, sizeof(operations[op_count].name) - 1);
    operations[op_count].name[sizeof(operations[op_count].name) - 1] = '\0'; // Гарантируем терминатор

    operations[op_count].min_args = min_args_val; // ИСПОЛЬЗУЕМ ЗНАЧЕНИЕ min_args_val
    operations[op_count].func = actual_func;      // ИСПОЛЬЗУЕМ ЗНАЧЕНИЕ actual_func
    op_count++;

    //printf("Загружено: %s (мин. аргументов: %d)\n", operations[op_count-1].name, operations[op_count-1].min_args);
    return 1;
}

// Загрузка всех .so из libs/
void load_libs() {
    DIR* dir = opendir(LIB_DIR);
    if (!dir) {
        fprintf(stderr, "Не удалось открыть каталог: %s\n", LIB_DIR);
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // Проверяем: файл, заканчивается на ".so"
        if (entry->d_type == DT_REG && strlen(entry->d_name) > 3 &&
            strcmp(entry->d_name + strlen(entry->d_name) - 3, ".so") == 0) {
            load_library(entry->d_name);
        }
    }

    closedir(dir);
}