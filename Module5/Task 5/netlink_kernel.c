#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <linux/init.h>

#define NETLINK_USER 31  // ID нашего протокола (должен совпадать в userspace)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Leonov Dmitrii");
MODULE_DESCRIPTION("Simple Netlink Kernel Module");

struct sock *nl_sk = NULL;

/**
 * Функция, вызываемая при приеме сообщения от пользователя
 */
static void hello_nl_recv_msg(struct sk_buff *skb)
{
    struct nlmsghdr *nlh;
    int pid;
    struct sk_buff *skb_out;
    int msg_size;
    char *msg = "Hello from Kernel!";
    int res;

    printk(KERN_INFO "Netlink: Entering: %s\n", __FUNCTION__);

    // 1. Получаем заголовок сообщения
    nlh = (struct nlmsghdr *)skb->data;
    printk(KERN_INFO "Netlink: Received message payload: %s\n", (char *)nlmsg_data(nlh));
    
    // Получаем PID процесса, который отправил сообщение (нужен для ответа)
    pid = nlh->nlmsg_pid; 

    // 2. Готовим ответ
    msg_size = strlen(msg);

    // Создаем новый буфер (sk_buff) для отправки данных
    skb_out = nlmsg_new(msg_size, GFP_KERNEL);
    if (!skb_out) {
        printk(KERN_ERR "Netlink: Failed to allocate new skb\n");
        return;
    }

    // Заполняем заголовок и данные
    // 0 - seq (последовательность), 0 - flags
    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    
    // NETLINK_CB(skb_out).dst_group = 0; // Unicast (только одному процессу)
    strncpy(nlmsg_data(nlh), msg, msg_size);

    // 3. Отправляем сообщение обратно пользователю (unicast)
    res = nlmsg_unicast(nl_sk, skb_out, pid);
    
    if (res < 0)
        printk(KERN_INFO "Netlink: Error while sending back to user\n");
}

// Конфигурация Netlink сокета
static struct netlink_kernel_cfg cfg = {
    .input = hello_nl_recv_msg,
};

static int __init hello_init(void)
{
    printk(KERN_INFO "Netlink: Module loading\n");

    // Создаем Netlink сокет в ядре
    // &init_net - пространство имен сети
    // NETLINK_USER - наш протокол
    // &cfg - конфигурация с функцией обратного вызова
    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    
    if (!nl_sk) {
        printk(KERN_ALERT "Netlink: Error creating socket.\n");
        return -10;
    }

    return 0;
}

static void __exit hello_exit(void)
{
    printk(KERN_INFO "Netlink: Module unloading\n");
    if (nl_sk) {
        netlink_kernel_release(nl_sk);
    }
}

module_init(hello_init);
module_exit(hello_exit);