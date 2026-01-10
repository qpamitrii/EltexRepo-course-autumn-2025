#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/if_ether.h> // Заголовки Ethernet

#define BUFFER_SIZE 65536

// Порты из Задачи 12, которые мы хотим перехватывать
#define TARGET_PORT_1 8000
#define TARGET_PORT_2 8001

// Функция для печати данных в Hex и ASCII (Дамп)
void print_dump(unsigned char* data, int size) {
    for (int i = 0; i < size; i++) {
        // Если начало строки, печатаем смещение
        if (i % 16 == 0) printf("\n\t%04x: ", i);
        
        // Печатаем HEX
        printf("%02x ", data[i]);
        
        // В конце строки или массива допечатываем ASCII представление
        if (i % 16 == 15 || i == size - 1) {
            // Выравнивание пробелами, если строка неполная
            if (i == size - 1) {
                for (int j = 0; j < 15 - (i % 16); j++) printf("   ");
            }
            printf(" | ");
            // Печать символов
            int start = i - (i % 16);
            for (int j = start; j <= i; j++) {
                if (data[j] >= 32 && data[j] <= 126) 
                    printf("%c", data[j]);
                else 
                    printf(".");
            }
        }
    }
    printf("\n");
}

int main() {
    int raw_sock;
    unsigned char buffer[BUFFER_SIZE];
    
    // 1. Создаем RAW сокет (требует sudo)
    // AF_PACKET - канальный уровень (захват заголовков Ethernet)
    // ETH_P_ALL - ловить вообще все протоколы
    raw_sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    
    if (raw_sock < 0) {
        perror("Socket Error (Вы запустили через sudo?)");
        return 1;
    }

    printf("Снифер запущен. Слушаем порты %d и %d...\n", TARGET_PORT_1, TARGET_PORT_2);

    while(1) {
        // 2. Получение пакета
        int data_size = recvfrom(raw_sock, buffer, BUFFER_SIZE, 0, NULL, NULL);
        if (data_size < 0) {
            perror("Recv error");
            return 1;
        }

        // --- РАЗБОР ЗАГОЛОВКОВ ---

        // A. Ethernet Header (первые 14 байт)
        struct ethhdr *eth = (struct ethhdr *)buffer;
        
        // Проверяем, что внутри IP пакет (0x0800)
        if (ntohs(eth->h_proto) != ETH_P_IP) {
            continue; 
        }

        // B. IP Header (идет сразу после Ethernet)
        // sizeof(struct ethhdr) = 14 байт
        struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));
        
        // Проверяем, что протокол внутри IP - это UDP (17)
        if (ip->protocol != IPPROTO_UDP) {
            continue;
        }

        // Вычисляем длину IP заголовка (обычно 20 байт, но может быть больше)
        // ip->ihl хранит длину в 32-битных словах, поэтому умножаем на 4
        int ip_header_len = ip->ihl * 4;

        // C. UDP Header (идет после IP заголовка)
        struct udphdr *udp = (struct udphdr *)(buffer + sizeof(struct ethhdr) + ip_header_len);

        // Получаем порты (конвертируем из Network Byte Order в Host)
        int src_port = ntohs(udp->source);
        int dest_port = ntohs(udp->dest);

        // --- ФИЛЬТРАЦИЯ ---
        // Нас интересуют только пакеты, связанные с нашими портами
        if ((src_port == TARGET_PORT_1 || src_port == TARGET_PORT_2) || 
            (dest_port == TARGET_PORT_1 || dest_port == TARGET_PORT_2)) {

            printf("\n========================================\n");
            printf("Захвачен UDP пакет! Размер: %d байт\n", data_size);
            
            // Вывод IP адресов
            struct in_addr src_ip, dest_ip;
            src_ip.s_addr = ip->saddr;
            dest_ip.s_addr = ip->daddr;
            printf("IP: %s -> %s\n", inet_ntoa(src_ip), inet_ntoa(dest_ip));
            printf("Port: %d -> %d\n", src_port, dest_port);

            // D. Данные (Payload)
            // Идут после UDP заголовка
            int header_size = sizeof(struct ethhdr) + ip_header_len + sizeof(struct udphdr);
            unsigned char *payload = buffer + header_size;
            int payload_size = data_size - header_size;

            printf("Содержимое (текст): ");
            for(int i=0; i < payload_size; i++) {
                if (payload[i] >= 32 && payload[i] <= 126) printf("%c", payload[i]);
            }
            printf("\n");

            printf("Полный дамп (HEX + ASCII):");
            print_dump(payload, payload_size);
        }
    }

    close(raw_sock);
    return 0;
}