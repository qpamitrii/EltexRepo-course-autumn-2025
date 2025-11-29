# Компилятор и флаги
CC       = gcc
CFLAGS   = -Wall -Wextra -std=c11 -O2
# При необходимости добавьте -g для отладки: CFLAGS += -g

# Имена исходных и объектных файлов
SRCS     = main2.3.c source2.3.c
OBJS     = $(SRCS:.c=.o)
TARGET   = code

# Правило по умолчанию
.PHONY: all
all: $(TARGET)

# Сборка исполняемого файла
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@

# Автоматическая компиляция .c → .o (использует встроенное правило Make,
# но явно укажем, чтобы контролировать CFLAGS)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Чистка
.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)

# Очень чистая очистка (если будут временные/лог-файлы — добавляйте)
.PHONY: clean-all
clean-all: clean