#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h> // Для определения размера файла

void error(const char *msg) {
    perror(msg);
    exit(0);
}

// Функция отправки файла
void send_file(int sock, char *filepath) {
    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        printf("ERROR: File %s not found\n", filepath);
        return;
    }

    // Определяем размер файла
    struct stat st;
    stat(filepath, &st);
    long filesize = st.st_size;

    char buffer[1024];
    // Формируем команду для сервера: file <имя> <размер>
    sprintf(buffer, "file %s %ld", filepath, filesize);
    
    // Отправляем заголовок
    write(sock, buffer, strlen(buffer));

    // Ждем подтверждения от сервера ("READY")
    memset(buffer, 0, sizeof(buffer));
    read(sock, buffer, sizeof(buffer));
    if (strncmp(buffer, "READY", 5) != 0) {
        printf("Server not ready or error.\n");
        fclose(fp);
        return;
    }

    printf("Sending file %s (%ld bytes)...\n", filepath, filesize);

    // Читаем файл и отправляем
    int n;
    while ((n = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        write(sock, buffer, n);
    }
    
    fclose(fp);
    printf("File sent.\n");
}

int main(int argc, char *argv[]) {
    int my_sock, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buff[1024];

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = atoi(argv[2]);
    my_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (my_sock < 0) error("ERROR opening socket");

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(my_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    printf("Connected to server.\n");
    printf("Commands:\n");
    printf(" 1. Math: <num> <op> <num> (e.g., 5 + 10)\n");
    printf(" 2. File: file <filename> (e.g., file data.txt)\n");
    printf(" 3. Quit: quit\n");

    while (1) {
        printf("Client> ");
        memset(buff, 0, sizeof(buff));
        fgets(buff, sizeof(buff) - 1, stdin);

        // Убираем \n
        buff[strcspn(buff, "\n")] = 0;

        if (strlen(buff) == 0) continue;

        // Если команда выхода
        if (strcmp(buff, "quit") == 0) {
            write(my_sock, buff, strlen(buff));
            break;
        }

        // Если команда передачи файла
        if (strncmp(buff, "file", 4) == 0) {
            char filename[256];
            // Извлекаем имя файла из строки "file filename"
            if (sscanf(buff, "file %s", filename) == 1) {
                send_file(my_sock, filename);
                
                // Читаем ответ сервера о завершении
                memset(buff, 0, sizeof(buff));
                n = read(my_sock, buff, sizeof(buff) - 1);
                if (n > 0) printf("Server> %s\n", buff);
            } else {
                printf("Usage: file <filename>\n");
            }
            continue;
        }

        // Обычная отправка (математика)
        write(my_sock, buff, strlen(buff));

        // Чтение ответа
        memset(buff, 0, sizeof(buff));
        n = read(my_sock, buff, sizeof(buff) - 1);
        if (n > 0) {
            printf("Server> %s", buff);
        } else {
            printf("Server disconnected.\n");
            break;
        }
    }

    close(my_sock);
    return 0;
}