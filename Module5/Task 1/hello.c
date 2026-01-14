#include <linux/module.h> // Необходим для всех модулей
#include <linux/kernel.h> // Необходим для KERN_INFO
#include <linux/init.h>   // Необходим для макросов __init и __exit

// Функция, которая вызывается при загрузке модуля (insmod)
static int __init hello_init(void) {
    printk(KERN_INFO "Hello world: Модуль успешно загружен!\n");
    return 0; // 0 означает успешную загрузку
}

// Функция, которая вызывается при выгрузке модуля (rmmod)
static void __exit hello_cleanup(void) {
    printk(KERN_INFO "Hello world: Модуль выгружен. Пока!\n");
}

// Регистрация функций инициализации и очистки
module_init(hello_init);
module_exit(hello_cleanup);

// --- ВАШЕ ЗАДАНИЕ ТУТ ---
// Меняйте данные в кавычках на свои

MODULE_LICENSE("My Custom Student License"); // Ваша выдуманная лицензия
// Примечание: При загрузке ядро напишет "module verification failed: signature and/or required key missing - tainting kernel",
// так как лицензия не GPL. Это нормально для учебного задания.

MODULE_AUTHOR("Leonov Dmitrii");             // Ваше Имя и Фамилия
MODULE_DESCRIPTION("My first kernel module for Eltex course"); // Описание
MODULE_VERSION("1.0");