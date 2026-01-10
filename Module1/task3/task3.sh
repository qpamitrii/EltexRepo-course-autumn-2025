#!/bin/bash

# Проверка имени скрипта
SCRIPT_NAME=$(basename "$0")
if [[ "$SCRIPT_NAME" == "template_task.sh" ]]; then
    echo "Я бригадир, сам не работаю" >&2
fi

# Получаем полный путь к скрипту
FULL_PATH="$0"

# Генерируем случайное число от 30 до 1800
SLEEP_TIME=$((RANDOM % 1771 + 30))  # 1800 - 30 + 1 = 1771

# Формируем имя лог-файла без пути (только имя файла)
LOG_FILE_NAME="report_$(basename "$FULL_PATH")_без_полного_пути.log"

# Путь к рабочему каталогу (текущий каталог)
WORK_DIR="."

# Запись в лог-файл без пути
{
    echo "[PID $$] $(date '+%Y-%m-%d %H:%M:%S') Скрипт запущен"
} > "$WORK_DIR/$LOG_FILE_NAME"

echo "Sleep начался. Будет работать $((SLEEP_TIME/60)) минут ($(($SLEEP_TIME%60)) сек)"
# Ждем случайное время
sleep "$SLEEP_TIME"

# Запись в лог-файл с полным путем
{
    echo "[PID $$] $(date '+%Y-%m-%d %H:%M:%S') Скрипт завершился, работал $SLEEP_TIME минут"
} >> "$WORK_DIR/$LOG_FILE_NAME"

exit 0
