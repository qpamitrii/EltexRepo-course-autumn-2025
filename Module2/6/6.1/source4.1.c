//Доработать решение задачи 2.1 (список контактов) так, чтобы 
//для хранения данных использовался двухсвязный упорядоченный список

#include "header4.1.h"

//Contact phonebook[MAX_CONTACTS];
Node* head = NULL;   // ← единственное ОПРЕДЕЛЕНИЕ во всей программе


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


// Вывод краткого списка (ID + ФИО)
void show_contact_list_brief() {
    printf("\n--- Список контактов ---\n");
    Node* current = head;
    int id = 0;
    if (!current) {
        printf("Нет сохранённых контактов.\n");
        return;
    }
    while (current) {
        printf("ID: %d — %s %s\n", id, current->data.surname, current->data.name);
        current = current->next;
        id++;
    }
}


// Получение узла по ID (позиции в списке)
Node* get_node_by_id(int id) {
    Node* current = head;
    int index = 0;
    while (current && index < id) {
        current = current->next;
        index++;
    }
    return (index == id) ? current : NULL;
}

// Просмотр контакта по ID
void view_contact_details() {
    int id;
    printf("Введите ID контакта для просмотра подробной информации: ");
    if (scanf("%d", &id) != 1) {
        printf("Ошибка ввода.\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');

    Node* node = get_node_by_id(id);
    if (!node) {
        printf("Контакт с ID %d не найден.\n", id);
        return;
    }

    printf("\n--- Подробная информация о контакте (ID: %d) ---\n", id);
    printf("Ф.И.О.: %s %s\n", node->data.surname, node->data.name);
    if (strlen(node->data.workplace) > 0)
        printf("Место работы: %s\n", node->data.workplace);
    if (strlen(node->data.position) > 0)
        printf("Должность: %s\n", node->data.position);
    if (strlen(node->data.phone) > 0)
        printf("Телефон(ы): %s\n", node->data.phone);
    if (strlen(node->data.email) > 0)
        printf("Email(ы): %s\n", node->data.email);
    if (strlen(node->data.social.telegram) > 0)
        printf("Telegram: %s\n", node->data.social.telegram);
    if (strlen(node->data.social.vk) > 0)
        printf("VK: %s\n", node->data.social.vk);
    if (strlen(node->data.social.whatsapp) > 0)
        printf("WhatsApp: %s\n", node->data.social.whatsapp);
    if (strlen(node->data.social.instagram) > 0)
        printf("Instagram: %s\n", node->data.social.instagram);
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

//###################################################
// Создание нового узла
Node* create_node(const Contact* contact) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (!new_node) return NULL;
    new_node->data = *contact;
    new_node->prev = NULL;
    new_node->next = NULL;
    return new_node;
}
// Сравнение двух контактов (по фамилии, затем по имени)
int compare_contacts(const Contact* a, const Contact* b) {
    int cmp = strcmp(a->surname, b->surname);
    if (cmp != 0) return cmp;
    return strcmp(a->name, b->name);
}

// Вставка контакта в упорядоченный список
void insert_toGlobal(Contact* contact) {
    Node* new_node = create_node(contact);
    if (!new_node) {
        printf("Ошибка выделения памяти.\n");
        return;
    }

    if (!head) {
        head = new_node;
        return;
    }

    Node* current = head;
    while (current) {
        int cmp = compare_contacts(contact, &current->data);
        if (cmp <= 0) {
            // Вставляем перед current
            new_node->prev = current->prev;
            new_node->next = current;
            if (current->prev) {
                current->prev->next = new_node;
            } else {
                head = new_node;
            }
            current->prev = new_node;
            return;
        }
        if (!current->next) break;
        current = current->next;
    }

    // Вставляем в конец
    current->next = new_node;
    new_node->prev = current;
}

// Добавление контакта
void UI_addContact() {
    Contact new_contact = {0};

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

    insert_toGlobal(&new_contact); // вставка в упорядоченный список
    printf("Контакт успешно добавлен!\n");
}

//################################################





// Редактирование контакта по ID
void edit_contact() {
    int id;
    printf("Введите ID контакта для редактирования: ");
    if (scanf("%d", &id) != 1) {
        printf("Ошибка ввода.\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');

    Node* node = get_node_by_id(id);
    if (!node) {
        printf("Контакт с ID %d не найден.\n", id);
        return;
    }

    printf("\n--- Редактирование контакта (ID: %d) ---\n", id);
    char temp[MAX_STR];

    // Фамилия
    printf("Фамилия (текущая: %s): ", node->data.surname);
    safe_input("", temp, MAX_STR);
    if (strlen(temp) > 0) strcpy(node->data.surname, temp);

    // Имя
    printf("Имя (текущее: %s): ", node->data.name);
    safe_input("", temp, MAX_STR);
    if (strlen(temp) > 0) strcpy(node->data.name, temp);

    // Проверка обязательных полей
    if (strlen(node->data.surname) == 0 || strlen(node->data.name) == 0) {
        printf("Ошибка: фамилия и имя обязательны!\n");
        return;
    }

    // Остальные поля
    printf("Место работы (текущее: %s): ", node->data.workplace[0] ? node->data.workplace : "(не задано)");
    safe_input("", temp, MAX_STR);
    if (strlen(temp) > 0) strcpy(node->data.workplace, temp);

    printf("Должность (текущая: %s): ", node->data.position[0] ? node->data.position : "(не задано)");
    safe_input("", temp, MAX_STR);
    if (strlen(temp) > 0) strcpy(node->data.position, temp);

    printf("Телефон(ы) (текущие: %s): ", node->data.phone[0] ? node->data.phone : "(не задано)");
    safe_input("", temp, MAX_STR);
    if (strlen(temp) > 0) strcpy(node->data.phone, temp);

    printf("Email(ы) (текущие: %s): ", node->data.email[0] ? node->data.email : "(не задано)");
    safe_input("", temp, MAX_STR);
    if (strlen(temp) > 0) strcpy(node->data.email, temp);

    printf("Telegram (текущий: %s): ", node->data.social.telegram[0] ? node->data.social.telegram : "(не задано)");
    safe_input("", temp, MAX_STR);
    if (strlen(temp) > 0) strcpy(node->data.social.telegram, temp);

    printf("VK (текущий: %s): ", node->data.social.vk[0] ? node->data.social.vk : "(не задано)");
    safe_input("", temp, MAX_STR);
    if (strlen(temp) > 0) strcpy(node->data.social.vk, temp);

    printf("WhatsApp (текущий: %s): ", node->data.social.whatsapp[0] ? node->data.social.whatsapp : "(не задано)");
    safe_input("", temp, MAX_STR);
    if (strlen(temp) > 0) strcpy(node->data.social.whatsapp, temp);

    printf("Instagram (текущий: %s): ", node->data.social.instagram[0] ? node->data.social.instagram : "(не задано)");
    safe_input("", temp, MAX_STR);
    if (strlen(temp) > 0) strcpy(node->data.social.instagram, temp);

    printf("Контакт успешно обновлён!\n");
}


//#########################
// Удаление узла по указателю
void delete_node(Node** head_ref, Node* node_to_delete) {
    if (!node_to_delete) return;

    if (node_to_delete->prev) {
        node_to_delete->prev->next = node_to_delete->next;
    } else {
        *head_ref = node_to_delete->next;
    }

    if (node_to_delete->next) {
        node_to_delete->next->prev = node_to_delete->prev;
    }

    free(node_to_delete);
}

// Удаление контакта по ID
void delete_contact() {
    int id;
    printf("Введите ID контакта для удаления: ");
    if (scanf("%d", &id) != 1) {
        printf("Ошибка ввода.\n");
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');

    Node* node = get_node_by_id(id);
    if (!node) {
        printf("Контакт с ID %d не найден.\n", id);
        return;
    }

    delete_node(&head, node);
    printf("Контакт с ID %d удалён.\n", id);
}


// Главное меню
void menu() {
    printf("\n=== Телефонная книга (двусвязный список) ===\n");
    printf("1. Добавить контакт\n");
    printf("2. Показать краткий список (ID + ФИО)\n");
    printf("3. Просмотреть контакт по ID\n");
    printf("4. Редактировать контакт\n");
    printf("5. Удалить контакт\n");
    printf("0. Выход\n");
    printf("Выберите действие: ");
}