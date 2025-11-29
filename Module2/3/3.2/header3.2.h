#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h> // для inet_ntoa, htonl, ntohl

// Функция для преобразования строки IP в unsigned int
unsigned int ip_to_uint(const char* ip_str);

// Функция для преобразования unsigned int в строку IP
char* uint_to_ip(unsigned int ip);