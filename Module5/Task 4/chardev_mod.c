#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>           // Для файловых операций
#include <linux/uaccess.h>      // Для copy_to_user, copy_from_user
#include <linux/device.h>       // Для class_create, device_create
#include <linux/cdev.h>
#include <linux/version.h>      // Чтобы проверять версию ядра

#define DEVICE_NAME "eltex_chardev" // Имя устройства в /dev
#define BUFFER_SIZE 1024            // Размер буфера

MODULE_LICENSE("GPL");            // Обязательно GPL для device_create
MODULE_AUTHOR("Leonov Dmitrii");
MODULE_DESCRIPTION("A simple character device driver");
MODULE_VERSION("1.0");

static int major_number;          // Номер устройства (выдается ядром)
static char device_buffer[BUFFER_SIZE]; // Буфер данных
static struct class*  char_class  = NULL; // Класс устройства
static struct device* char_device = NULL; // Структура устройства
static int open_count = 0;        // Сколько раз устройство открыто

// Прототипы функций
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char __user *, size_t, loff_t *);

// Структура операций с файлом
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
};

/**
 * Вызывается при открытии устройства (cat /dev/...)
 */
static int dev_open(struct inode *inodep, struct file *filep) {
    open_count++;
    printk(KERN_INFO "Chardev: Device opened %d times\n", open_count);
    return 0;
}

/**
 * Вызывается при чтении (cat /dev/...)
 */
static ssize_t dev_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset) {
    int bytes_read = 0;

    // Если мы уже прочитали весь буфер, возвращаем 0 (EOF)
    if (*offset >= strlen(device_buffer)) {
        return 0;
    }

    // Вычисляем, сколько байт отправлять
    // (либо сколько просит пользователь, либо остаток строки)
    if (len > strlen(device_buffer) - *offset) {
        len = strlen(device_buffer) - *offset;
    }

    // Копируем данные пользователю
    if (copy_to_user(buffer, device_buffer + *offset, len)) {
        return -EFAULT;
    }

    *offset += len;    // Сдвигаем позицию чтения
    bytes_read = len;

    printk(KERN_INFO "Chardev: Sent %d bytes to user\n", bytes_read);
    return bytes_read;
}

/**
 * Вызывается при записи (echo "text" > /dev/...)
 */
static ssize_t dev_write(struct file *filep, const char __user *buffer, size_t len, loff_t *offset) {
    if (len > BUFFER_SIZE - 1) {
        len = BUFFER_SIZE - 1;
    }

    // Копируем данные от пользователя в ядро
    if (copy_from_user(device_buffer, buffer, len)) {
        return -EFAULT;
    }

    device_buffer[len] = '\0'; // Добавляем конец строки
    printk(KERN_INFO "Chardev: Received %zu bytes: %s\n", len, device_buffer);
    
    return len;
}

/**
 * Вызывается при закрытии устройства
 */
static int dev_release(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "Chardev: Device closed\n");
    return 0;
}

/**
 * Инициализация модуля
 */
static int __init chardev_init(void) {
    printk(KERN_INFO "Chardev: Initializing...\n");

    // 1. Динамически выделяем старший номер (major number)
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "Chardev: Failed to register a major number\n");
        return major_number;
    }
    printk(KERN_INFO "Chardev: Registered correctly with major number %d\n", major_number);

    // 2. Создаем класс устройства (для udev)
    // ВНИМАНИЕ: В ядре 6.4+ изменился API class_create
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
    char_class = class_create(DEVICE_NAME);
#else
    char_class = class_create(THIS_MODULE, DEVICE_NAME);
#endif

    if (IS_ERR(char_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "Chardev: Failed to register device class\n");
        return PTR_ERR(char_class);
    }

    // 3. Создаем файл устройства /dev/eltex_chardev
    char_device = device_create(char_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(char_device)) {
        class_destroy(char_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "Chardev: Failed to create the device\n");
        return PTR_ERR(char_device);
    }

    // Инициализируем буфер приветствием
    strcpy(device_buffer, "Hello from Kernel!");

    printk(KERN_INFO "Chardev: Device created at /dev/%s\n", DEVICE_NAME);
    return 0;
}

/**
 * Выход из модуля
 */
static void __exit chardev_exit(void) {
    device_destroy(char_class, MKDEV(major_number, 0)); // Удаляем /dev/...
    class_unregister(char_class);                       // Удаляем класс
    class_destroy(char_class);
    unregister_chrdev(major_number, DEVICE_NAME);       // Освобождаем номер
    printk(KERN_INFO "Chardev: Goodbye!\n");
}

module_init(chardev_init);
module_exit(chardev_exit);