obj-m += netlink_kernel.o

all:
	# Сборка модуля ядра
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	# Сборка программы пользователя
	gcc user_app.c -o user_app

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f user_app