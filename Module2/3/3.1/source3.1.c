#include "header3.1.h"

// Функция: вывод прав в буквенном виде
void print_permissions(mode_t mode) {
    char perms[11] = "----------";

    // Тип файла (не используется, но можно добавить)
    if (S_ISDIR(mode)) perms[0] = 'd';
    else if (/*S_ISLNK*/S_ISBLK(mode)) perms[0] = 'l';
    else if (S_ISREG(mode)) perms[0] = '-';

    // Владелец
    if (mode & S_IRUSR) perms[1] = 'r';
    if (mode & S_IWUSR) perms[2] = 'w';
    if (mode & S_IXUSR) perms[3] = 'x';
    // Группа
    if (mode & S_IRGRP) perms[4] = 'r';
    if (mode & S_IWGRP) perms[5] = 'w';
    if (mode & S_IXGRP) perms[6] = 'x';
    // Остальные
    if (mode & S_IROTH) perms[7] = 'r';
    if (mode & S_IWOTH) perms[8] = 'w';
    if (mode & S_IXOTH) perms[9] = 'x';

    printf("%s", perms);
}

// Функция: вывод прав в цифровом виде (восемеричном)
void print_octal_permissions(mode_t mode) {
    printf("%03o", (unsigned int)(mode & 0777));
}

// Функция: вывод прав в битовом виде (32 бита, только младшие 9 значимых)
void print_binary_permissions(mode_t mode) {
    unsigned int m = (unsigned int)(mode & 0777); // только последние 9 бит
    for (int i = 8; i >= 0; i--) {
        printf("%d", (m >> i) & 1);
    }
}

// Функция: преобразование буквенного представления в mode_t
mode_t str_to_mode(const char* str) {
    mode_t mode = 0;
    if (strlen(str) != 9) return 0;

    // Владелец
    if (str[0] == 'r') mode |= S_IRUSR;
    if (str[1] == 'w') mode |= S_IWUSR;
    if (str[2] == 'x') mode |= S_IXUSR;
    // Группа
    if (str[3] == 'r') mode |= S_IRGRP;
    if (str[4] == 'w') mode |= S_IWGRP;
    if (str[5] == 'x') mode |= S_IXGRP;
    // Остальные
    if (str[6] == 'r') mode |= S_IROTH;
    if (str[7] == 'w') mode |= S_IWOTH;
    if (str[8] == 'x') mode |= S_IXOTH;

    return mode;
}

// Функция: преобразование восьмеричной строки в mode_t
mode_t oct_to_mode(const char* str) {
    if (!str || strlen(str) == 0) return (mode_t)-1; // ошибка
    char* end;
    errno = 0;
    long val = strtol(str, &end, 8);
    if (errno != 0 || *end != '\0' || val < 0 || val > 0777) {
        return (mode_t)-1; // ошибка
    }
    return (mode_t)val;
}

// Функция: применение команды chmod (без изменения файла!)
mode_t apply_chmod_command(mode_t current, const char* cmd) {
    if (!cmd) return current;

    mode_t new_mode = current;
    int who = 0; // битовая маска: U=1, G=2, O=4
    int op = 0;  // '+', '-', '='
    int perm = 0; // r=4, w=2, x=1 → в восьмеричной маске

    const char* p = cmd;
    while (*p) {
        // 1. Пропускаем пробелы
        while (*p == ' ') p++;

        // 2. Определяем "кто"
        who = 0;
        while (*p == 'u' || *p == 'g' || *p == 'o' || *p == 'a') {
            switch (*p) {
                case 'u': who |= 1; break;
                case 'g': who |= 2; break;
                case 'o': who |= 4; break;
                case 'a': who |= 7; break;
            }
            p++;
        }
        if (who == 0) who = 7; // по умолчанию — all

        // 3. Операция
        if (*p == '+' || *p == '-' || *p == '=') {
            op = *p;
            p++;
        } else {
            // неверная команда, пропускаем
            break;
        }

        // 4. Права (r,w,x)
        perm = 0;
        while (*p == 'r' || *p == 'w' || *p == 'x') {
            switch (*p) {
                case 'r': perm |= 4; break;
                case 'w': perm |= 2; break;
                case 'x': perm |= 1; break;
            }
            p++;
        }

        // Если perm == 0 — возможно, это цифры? пропускаем (для простоты)
        if (perm == 0) continue;

        // 5. Маска для применения: perm в нужных частях
        mode_t mask = 0;
        if (who & 1) mask |= (perm << 6); // user:   rwx------ → << 6
        if (who & 2) mask |= (perm << 3); // group:  ---rwx--- → << 3
        if (who & 4) mask |= (perm << 0); // other:  ------rwx → << 0

        // 6. Применяем
        switch (op) {
            case '+':
                new_mode |= mask;
                break;
            case '-':
                new_mode &= ~mask;
                break;
            case '=':
                // сначала сбрасываем биты у выбранных групп
                if (who & 1) new_mode &= ~0700;
                if (who & 2) new_mode &= ~0070;
                if (who & 4) new_mode &= ~0007;
                // затем устанавливаем новые
                new_mode |= mask;
                break;
        }
    }

    return new_mode;
}

// Функция: чтение прав из файла через stat
int get_file_permissions(const char* filename, mode_t* mode) {
    struct stat sb;
    if (stat(filename, &sb) == -1) {
        perror("stat");
        return 0;
    }
    *mode = sb.st_mode;
    return 1;
}

// Функция: ввод строки с очисткой буфера
char* safe_input(char* buffer, size_t size) {
    if (fgets(buffer, size, stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
    }
    return buffer;
}

// Главное меню
void show_menu() {
    printf("\n=== Утилита расчета прав доступа ===\n");
    printf("1. Ввести права в буквенном/цифровом виде\n");
    printf("2. Получить права файла\n");
    printf("3. Изменить права по команде (как chmod)\n");
    printf("0. Выход\nВыберите действие: ");
}