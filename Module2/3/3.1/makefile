# Имя целевого исполняемого файла
TARGET = code

# Компилятор (gcc подходит для Windows/Linux/Mac)
CC = gcc

# Флаги компиляции
CFLAGS = -Wall -Wextra -std=c99 -I.

# Исходные файлы
SRCS = main3.1.c source3.1.c

# Объектные файлы (автоматически генерируются)
OBJS = $(SRCS:.c=.o)

# Цель по умолчанию
all: $(TARGET)

# Правило сборки исполняемого файла
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Правило для каждого .c → .o
%.o: %.c header3.1.h
	$(CC) $(CFLAGS) -c $< -o $@

# Запуск программы
run: $(TARGET)
	./$(TARGET)

# Очистка
clean:
	rm -f $(TARGET) $(OBJS)

# Пересборка
rebuild: clean all

.PHONY: all run clean rebuild