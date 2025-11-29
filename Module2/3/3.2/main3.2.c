#include "header3.2.h"

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Использование: %s <IP_шлюза> <маска_подсети> <N>\n", argv[0]);
        return 1;
    }

    // Парсим аргументы
    uint32_t gateway = ip_to_uint(argv[1]);
    uint32_t subnet_mask = ip_to_uint(argv[2]);
    int N = atoi(argv[3]);

    if (N <= 0) {
        fprintf(stderr, "Ошибка: N должно быть положительным числом.\n");
        return 1;
    }

    // Вычисляем сетевой адрес шлюза
    uint32_t gateway_network = gateway & subnet_mask;
    uint32_t broadcast = gateway_network | (~subnet_mask);


    // Инициализируем генератор случайных чисел
    srand(time(NULL));

    int own_subnet_count = 0;
    int other_subnet_count = 0;

    printf("=== Имитация передачи пакетов ===\n");
    printf("Шлюз: %s\n", uint_to_ip(gateway));
    printf("Маска подсети: %s\n", uint_to_ip(subnet_mask));
    printf("Широковещательный адрес: %s\n", uint_to_ip(broadcast));
    printf("Количество пакетов: %d\n", N);
    printf("\nГенерируемые IP-адреса назначения:\n");

    for (int i = 0; i < N; i++) {
        uint32_t dest_ip;
        int attempts = 0;
        const int MAX_ATTEMPTS = 1000;

        do {
            //dest_ip = ((uint32_t)rand() << 16) | rand(); // однократная генерация 32-битного числа
            // Генерируем 4 байта по отдельности
            dest_ip = ((rand() & 0xFF) << 24) | //rand() & 0xFF — оставляем только младшие 8 бит 
                ((rand() & 0xFF) << 16) |
                ((rand() & 0xFF) <<  8) |
                (rand() & 0xFF);

            attempts++;
            // Перегенерируем, если совпадает с шлюзом или broadcast
        } while (
            (dest_ip == gateway || dest_ip == broadcast) &&
            attempts < MAX_ATTEMPTS
        );

        // На всякий случай — если не повезло (крайне маловероятно)
        if (attempts >= MAX_ATTEMPTS) {
            // Просто сдвинем адрес, чтобы избежать зацикливания
            dest_ip = (dest_ip + 1) & 0xFFFFFFFF;
            if (dest_ip == gateway || dest_ip == broadcast) {
                dest_ip = (dest_ip + 1) & 0xFFFFFFFF;
            }
        }

        // Определяем принадлежность к подсети
        uint32_t dest_network = dest_ip & subnet_mask;
        int is_own_subnet = (dest_network == gateway_network);

        if(is_own_subnet){
        printf("%d. %s -> %s\n",
               i + 1,
               uint_to_ip(dest_ip),
               is_own_subnet ? "своя подсеть" : "другая сеть");
        }

        if (is_own_subnet) {
            own_subnet_count++;
        } else {
            other_subnet_count++;
        }
    }

    // Вывод статистики
    printf("\n=== Статистика ===\n");
    printf("Пакетов для своей подсети: %d (%.9f%%)\n",
           own_subnet_count,
           (double)own_subnet_count / N * 100.0);
    printf("Пакетов для других сетей: %d (%.9f%%)\n",
           other_subnet_count,
           (double)other_subnet_count / N * 100.0);

    return 0;
}

//первый аргумент - ip адрес Шлюза
//второй аргумент - маска подсети
//Третий аргумент - количество пакетов (N случайных чисел – IP адресов назначения в пакете)

//192.168.1.0 255.0.0.0 100000