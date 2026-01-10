#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <sys/wait.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}

// Обработчик сигналов, чтобы убирать "зомби" процессы
void zombie_handler(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int nclients = 0;

void printusers() {
    if (nclients) printf("%d user on-line\n", nclients);
    else printf("No User on line\n");
}

// Функция приема файла
void receive_file(int sock, char *buffer) {
    char filename[256];
    long filesize;
    
    // Парсим заголовок: file <имя> <размер>
    sscanf(buffer, "file %s %ld", filename, &filesize);
    
    // Добавим префикс, чтобы не перезаписать исходный файл, если запускаем в той же папке
    char new_filename[300];
    sprintf(new_filename, "server_%s", filename);

    printf("Receiving file: %s (%ld bytes) -> saving as %s\n", filename, filesize, new_filename);
    
    // Говорим клиенту: ГОТОВ
    write(sock, "READY", 5);

    FILE *fp = fopen(new_filename, "wb");
    if (!fp) {
        perror("File open error");
        return;
    }

    long total_read = 0;
    int bytes_read;
    // Читаем данные чанками
    while (total_read < filesize) {
        bytes_read = read(sock, buffer, 1024); // Используем тот же буфер
        if (bytes_read <= 0) break;
        fwrite(buffer, 1, bytes_read, fp);
        total_read += bytes_read;
    }
    fclose(fp);
    
    strcpy(buffer, "File uploaded successfully.\n");
    write(sock, buffer, strlen(buffer));
}

// Основная функция работы с клиентом
void dostuff(int sock) {
    int n;
    char buff[1024];
    float a, b, result;
    char op;

    while (1) {
        memset(buff, 0, sizeof(buff));
        n = read(sock, buff, sizeof(buff) - 1);
        if (n <= 0) break; // Клиент отключился

        // Удаляем \n в конце строки
        buff[strcspn(buff, "\n")] = 0;

        // 1. Команда QUIT
        if (strcmp(buff, "quit") == 0) {
            break;
        }

        // 2. Команда FILE
        if (strncmp(buff, "file", 4) == 0) {
            receive_file(sock, buff);
            continue;
        }

        // 3. Математика (парсим строку вида "10 + 5")
        if (sscanf(buff, "%f %c %f", &a, &op, &b) == 3) {
            switch (op) {
                case '+': result = a + b; break;
                case '-': result = a - b; break;
                case '*': result = a * b; break;
                case '/': 
                    if (b != 0) result = a / b; 
                    else {
                        write(sock, "Error: Division by zero\n", 24);
                        continue;
                    }
                    break;
                default:
                    write(sock, "Error: Unknown operator\n", 24);
                    continue;
            }
            sprintf(buff, "Result: %.2f\n", result);
            write(sock, buff, strlen(buff));
        } 
        else {
            char *msg = "Unknown command. Use: <num> <op> <num> OR file <name> OR quit\n";
            write(sock, msg, strlen(msg));
        }
    }
    
    close(sock);
    nclients--;
    printf("-disconnect\n");
    printusers();
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno, pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    // Обработка зомби-процессов
    signal(SIGCHLD, zombie_handler);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    // Опция для повторного использования порта (чтобы не ждать тайм-аута после перезапуска)
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    printf("Server started on port %d\n", portno);

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0) error("ERROR on accept");

        nclients++;
        printf("New connection from %s\n", inet_ntoa(cli_addr.sin_addr));
        printusers();

        pid = fork();
        if (pid < 0) error("ERROR on fork");

        if (pid == 0) {
            // Дочерний процесс
            close(sockfd);
            dostuff(newsockfd);
            exit(0);
        } else {
            // Родительский процесс
            close(newsockfd);
        }
    }
    close(sockfd);
    return 0;
}