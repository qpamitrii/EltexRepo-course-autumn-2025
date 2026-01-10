#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h> // Библиотека для select

#define PORT 8080
#define BUF_SIZE 1024

// Функция для обработки математики (та же логика, что в задаче 13)
void handle_math(int sock, char *buffer) {
    float a, b, res;
    char op;
    char response[BUF_SIZE];

    if (sscanf(buffer, "%f %c %f", &a, &op, &b) == 3) {
        switch (op) {
            case '+': res = a + b; break;
            case '-': res = a - b; break;
            case '*': res = a * b; break;
            case '/': 
                if (b == 0) {
                    strcpy(response, "Error: Division by zero\n");
                    send(sock, response, strlen(response), 0);
                    return;
                }
                res = a / b; 
                break;
            default:
                strcpy(response, "Error: Unknown operator\n");
                send(sock, response, strlen(response), 0);
                return;
        }
        sprintf(response, "Result: %.2f\n", res);
        send(sock, response, strlen(response), 0);
    } else {
        strcpy(response, "Error: Invalid math format\n");
        send(sock, response, strlen(response), 0);
    }
}

// Функция приема файла (ВНИМАНИЕ: блокирующая версия для простоты)
// В идеальном async-сервере это нужно разбивать на состояния.
void handle_file(int sock, char *buffer) {
    char filename[256];
    long filesize;
    
    sscanf(buffer, "file %s %ld", filename, &filesize);
    
    char new_filename[300];
    sprintf(new_filename, "select_server_%s", filename);
    printf("Receiving file '%s' (%ld bytes)...\n", new_filename, filesize);

    send(sock, "READY", 5, 0); // Говорим клиенту: шли данные

    FILE *fp = fopen(new_filename, "wb");
    if (!fp) return;

    long total = 0;
    int n;
    // Этот цикл заблокирует весь сервер, пока файл не скачается.
    // Для учебной задачи это допустимо.
    while (total < filesize) {
        n = recv(sock, buffer, BUF_SIZE, 0);
        if (n <= 0) break;
        fwrite(buffer, 1, n, fp);
        total += n;
    }
    fclose(fp);

    char *msg = "File upload complete.\n";
    send(sock, msg, strlen(msg), 0);
    printf("File saved.\n");
}

int main() {
    int listener;     // Слушающий сокет
    int newfd;        // Сокет для нового подключения
    struct sockaddr_in server_addr, client_addr;
    socklen_t addrlen;
    
    char buf[BUF_SIZE]; // Буфер для данных
    int nbytes;

    int opt = 1;

    // Наборы дескрипторов файлов
    fd_set master;    // Главный список
    fd_set read_fds;  // Временный список для select()
    int fdmax;        // Максимальный номер дескриптора

    // 1. Очистка наборов
    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    // 2. Создание слушающего сокета
    if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    // Разрешаем повторное использование порта
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 3. Bind
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    memset(&(server_addr.sin_zero), '\0', 8);

    if (bind(listener, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(1);
    }

    // 4. Listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(1);
    }

    // Добавляем слушающий сокет в главный набор
    FD_SET(listener, &master);
    // Пока что самый большой дескриптор - это слушающий
    fdmax = listener;

    printf("Multiplexing Server (SELECT) running on port %d...\n", PORT);

    // --- ГЛАВНЫЙ ЦИКЛ ---
    while(1) {
        // Копируем master в read_fds, так как select изменяет переданный набор
        read_fds = master;

        // 5. Вызов select
        // Ждем, пока что-то произойдет на любом из сокетов
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(1);
        }

        // 6. Пробегаем по всем дескрипторам, чтобы узнать, кто "проснулся"
        for(int i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // Если сокет i готов к чтению
                
                if (i == listener) {
                    // --- НОВОЕ ПОДКЛЮЧЕНИЕ ---
                    addrlen = sizeof(client_addr);
                    if ((newfd = accept(listener, (struct sockaddr *)&client_addr, &addrlen)) == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // Добавляем новичка в набор
                        if (newfd > fdmax) {    // Обновляем максимум
                            fdmax = newfd;
                        }
                        printf("New connection from %s on socket %d\n", 
                               inet_ntoa(client_addr.sin_addr), newfd);
                    }
                } else {
                    // --- ДАННЫЕ ОТ КЛИЕНТА ---
                    memset(buf, 0, BUF_SIZE);
                    if ((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0) {
                        // Ошибка или закрытие соединения
                        if (nbytes == 0) {
                            printf("Socket %d hung up\n", i);
                        } else {
                            perror("recv");
                        }
                        close(i);           // Закрываем сокет
                        FD_CLR(i, &master); // Удаляем из набора
                    } else {
                        // Данные получены, обрабатываем
                        buf[strcspn(buf, "\n")] = 0; // Убираем перенос строки

                        if (strcmp(buf, "quit") == 0) {
                            printf("Client on socket %d requested quit.\n", i);
                            close(i);
                            FD_CLR(i, &master);
                        } 
                        else if (strncmp(buf, "file", 4) == 0) {
                            handle_file(i, buf);
                        }
                        else if (strchr(buf, '+') || strchr(buf, '-') || 
                                 strchr(buf, '*') || strchr(buf, '/')) {
                            handle_math(i, buf);
                        } 
                        else {
                             char *msg = "Unknown command\n";
                             send(i, msg, strlen(msg), 0);
                        }
                    }
                }
            }
        }
    }
    
    return 0;
}