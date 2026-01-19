/**
Basic netfilter module for Linux
 
Makefile
obj-m += nf.o
all:
    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
*/
 
#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/netfilter_ipv4.h>
#include<linux/skbuff.h>
#include<linux/ip.h>
#include<linux/inet.h>

#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/list.h>

#include <linux/netfilter.h>

 
MODULE_AUTHOR("Leonov Dmitrii");
MODULE_DESCRIPTION("Task 6: Basic netfilter module");
MODULE_LICENSE("GPL");


struct blocked_ip{
    __be32 ip;
    struct list_head list;
};

static LIST_HEAD(blocked_list);
static DEFINE_SPINLOCK(my_lock);


static struct nf_hook_ops nfin;
 
static unsigned int hook_func_in(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct ethhdr *eth;
    struct iphdr *ip_header;
    struct blocked_ip *entry;
 
    eth = (struct ethhdr*)skb_mac_header(skb);
    ip_header = (struct iphdr *)skb_network_header(skb);
    //printk(KERN_INFO "src mac %pM, dst mac %pM\n", eth->h_source, eth->h_dest);
    printk(KERN_INFO "Dest IP addr: %pI4\n", &ip_header->daddr);


    spin_lock_bh(&my_lock); 
    //цикл поиска
    list_for_each_entry(entry, &blocked_list, list) {
        if (ip_header->daddr == entry->ip) {
            printk(KERN_INFO "IP_FILTER: Blocked outgoing to %pI4\n", &ip_header->daddr);
            spin_unlock_bh(&my_lock);
            return NF_DROP; // Уничтожить пакет!
        }
    }

    spin_unlock_bh(&my_lock);


    return NF_ACCEPT;
}



static int my_ip_to_int(const char *str, __be32 *ip) {
    unsigned int a, b, c, d;
    
    if (sscanf(str, "%u.%u.%u.%u", &a, &b, &c, &d) != 4) {
        return -EINVAL;
    }

    if (a > 255 || b > 255 || c > 255 || d > 255) {
        return -EINVAL;
    }

    // 3. Собираем байты в одно 32-битное число.
    // Сдвигаем биты:
    *ip = htonl((a << 24) | (b << 16) | (c << 8) | d);

    return 0;
}

//ProcFS: Чтение
static int my_proc_show(struct seq_file *m, void *v) {
    struct blocked_ip *entry;
    
    // Блокируем доступ, чтобы список не менялся пока читаем
    spin_lock_bh(&my_lock); 
    seq_printf(m, "Blocked IPs:\n");

    //цикл поиска
    list_for_each_entry(entry, &blocked_list, list) {
        seq_printf(m, "%pI4\n", &entry->ip);
    }
    
    spin_unlock_bh(&my_lock);
    return 0;
}

static int my_proc_open(struct inode *inode, struct file *file) {
    return single_open(file, my_proc_show, NULL);
}

//Запись
static ssize_t my_proc_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos) {
    char buf[128];
    char command[10];
    char ip_str[20];
    __be32 ip_val;
    struct blocked_ip *entry;

    if (count > 127) return -EINVAL;
    if (copy_from_user(buf, ubuf, count)) return -EFAULT;
    buf[count] = 0;

    //"add 192.168.0.1"
    if (sscanf(buf, "%s %s", command, ip_str) != 2) {
        printk(KERN_INFO "Bad format\n");
        return count;
    }


    if (my_ip_to_int(ip_str, &ip_val) != 0) {
        printk(KERN_WARNING "Invalid IP format: %s\n", ip_str);
        return -EINVAL;
    }

    //PROCFS: Add/Del
    if (strcmp(command, "add") == 0) {
        entry = kmalloc(sizeof(struct blocked_ip), GFP_KERNEL);
        entry->ip = ip_val;
        
        spin_lock_bh(&my_lock);
        list_add(&entry->list, &blocked_list);
        spin_unlock_bh(&my_lock);
        printk(KERN_INFO "IP_FILTER: Added %pI4\n", &ip_val);
        
    } else if (strcmp(command, "del") == 0) {
        // Тут нужно пройтись по списку, найти IP и сделать list_del + kfree
        // (попробуйте реализовать этот цикл сами по аналогии с чтением)
        struct blocked_ip *tmp;

        spin_lock_bh(&my_lock);

        list_for_each_entry_safe(entry, tmp, &blocked_list, list){
            if (entry->ip == ip_val) {
                list_del(&entry->list);
                kfree(entry);
                printk(KERN_INFO "IP removed: %pI4\n", &ip_val);
                break;
            }
        }

        spin_unlock_bh(&my_lock);
    }

    return count;
}

//Регистрация файла
static const struct proc_ops my_proc_ops = {
    .proc_open = my_proc_open,
    .proc_read = seq_read,
    .proc_write = my_proc_write,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

static int __init init_main(void) {
    // Создаем файл /proc/ip_filter
    proc_create("ip_filter", 0666, NULL, &my_proc_ops);
    
    nfin.hook     = hook_func_in;
    nfin.hooknum  = NF_INET_LOCAL_OUT;
    nfin.pf       = PF_INET;
    nfin.priority = NF_IP_PRI_FIRST;
    nf_register_net_hook(&init_net, &nfin); 


    printk(KERN_INFO "IP_FILTER: Module loaded.\n");
    return 0;
}

static void __exit cleanup_main(void) {
    struct blocked_ip *entry, *tmp;
    remove_proc_entry("ip_filter", NULL);

    // ... тут удаление нетфильтра ...
    nf_unregister_net_hook(&init_net, &nfin); 

    spin_lock_bh(&my_lock);
    list_for_each_entry_safe(entry, tmp, &blocked_list, list) {
        list_del(&entry->list);
        kfree(entry);
    }

    spin_unlock_bh(&my_lock);

    printk(KERN_INFO "IP_FILTER: Module unloaded.\n");
}


module_init(init_main);
module_exit(cleanup_main);