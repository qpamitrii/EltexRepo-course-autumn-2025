#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include "checksum.h"

#define SERVER_PORT 8888
#define BUFFER_SIZE 4096
#define EXIT_CMD "EXIT_CLOSE_CONNECTION"
// Маркер для пакетов сервера (чтобы отличать их от клиентских)
#define SERVER_TOS_MARKER 100

typedef struct ClientNode {
    uint32_t ip;
    uint16_t port;
    int counter;
    struct ClientNode *next;
} ClientNode;

ClientNode *head = NULL;

ClientNode* get_client(uint32_t ip, uint16_t port) {
    ClientNode *current = head;
    while (current != NULL) {
        if (current->ip == ip && current->port == port) {
            return current;
        }
        current = current->next;
    }
    ClientNode *new_node = (ClientNode*)malloc(sizeof(ClientNode));
    new_node->ip = ip;
    new_node->port = port;
    new_node->counter = 0;
    new_node->next = head;
    head = new_node;
    return new_node;
}

void reset_client(uint32_t ip, uint16_t port) {
    ClientNode *current = head;
    while (current != NULL) {
        if (current->ip == ip && current->port == port) {
            printf("Client %s:%d sent exit. Resetting counter.\n", 
                   inet_ntoa(*(struct in_addr*)&ip), ntohs(port));
            current->counter = 0; 
            return;
        }
        current = current->next;
    }
}

int main() {
    int sock;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in saddr;
    
    sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    int one = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("Setsockopt failed");
        return 1;
    }

    printf("Raw UDP Echo Server running on port %d...\n", SERVER_PORT);

    while (1) {
        socklen_t saddr_len = sizeof(saddr);
        int data_size = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&saddr, &saddr_len);
        if (data_size < 0) continue;

        struct iphdr *iph = (struct iphdr *)buffer;
        int ip_header_len = iph->ihl * 4;

        if (iph->protocol != IPPROTO_UDP) continue;

        // !!! ГЛАВНАЯ ЗАЩИТА !!!
        // Если в пакете стоит наша метка (TOS=100), значит это наш собственный ответ.
        // Игнорируем его, чтобы не было бесконечного цикла.
        if (iph->tos == SERVER_TOS_MARKER) {
            continue;
        }

        struct udphdr *udph = (struct udphdr *)(buffer + ip_header_len);
        
        if (ntohs(udph->dest) != SERVER_PORT) continue;

        char *payload = buffer + ip_header_len + sizeof(struct udphdr);
        int payload_len = data_size - ip_header_len - sizeof(struct udphdr);
        
        char msg_content[BUFFER_SIZE];
        if (payload_len < BUFFER_SIZE) {
            memcpy(msg_content, payload, payload_len);
            msg_content[payload_len] = '\0';
        } else {
            continue; 
        }

        if (strncmp(msg_content, EXIT_CMD, strlen(EXIT_CMD)) == 0) {
            reset_client(iph->saddr, udph->source);
            continue;
        }

        ClientNode *client = get_client(iph->saddr, udph->source);
        client->counter++;

        char reply_payload[BUFFER_SIZE];
        snprintf(reply_payload, BUFFER_SIZE, "%s %d", msg_content, client->counter);
        int reply_len = strlen(reply_payload);

        printf("Replying to %s:%d: %s\n", 
               inet_ntoa(*(struct in_addr*)&iph->saddr), ntohs(udph->source), reply_payload);

        char send_buf[BUFFER_SIZE];
        memset(send_buf, 0, BUFFER_SIZE);

        struct iphdr *resp_iph = (struct iphdr *)send_buf;
        struct udphdr *resp_udph = (struct udphdr *)(send_buf + sizeof(struct iphdr));
        char *resp_data = send_buf + sizeof(struct iphdr) + sizeof(struct udphdr);

        memcpy(resp_data, reply_payload, reply_len);

        resp_iph->ihl = 5;
        resp_iph->version = 4;
        
        // !!! СТАВИМ МЕТКУ !!!
        resp_iph->tos = SERVER_TOS_MARKER; 
        
        resp_iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + reply_len);
        resp_iph->id = htonl(54321);
        resp_iph->frag_off = 0;
        resp_iph->ttl = 255;
        resp_iph->protocol = IPPROTO_UDP;
        resp_iph->check = 0;
        resp_iph->saddr = iph->daddr;
        resp_iph->daddr = iph->saddr;
        resp_iph->check = csum((unsigned short *)resp_iph, sizeof(struct iphdr));

        resp_udph->source = udph->dest;
        resp_udph->dest = udph->source;
        resp_udph->len = htons(sizeof(struct udphdr) + reply_len);
        resp_udph->check = 0;

        struct pseudo_header psh;
        psh.source_address = resp_iph->saddr;
        psh.dest_address = resp_iph->daddr;
        psh.placeholder = 0;
        psh.protocol = IPPROTO_UDP;
        psh.udp_length = htons(sizeof(struct udphdr) + reply_len);

        int psize = sizeof(struct pseudo_header) + sizeof(struct udphdr) + reply_len;
        char *pseudogram = malloc(psize);
        memcpy(pseudogram, (char*)&psh, sizeof(struct pseudo_header));
        memcpy(pseudogram + sizeof(struct pseudo_header), resp_udph, sizeof(struct udphdr) + reply_len);
        resp_udph->check = csum((unsigned short*)pseudogram, psize);
        free(pseudogram);

        struct sockaddr_in dest;
        dest.sin_family = AF_INET;
        dest.sin_addr.s_addr = iph->saddr;

        if (sendto(sock, send_buf, ntohs(resp_iph->tot_len), 0, (struct sockaddr *)&dest, sizeof(dest)) < 0) {
            perror("Sendto failed");
        }
    }
    close(sock);
    return 0;
}