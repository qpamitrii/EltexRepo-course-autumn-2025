#include <linux/module.h>
#include <linux/configfs.h>
#include <linux/init.h>
#include <linux/tty.h>          // Для tty_struct
#include <linux/kd.h>           // Для KDSETLED
#include <linux/vt.h>
#include <linux/console_struct.h> // Для vc_cons
#include <linux/vt_kern.h>
#include <linux/kobject.h> 
#include <linux/sysfs.h> 
#include <linux/string.h>

MODULE_DESCRIPTION("Eltex Keyboard LED control via SysFS");
MODULE_AUTHOR("Leonov Dmitrii");
MODULE_LICENSE("GPL");

struct tty_driver *my_driver;
struct tty_struct *my_tty = NULL;
static struct kobject *led_kobj;
static int led_value = 0;

// Определение внешних переменных ядра (нужны для доступа к консоли)
// В некоторых версиях ядра это может вызвать ошибку линковки,
// но это стандартный способ для примера kbleds.
extern int fg_console;
/**
 * Функция установки LED.
 * Вызывает ioctl (KDSETLED) на текущей tty.
 */
static void set_leds(int mask)
{
    // Проверяем, есть ли доступ к tty и есть ли у драйвера функция ioctl
    if (!my_tty || !my_tty->driver->ops->ioctl)
        return;

    // Магия задания: вызываем ioctl из пространства ядра
    // KDSETLED - команда установки диодов
    // mask - битовая маска (1, 2, 4...)
    (my_tty->driver->ops->ioctl) (my_tty, KDSETLED, mask);
}

/**
 * Sysfs: Чтение значения (cat /sys/kernel/eltex_leds/led_control)
 */
static ssize_t led_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", led_value);
}

/**
 * Sysfs: Запись значения (echo "7" > /sys/kernel/eltex_leds/led_control)
 */
static ssize_t led_store(struct kobject *kobj, struct kobj_attribute *attr,
                         const char *buf, size_t count)
{
    int ret;

    // Преобразуем строку в число
    ret = kstrtoint(buf, 10, &led_value);
    if (ret < 0)
        return ret;

    // Вызываем функцию мигания
    set_leds(led_value);

    return count;
}

// Создаем атрибут sysfs (имя файла, права, функции)
static struct kobj_attribute led_attribute =
    __ATTR(led_control, 0660, led_show, led_store);

static int __init kbleds_init(void)
{
    int error = 0;

    printk(KERN_INFO "kbleds: loading\n");
    printk(KERN_INFO "kbleds: fg_console is %d\n", fg_console);

    // Получаем доступ к структуре tty текущей консоли
    my_tty = vc_cons[fg_console].d->port.tty;

    if (!my_tty) {
        printk(KERN_ERR "kbleds: cannot find tty\n");
        return -ENODEV;
    }

    // Создаем папку /sys/kernel/eltex_leds
    led_kobj = kobject_create_and_add("eltex_leds", kernel_kobj);
    if (!led_kobj)
        return -ENOMEM;

    // Создаем файл внутри папки
    error = sysfs_create_file(led_kobj, &led_attribute.attr);
    if (error) {
        printk(KERN_ERR "kbleds: failed to create sysfs file\n");
        kobject_put(led_kobj);
    }

    return error;
}

static void __exit kbleds_exit(void)
{
    // Сбрасываем лампочки при выходе (опционально, можно выключить все = 0)
    set_leds(0); 
    
    // Удаляем sysfs
    kobject_put(led_kobj);
    
    printk(KERN_INFO "kbleds: unloading\n");
}

module_init(kbleds_init);
module_exit(kbleds_exit);