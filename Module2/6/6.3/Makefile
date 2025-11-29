CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LDFLAGS = -ldl

# 1. Объявляем список всех .so файлов
LIBS_SO = libs/sum.so libs/subtract.so libs/multiply.so libs/divide.so

all: code

# 2. Цель code теперь зависит от всех скомпилированных библиотек
code: main2.3.c source2.3.c $(LIBS_SO)
	$(CC) $^ -o $@ $(LDFLAGS)

# Шаблоны сборки для плагинов
# Используем автоматические переменные: $@ - имя цели, $< - первая зависимость
%.so: %.c
	mkdir -p libs
	$(CC) -shared -fPIC -I. -o $@ $<

# Если вы хотите использовать .c файлы из папки libs,
# и получать .so файлы в той же папке:
libs/sum.so: libs/sum.c
	mkdir -p libs
	$(CC) -shared -fPIC -I. -o $@ $< -lm

libs/subtract.so: libs/subtract.c
	$(CC) -shared -fPIC -I. -o $@ $< -lm

libs/multiply.so: libs/multiply.c
	$(CC) -shared -fPIC -I. -o $@ $< -lm

libs/divide.so: libs/divide.c
	$(CC) -shared -fPIC -I. -o $@ $< -lm

clean:
	rm -f code libs/*.so

.PHONY: all clean