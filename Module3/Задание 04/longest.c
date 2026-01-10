//поиск самой длинной строки

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Использование: longest <строка1> <строка2> ...\n");
        return 1;
    }

    char* longest = argv[1];
    int max_len = strlen(longest);

    for (int i = 2; i < argc; i++) {
        int len = strlen(argv[i]);
        if (len > max_len) {
            longest = argv[i];
            max_len = len;
        }
    }

    printf("Самая длинная строка: '%s' (длина: %d)\n", longest, max_len);
    return 0;
}