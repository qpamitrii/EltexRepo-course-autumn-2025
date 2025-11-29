CC = gcc
CFLAGS = -Wall -Wextra
LIBNAME = libphonebook.so   # ← изменили расширение на .so

all: code

# Компилируем основную программу
code: main4.1.c $(LIBNAME)
	$(CC) main4.1.c -L. -lphonebook -o code

# Создаём динамическую библиотеку
$(LIBNAME): source4.1.o
	$(CC) -shared -fPIC -o $(LIBNAME) source4.1.o

# Компилируем исходник в объектный файл (с флагом -fPIC!)
source4.1.o: source4.1.c header4.1.h
	$(CC) -c -fPIC source4.1.c -o source4.1.o

clean:
	rm -f *.o *.so code

.PHONY: all clean