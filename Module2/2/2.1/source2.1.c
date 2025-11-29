#include "header2.1.h"

Contact phonebook[MAX_CONTACTS];

// Инициализация телефонной книги
void init_phonebook() {
    for (int i = 0; i < MAX_CONTACTS; i++) {
        phonebook[i].used = 0;
    }
}

// Вспомогательная функция для безопасного ввода строки
void safe_input(char* prompt, char* buffer, size_t size) {
    printf("%s", prompt);
    if (fgets(buffer, size, stdin) != NULL) {
        
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') { // Убираем символ новой строки, если есть
            buffer[len - 1] = '\0';
        }
    }
}


void show_contact_list_brief() {
    printf("\n--- Список контактов ---\n");
    int found = 0;
    for (int i = 0; i < MAX_CONTACTS; i++) {
        if (phonebook[i].used) {
            found = 1;
            printf("ID: %d — %s %s\n", i, phonebook[i].surname, phonebook[i].name);
        }
    }
    if (!found) {
        printf("Нет сохранённых контактов.\n");
    }
}

void view_contact_details() {
    int id;
    printf("Введите ID контакта для просмотра подробной информации: ");
    if (scanf("%d", &id) != 1 || id < 0 || id >= MAX_CONTACTS || !phonebook[id].used) {
        printf("Неверный ID контакта!\n");
        while (getchar() != '\n'); // очистка буфера
        return;
    }
    while (getchar() != '\n'); // очистка после scanf

    // Вывод полной информации
    printf("\n--- Подробная информация о контакте (ID: %d) ---\n", id);
    printf("Ф.И.О.: %s %s\n", phonebook[id].surname, phonebook[id].name);
    if (strlen(phonebook[id].workplace) > 0)
        printf("Место работы: %s\n", phonebook[id].workplace);
    if (strlen(phonebook[id].position) > 0)
        printf("Должность: %s\n", phonebook[id].position);
    if (strlen(phonebook[id].phone) > 0)
        printf("Телефон(ы): %s\n", phonebook[id].phone);
    if (strlen(phonebook[id].email) > 0)
        printf("Email(ы): %s\n", phonebook[id].email);
    if (strlen(phonebook[id].social.telegram) > 0)
        printf("Telegram: %s\n", phonebook[id].social.telegram);
    if (strlen(phonebook[id].social.vk) > 0)
        printf("VK: %s\n", phonebook[id].social.vk);
    if (strlen(phonebook[id].social.whatsapp) > 0)
        printf("WhatsApp: %s\n", phonebook[id].social.whatsapp);
    if (strlen(phonebook[id].social.instagram) > 0)
        printf("Instagram: %s\n", phonebook[id].social.instagram);
    printf("---------------------------------------------\n");
}
/*void showListContact() {
    printf("\n--- Список контактов ---");
    int found = 0;
    for (int i = 0; i < MAX_CONTACTS; i++) {
        if (phonebook[i].used) {
            found = 1;
            printf("\nID: %d\n", i);
            printf("Ф.И.О.: %s %s\n", phonebook[i].surname, phonebook[i].name);
            if (strlen(phonebook[i].workplace) > 0)
                printf("Место работы: %s\n", phonebook[i].workplace);
            if (strlen(phonebook[i].position) > 0)
                printf("Должность: %s\n", phonebook[i].position);
            if (strlen(phonebook[i].phone) > 0)
                printf("Телефон(ы): %s\n", phonebook[i].phone);
            if (strlen(phonebook[i].email) > 0)
                printf("Email(ы): %s\n", phonebook[i].email);
            if (strlen(phonebook[i].social.telegram) > 0)
                printf("Telegram: %s\n", phonebook[i].social.telegram);
            if (strlen(phonebook[i].social.vk) > 0)
                printf("VK: %s\n", phonebook[i].social.vk);
            if (strlen(phonebook[i].social.whatsapp) > 0)
                printf("WhatsApp: %s\n", phonebook[i].social.whatsapp);
            if (strlen(phonebook[i].social.instagram) > 0)
                printf("Instagram: %s\n", phonebook[i].social.instagram);
            printf("------------------------\n");
        }
    }
    if (!found) {
        printf("Нет сохранённых контактов.\n");
    }
}*/


// Добавление контакта
void UI_addContact() {
    Contact new_contact = {0}; // все поля обнулены

    printf("\n--- Добавление нового контакта ---\n");
    safe_input("Введите фамилию (обязательно): ", new_contact.surname, MAX_STR);
    safe_input("Введите имя (обязательно): ", new_contact.name, MAX_STR);

    if (strlen(new_contact.surname) == 0 || strlen(new_contact.name) == 0) {
        printf("Ошибка: фамилия и имя обязательны!\n");
        return;
    }

    safe_input("Место работы (опционально): ", new_contact.workplace, MAX_STR);
    safe_input("Должность (опционально): ", new_contact.position, MAX_STR);
    safe_input("Телефон(ы) (опционально): ", new_contact.phone, MAX_STR);
    safe_input("Email(ы) (опционально): ", new_contact.email, MAX_STR);
    safe_input("Telegram (опционально): ", new_contact.social.telegram, MAX_STR);
    safe_input("VK (опционально): ", new_contact.social.vk, MAX_STR);
    safe_input("WhatsApp (опционально): ", new_contact.social.whatsapp, MAX_STR);
    safe_input("Instagram (опционально): ", new_contact.social.instagram, MAX_STR);

    new_contact.used = 1;

    // Добавляем контакт через функцию инкапсуляции
    int id = add_contact_to_book(phonebook, MAX_CONTACTS, &new_contact);
    if (id == -1) {
        printf("Ошибка: телефонная книга заполнена! Невозможно добавить контакт.\n");
    } else {
        printf("Контакт успешно добавлен (ID: %d)!\n", id);
    }
}

// Добавление в глоб массив
int add_contact_to_book(Contact* book, int max_size, const Contact* contact) {
    if (!contact || contact->used != 1) return -1; // недопустимый контакт

    for (int i = 0; i < max_size; i++) {
        if (!book[i].used) {
            book[i] = *contact;
            return i; // возвращаем ID
        }
    }
    return -1; // нет места
}


// Редактирование контакта по ID
void edit_contact() {
    int id;
    printf("Введите ID контакта для редактирования: ");
    if (scanf("%d", &id) != 1 || id < 0 || id >= MAX_CONTACTS || !phonebook[id].used) {
        printf("Неверный ID контакта!\n");
        while (getchar() != '\n'); // очистка буфера ввода
        return;
    }
    while (getchar() != '\n'); // очистка буфера после scanf

    printf("\n--- Редактирование контакта (ID: %d) ---\n", id);
    printf("Фамилия (текущая: %s): ", phonebook[id].surname);
    safe_input("", phonebook[id].surname, MAX_STR);
    printf("Фамилия (текущая: %s): ", phonebook[id].name);
    safe_input("", phonebook[id].name, MAX_STR);
    //safe_input("Имя (текущее: %s): ", phonebook[id].name, MAX_STR);

    if (strlen(phonebook[id].surname) == 0 || strlen(phonebook[id].name) == 0) {
        printf("Ошибка: фамилия и имя обязательны!\n");
        return;
    }

    printf("Место работы (текущее: %s): ", phonebook[id].workplace);
    safe_input("", phonebook[id].workplace, MAX_STR);

    printf("Должность (текущая: %s): ", phonebook[id].position);
    safe_input("", phonebook[id].position, MAX_STR);

    printf("Телефон(ы) (текущие: %s): ", phonebook[id].phone);
    safe_input("", phonebook[id].phone, MAX_STR);

    printf("Email(ы) (текущие: %s): ", phonebook[id].email);
    safe_input("", phonebook[id].email, MAX_STR);

    
    printf("Telegram (текущий: %s): ", phonebook[id].social.telegram);
    safe_input("", phonebook[id].social.telegram, MAX_STR);

    printf("VK (текущий: %s): ", phonebook[id].social.vk);
    safe_input("", phonebook[id].social.vk, MAX_STR);
    printf("WhatsApp (текущий: %s): ", phonebook[id].social.whatsapp);
    safe_input("", phonebook[id].social.whatsapp, MAX_STR);
    printf("Instagram (текущий: %s): ", phonebook[id].social.instagram);
    safe_input("", phonebook[id].social.instagram, MAX_STR);

    printf("Контакт успешно обновлён!\n");
}

// Удаление контакта по ID
void delete_contact() {
    int id;
    printf("Введите ID контакта для удаления: ");
    if (scanf("%d", &id) != 1 || id < 0 || id >= MAX_CONTACTS || !phonebook[id].used) {
        printf("Неверный ID контакта!\n");
        while (getchar() != '\n');
        return;
    }
    phonebook[id].used = 0;
    printf("Контакт с ID %d удалён.\n", id);
}


// Главное меню
void menu() {
    printf("\n=== Телефонная книга ===\n");
    printf("1. Добавить контакт\n");
    printf("2. Показать краткий список (ID + ФИО)\n");
    printf("3. Просмотреть контакт по ID\n");   // ← новый пункт
    printf("4. Редактировать контакт\n");
    printf("5. Удалить контакт\n");
    printf("0. Выход\n");
    printf("Выберите действие: ");
}