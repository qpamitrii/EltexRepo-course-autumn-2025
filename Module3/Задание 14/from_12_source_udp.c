#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>

#define BUF_SIZE 1024
#define IP_ADDR "127.0.0.1" // Работаем на локальной машине

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Использование: %s <ВАШ ПОРТ> <ПОРТ СОБЕСЕДНИКА>\n", argv[0]);
        printf("Пример 1: %s 8000 8001\n", argv[0]);
        printf("Пример 2: %s 8001 8000\n", argv[0]);
        exit(1);
    }

    int my_port = atoi(argv[1]);
    int target_port = atoi(argv[2]);
    int sock;
    struct sockaddr_in my_addr, target_addr;

    // 1. Создание UDP сокета
    // AF_INET - IPv4, SOCK_DGRAM - UDP (без соединения)
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    // 2. Настройка своего адреса (чтобы слушать входящие)
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Слушаем на всех интерфейсах
    my_addr.sin_port = htons(my_port);

    // 3. Привязка сокета (bind) к своему порту
    if (bind(sock, (struct sockaddr*)&my_addr, sizeof(my_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    // 4. Настройка адреса собеседника (куда отправлять)
    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(target_port);
    // Преобразуем IP строку в сетевой формат
    if (inet_pton(AF_INET, IP_ADDR, &target_addr.sin_addr) <= 0) {
        perror("inet_pton");
        exit(1);
    }

    printf(">>> UDP Чат запущен на порту %d. Пишем на порт %d <<<\n", my_port, target_port);
    printf("Введите 'exit' для выхода.\n");

    // 5. Раздвоение процесса для асинхронности
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        // --- ДОЧЕРНИЙ ПРОЦЕСС (ПОЛУЧЕНИЕ СООБЩЕНИЙ) ---
        char buffer[BUF_SIZE];
        struct sockaddr_in sender_addr;
        socklen_t addr_len = sizeof(sender_addr);

        while (1) {
            // recvfrom блокируется, пока не придет пакет
            int n = recvfrom(sock, buffer, BUF_SIZE - 1, 0, 
                             (struct sockaddr*)&sender_addr, &addr_len);
            
            if (n < 0) {
                perror("recvfrom");
                exit(1);
            }

            buffer[n] = '\0'; // Завершаем строку
            
            // Красивый вывод входящего сообщения
            // \r стирает текущую строку ввода ("Вы: "), чтобы текст не наезжал
            printf("\r[Собеседник]: %s", buffer);
            
            // Восстанавливаем приглашение ввода
            printf("Вы: ");
            fflush(stdout); 
        }

    } else {
        // --- РОДИТЕЛЬСКИЙ ПРОЦЕСС (ОТПРАВКА СООБЩЕНИЙ) ---
        char buffer[BUF_SIZE];
        
        while (1) {
            printf("Вы: "); // Приглашение ввода
            if (fgets(buffer, BUF_SIZE, stdin) == NULL) break;

            // Проверка на выход
            if (strncmp(buffer, "exit", 4) == 0) {
                break;
            }

            // Отправка сообщения (sendto)
            // sock - дескриптор, buffer - данные
            // target_addr - кому отправляем
            sendto(sock, buffer, strlen(buffer), 0, 
                   (struct sockaddr*)&target_addr, sizeof(target_addr));
        }

        // Завершение работы
        printf("Завершение чата...\n");
        kill(pid, SIGTERM); // Убиваем слушающий процесс-ребенок
        close(sock);
        wait(NULL); // Ждем завершения ребенка
    }

    return 0;
}