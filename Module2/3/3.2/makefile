# Имя целевого исполняемого файла
TARGET = code

# Компилятор (gcc подходит для Windows/Linux/Mac)
CC = gcc

# Флаги компиляции
CFLAGS = -Wall -Wextra -std=c99 -I.

# Исходные файлы
SRCS = main3.2.c source3.2.c

# Объектные файлы (автоматически генерируются)
OBJS = $(SRCS:.c=.o)

# Цель по умолчанию
all: $(TARGET)

# Правило сборки исполняемого файла
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Правило для каждого .c → .o
%.o: %.c header3.2.h
	$(CC) $(CFLAGS) -c $< -o $@

# Запуск программы
run: $(TARGET)
	./$(TARGET) 192.168.1.0 255.0.0.0 100000

# Очистка
clean:
	rm -f $(TARGET) $(OBJS)

# Пересборка
rebuild: clean all

.PHONY: all run clean rebuild