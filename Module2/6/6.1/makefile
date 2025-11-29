CC = gcc
CFLAGS = -Wall -Wextra
LIBNAME = libphonebook.a

all: code

# Компилируем основную программ
code: main4.1.c $(LIBNAME)
	$(CC) main4.1.c -L. -lphonebook -o code

# Создаём статическую библиотеку из объектного файла
$(LIBNAME): source4.1.o
	ar rcs $(LIBNAME) source4.1.o

# Компилируем исходник в объектный файл
source4.1.o: source4.1.c header4.1.h
	$(CC) -c source4.1.c -o source4.1.o

clean:
	rm -f *.o *.a code

.PHONY: clean