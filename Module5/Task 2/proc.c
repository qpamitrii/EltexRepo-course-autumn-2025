#include <linux/module.h>       // Нужен для любого модуля
#include <linux/kernel.h>       // KERN_INFO
#include <linux/init.h>         // Макросы __init и __exit
#include <linux/proc_fs.h>      // Работа с /proc
#include <linux/uaccess.h>      // copy_to_user, copy_from_user
#include <linux/version.h>      // Проверка версии ядра

// --- ИЗБАВЛЕНИЕ ОТ ХАРДКОДА (МАГИЧЕСКИХ ЧИСЕЛ) ---
#define PROCFS_NAME "eltex_proc" // Имя файла в /proc
#define PROCFS_MAX_SIZE 1024     // Максимальный размер буфера

// --- ИЗОЛЯЦИЯ ПЕРЕМЕННЫХ (STATIC) ---
static char proc_buffer[PROCFS_MAX_SIZE]; // Буфер для данных
static unsigned long proc_buffer_len = 0; // Текущая длина данных
static struct proc_dir_entry *our_proc_file; // Указатель на файл в /proc

// --- ОПИСАНИЕ МОДУЛЯ ---
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Eltex module for proc interaction");
MODULE_VERSION("1.0");

/**
 * Функция чтения (вызывается, когда cat /proc/eltex_proc)
 */
static ssize_t procfile_read(struct file *filePointer, char __user *buffer,
                             size_t buffer_length, loff_t *offset)
{
    // Если смещение больше 0, значит мы уже все передали.
    // Это предотвращает бесконечный цикл при использовании cat
    if (*offset > 0 || proc_buffer_len == 0) {
        return 0;
    }

    // Копируем данные из ядра в пространство пользователя
    if (copy_to_user(buffer, proc_buffer, proc_buffer_len)) {
        return -EFAULT; // Ошибка памяти
    }

    *offset = proc_buffer_len; // Обновляем смещение
    
    printk(KERN_INFO "Proc: Read %lu bytes\n", proc_buffer_len);
    return proc_buffer_len; // Возвращаем количество переданных байт
}

/**
 * Функция записи (вызывается, когда echo "text" > /proc/eltex_proc)
 */
static ssize_t procfile_write(struct file *file, const char __user *buff,
                              size_t len, loff_t *off)
{
    // Проверка на переполнение буфера
    if (len > PROCFS_MAX_SIZE) {
        proc_buffer_len = PROCFS_MAX_SIZE;
    } else {
        proc_buffer_len = len;
    }

    // Копируем данные из пространства пользователя в ядро
    if (copy_from_user(proc_buffer, buff, proc_buffer_len)) {
        return -EFAULT;
    }

    // Для безопасности добавляем нуль-терминатор, если это строка,
    // но в общем случае для сырых данных это может быть необязательно.
    // Однако для удобства чтения сделаем буфер безопасным для printk,
    // если данные - текст.
    if (proc_buffer_len < PROCFS_MAX_SIZE)
        proc_buffer[proc_buffer_len] = '\0';
    else
        proc_buffer[PROCFS_MAX_SIZE - 1] = '\0';

    printk(KERN_INFO "Proc: Written %lu bytes: %s\n", proc_buffer_len, proc_buffer);
    return proc_buffer_len;
}

// --- АДАПТАЦИЯ ПОД ВЕРСИЮ ЯДРА ---
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
// Новая структура для ядер 5.6+
static const struct proc_ops proc_file_fops = {
    .proc_read = procfile_read,
    .proc_write = procfile_write,
};
#else
// Старая структура для старых ядер (< 5.6)
static const struct file_operations proc_file_fops = {
    .read = procfile_read,
    .write = procfile_write,
};
#endif

/**
 * Инициализация модуля
 */
static int __init proc_init(void)
{
    // Создаем файл в /proc
    our_proc_file = proc_create(PROCFS_NAME, 0666, NULL, &proc_file_fops);
    
    if (NULL == our_proc_file) {
        proc_remove(our_proc_file);
        printk(KERN_ALERT "Error: Could not initialize /proc/%s\n", PROCFS_NAME);
        return -ENOMEM;
    }

    printk(KERN_INFO "Proc: Module loaded. File /proc/%s created\n", PROCFS_NAME);
    return 0;
}

/**
 * Очистка модуля
 */
static void __exit proc_cleanup(void)
{
    proc_remove(our_proc_file); // Удаляем файл из /proc
    printk(KERN_INFO "Proc: Module unloaded. /proc/%s removed\n", PROCFS_NAME);
}

module_init(proc_init);
module_exit(proc_cleanup);