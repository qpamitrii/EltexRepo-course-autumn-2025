#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <signal.h>
#include "checksum.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8888
#define BUFFER_SIZE 4096
#define EXIT_CMD "EXIT_CLOSE_CONNECTION"
#define SERVER_TOS_MARKER 100

int sock;
struct sockaddr_in dest_addr;
int my_client_port = 0;

void send_packet(const char *data) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    struct iphdr *iph = (struct iphdr *)buffer;
    struct udphdr *udph = (struct udphdr *)(buffer + sizeof(struct iphdr));
    char *payload = buffer + sizeof(struct iphdr) + sizeof(struct udphdr);

    int data_len = strlen(data);
    strcpy(payload, data);

    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0; // Клиент отправляет с TOS 0
    iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + data_len);
    iph->id = htonl(12345);
    iph->frag_off = 0;
    iph->ttl = 255;
    iph->protocol = IPPROTO_UDP;
    iph->check = 0;
    iph->saddr = inet_addr("127.0.0.1");
    iph->daddr = dest_addr.sin_addr.s_addr;
    iph->check = csum((unsigned short *)iph, sizeof(struct iphdr));

    udph->source = htons(my_client_port);
    udph->dest = htons(SERVER_PORT);
    udph->len = htons(sizeof(struct udphdr) + data_len);
    udph->check = 0;

    struct pseudo_header psh;
    psh.source_address = iph->saddr;
    psh.dest_address = iph->daddr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_UDP;
    psh.udp_length = htons(sizeof(struct udphdr) + data_len);

    int psize = sizeof(struct pseudo_header) + sizeof(struct udphdr) + data_len;
    char *pseudogram = malloc(psize);
    memcpy(pseudogram, (char*)&psh, sizeof(struct pseudo_header));
    memcpy(pseudogram + sizeof(struct pseudo_header), udph, sizeof(struct udphdr) + data_len);
    udph->check = csum((unsigned short*)pseudogram, psize);
    free(pseudogram);

    if (sendto(sock, buffer, ntohs(iph->tot_len), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("Send failed");
    }
}

void handle_sigint(int sig) {
    printf("\nSending exit message and closing...\n");
    send_packet(EXIT_CMD);
    close(sock);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <CLIENT_PORT>\n", argv[0]);
        return 1;
    }
    my_client_port = atoi(argv[1]);
    
    char msg[100];
    char recv_buf[BUFFER_SIZE];

    signal(SIGINT, handle_sigint);

    sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }
    int one = 1;
    setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one));

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(SERVER_PORT);
    dest_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    printf("Raw UDP Client (Port %d). Type message:\n", my_client_port);

    while (1) {
        printf("> ");
        if (fgets(msg, sizeof(msg), stdin) == NULL) break;
        msg[strcspn(msg, "\n")] = 0;

        send_packet(msg);

        // Ожидание ответа
        while(1) {
            struct sockaddr_in src_addr;
            socklen_t addr_len = sizeof(src_addr);
            int len = recvfrom(sock, recv_buf, BUFFER_SIZE, 0, (struct sockaddr*)&src_addr, &addr_len);
            
            if (len > 0) {
                struct iphdr *r_iph = (struct iphdr*)recv_buf;
                int ip_hdr_len = r_iph->ihl * 4;
                
                // Фильтрация
                if (r_iph->protocol == IPPROTO_UDP) {
                    struct udphdr *r_udph = (struct udphdr*)(recv_buf + ip_hdr_len);
                    
                    // 1. Проверяем порты
                    if (ntohs(r_udph->dest) == my_client_port && ntohs(r_udph->source) == SERVER_PORT) {
                        
                        // Проверяем метку сервера
                        // Если метки нет (tos=0), значит это наш собственный пакет (эхо), игнорируем
                        if (r_iph->tos != SERVER_TOS_MARKER) {
                            continue;
                        }

                        char *r_data = recv_buf + ip_hdr_len + sizeof(struct udphdr);
                        int data_len = len - ip_hdr_len - sizeof(struct udphdr);
                        
                        char print_buf[BUFFER_SIZE];
                        if (data_len < BUFFER_SIZE) {
                            memcpy(print_buf, r_data, data_len);
                            print_buf[data_len] = '\0';
                            printf("Server reply: %s\n", print_buf);
                        }
                        break; // Получили верный ответ, выходим
                    }
                }
            }
        }
    }
    close(sock);
    return 0;
}