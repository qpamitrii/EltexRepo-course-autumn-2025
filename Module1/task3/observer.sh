#!/bin/bash

CONFIG_FILE="observer.conf"
LOG_FILE="observer.log"

# Проверяем существование конфигурационного файла
[[ -f "$CONFIG_FILE" ]] || {
    echo "Ошибка: файл конфигурации '$CONFIG_FILE' не найден." >&2
    exit 1
}

# Функция: проверяет, запущен ли скрипт по его полному или относительному пути
is_running_via_proc() {
    local target="$1"
    local abs_target

    # Приводим к абсолютному пути для корректного сравнения
    if [[ "$target" == /* ]]; then
        abs_target="$target"
    else
        abs_target="$PWD/$target"
    fi

    # Обходим все PID-директории в /proc
    for pid_dir in /proc/[0-9]*; do
        [[ -d "$pid_dir" ]] || continue

        cmdline_file="$pid_dir/cmdline"
        [[ -r "$cmdline_file" ]] || continue

        # Читаем cmdline и заменяем \0 на пробелы
        cmdline=$(tr '\0' ' ' < "$cmdline_file" 2>/dev/null)

        # Пропускаем пустые или некорректные записи
        [[ -z "$cmdline" ]] && continue

        # Проверяем, содержится ли целевой путь (абсолютный или относительный) в командной строке
        if [[ "$cmdline" == *"$abs_target"* ]] || [[ "$cmdline" == *"$target"* ]]; then
            return 0  # найден запущенный процесс
        fi
    done

    return 1  # не найден
}

# Основной цикл
while IFS= read -r script_name; do
    [[ -z "$script_name" || "$script_name" =~ ^[[:space:]]*# ]] && continue

    # Проверка — ищем в /proc
    if ! is_running_via_proc "$script_name"; then
        # Запускаем через nohup
        {
            echo "$(date '+%Y-%m-%d %H:%M:%S') Перезапущен скрипт: $script_name"
        } >> "$LOG_FILE"
        nohup "$script_name" > /dev/null 2>&1 &
    fi
done < "$CONFIG_FILE"

exit 0
