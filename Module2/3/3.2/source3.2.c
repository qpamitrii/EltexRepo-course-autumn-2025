#include "header3.2.h"

// Функция для преобразования строки IP в unsigned int
uint32_t ip_to_uint(const char* ip_str) {
    struct in_addr addr;
    if (inet_pton(AF_INET, ip_str, &addr) != 1) {
        fprintf(stderr, "Ошибка: неверный формат IP-адреса '%s'\n", ip_str);
        exit(1);
    }
    return ntohl(addr.s_addr); // network to host long
}

// Функция для преобразования unsigned int в строку IP
char* uint_to_ip(unsigned int ip) {
    static char buffer[16]; // 16 = 4*3 + 3 точки + \0
    struct in_addr addr;
    addr.s_addr = htonl(ip); // host to network long
    if (inet_ntop(AF_INET, &addr, buffer, sizeof(buffer)) == NULL) {
        strcpy(buffer, "invalid");
    }
    return buffer;
}