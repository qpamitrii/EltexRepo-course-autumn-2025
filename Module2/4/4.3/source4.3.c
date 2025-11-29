//Доработать решение задачи 2.1 (список контактов) так, чтобы 
//для хранения данных использовалось бинарное дерево. Добавить 
//периодическую балансировку дерева.

//перебалансировка каждые 4 изменения в дереве!!

#include "header4.3.h"


// Вспомогательные функции для высоты
int height(TreeNode* node) {
    return node ? node->height : 0;
}

void update_height(TreeNode* node) {
    if (node)
        node->height = 1 + (height(node->left) > height(node->right) ? height(node->left) : height(node->right));
}

// Ротации (для AVL, если нужно, но в нашей задаче — только для перебалансировки)
TreeNode* rotate_right(TreeNode* y) {
    TreeNode* x = y->left;
    TreeNode* T2 = x->right;

    x->right = y;
    y->left = T2;

    update_height(y);
    update_height(x);
    return x;
}

TreeNode* rotate_left(TreeNode* x) {
    TreeNode* y = x->right;
    TreeNode* T2 = y->left;

    y->left = x;
    x->right = T2;

    update_height(x);
    update_height(y);
    return y;
}

// Создание пустой телефонной книги
PhoneBookTree* create_phonebook_tree() {
    PhoneBookTree* pb = (PhoneBookTree*)malloc(sizeof(PhoneBookTree));
    if (!pb) return NULL;

    pb->root = NULL;
    pb->size = 0;
    pb->change_count = 0;
    return pb;
}

// Уничтожение всего дерева
void destroy_phonebook_tree(TreeNode* root) {
    if (root) {
        destroy_phonebook_tree(root->left);
        destroy_phonebook_tree(root->right);
        free(root);
    }
}


// Реализация get_int_input
int get_int_input(const char* prompt) {
    if (prompt && *prompt)
        printf("%s", prompt);
    int val;
    char buf[100];
    if (fgets(buf, sizeof(buf), stdin) == NULL)
        return -1;
    if (sscanf(buf, "%d", &val) != 1)
        return -1;
    return val;
}

void safe_input(const char* prompt, char* buffer, size_t size) {
    if (prompt && *prompt)
        printf("%s", prompt);
    if (fgets(buffer, size, stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n')
            buffer[len - 1] = '\0';
    }
}

//##################################################
//Просмотр телеф книги

// Рекурсивный in-order обход дерева для краткого вывода
void print_brief_inorder(TreeNode* node) {
    if (node == NULL) return;
    print_brief_inorder(node->left);
    printf("ID: %d | %s %s\n", node->data.id, node->data.surname, node->data.name);
    print_brief_inorder(node->right);
}
// Показать краткий список (ID + ФИО)
void show_contact_list_brief(PhoneBookTree* pb) {
    if (!pb || !pb->root) {
        printf("Телефонная книга пуста.\n");
        return;
    }

    printf("\n--- Список контактов ---\n");
    print_brief_inorder(pb->root);  // ← теперь вызов корректен
}

// Просмотреть контакт по ID
void view_contact_details(PhoneBookTree* pb) {
    if (!pb) return;
    int id = get_int_input("Введите ID контакта для просмотра: ");
    if (id < 0) {
        printf("Неверный ID!\n");
        return;
    }

    TreeNode* node = find_node(pb->root, id);
    if (!node) {
        printf("Контакт с ID %d не найден.\n", id);
        return;
    }

    Contact* c = &node->data;
    printf("\n=== Детали контакта (ID: %d) ===\n", c->id);
    printf("ФИО: %s %s\n", c->surname, c->name);
    printf("Место работы: %s\n", c->workplace);
    printf("Должность: %s\n", c->position);
    printf("Телефон: %s\n", c->phone);
    printf("Email: %s\n", c->email);
    printf("Telegram: %s\n", c->social.telegram);
    printf("VK: %s\n", c->social.vk);
    printf("WhatsApp: %s\n", c->social.whatsapp);
    printf("Instagram: %s\n", c->social.instagram);
}



//##################################################
// Вставка узла по ID (бинарное дерево по возрастанию ID)
TreeNode* insert_node(TreeNode* node, Contact* contact, int id) {
    if (!node) {
        TreeNode* new_node = malloc(sizeof(TreeNode));
        if (!new_node) return NULL;
        new_node->data = *contact;
        new_node->data.id = id;  // важно: перезаписываем ID!
        new_node->left = new_node->right = NULL;
        new_node->height = 1;
        return new_node;
    }

    if (id < node->data.id) {
        node->left = insert_node(node->left, contact, id);
    } else if (id > node->data.id) {
        node->right = insert_node(node->right, contact, id);
    } else {
        // ID уже существует — это нормально при авто-ID, просто обновляем данные
        printf("🔄 Контакт с ID %d обновлен\n", id);
        node->data = *contact;
        node->data.id = id;  // сохраняем оригинальный ID
        return node;  // возвращаем тот же узел
    }

    update_height(node);
    return node;
}

// Основная функция добавления контакта в дерево
int add_contact_to_tree(PhoneBookTree* pb, Contact* contact) {
    if (!pb || !contact) return -1;

    int new_id = pb->size;
    
    // Создаем новый узел
    TreeNode* new_node = malloc(sizeof(TreeNode));
    if (!new_node) return -1;
    
    new_node->data = *contact;
    new_node->data.id = new_id;
    new_node->left = new_node->right = NULL;
    new_node->height = 1;

    // Вставляем в дерево
    if (!pb->root) {
        pb->root = new_node;
    } else {
        // Ищем место для вставки
        TreeNode* current = pb->root;
        TreeNode* parent = NULL;
        
        while (current) {
            parent = current;
            if (new_id < current->data.id) {
                current = current->left;
            } else {
                current = current->right;
            }
        }
        
        // Вставляем как потомка parent
        if (new_id < parent->data.id) {
            parent->left = new_node;
        } else {
            parent->right = new_node;
        }
        
        // ОБНОВЛЯЕМ ВЫСОТЫ ВСЕГО ДЕРЕВА от корня
        update_heights_to_root(pb->root);
    }

    pb->size++;
    pb->change_count++;

    // Периодическая балансировка
    if (pb->change_count % 4 == 0) {
        printf("Периодическая полная перебалансировка дерева...\n");
        pb->root = full_rebalance(pb->root);
    }

    return new_id;
}

// Вспомогательная функция для обновления высот до корня
void update_heights_to_root(TreeNode* node) {
    if (!node) return;
    
    // Рекурсивно обновляем высоты от листьев к корню
    if (node->left) {
        update_heights_to_root(node->left);
    }
    if (node->right) {
        update_heights_to_root(node->right);
    }
    
    update_height(node);
}

// Интерфейс добавления контакта через UI
void UI_addContact(PhoneBookTree* pb) {
    if (!pb) return;

    Contact new_contact = {0};
    char temp[MAX_STR];

    printf("\n--- Добавление нового контакта ---\n");

    printf("Фамилия: ");
    safe_input("", temp, MAX_STR);
    strcpy(new_contact.surname, temp);

    printf("Имя: ");
    safe_input("", temp, MAX_STR);
    strcpy(new_contact.name, temp);

    if (!*new_contact.surname || !*new_contact.name) {
        printf("Ошибка: фамилия и имя обязательны!\n");
        return;
    }

    printf("Место работы: ");
    safe_input("", temp, MAX_STR);
    strcpy(new_contact.workplace, temp);

    printf("Должность: ");
    safe_input("", temp, MAX_STR);
    strcpy(new_contact.position, temp);

    printf("Телефон(ы): ");
    safe_input("", temp, MAX_STR);
    strcpy(new_contact.phone, temp);

    printf("Email(ы): ");
    safe_input("", temp, MAX_STR);
    strcpy(new_contact.email, temp);

    printf("Telegram: ");
    safe_input("", temp, MAX_STR);
    strcpy(new_contact.social.telegram, temp);

    printf("VK: ");
    safe_input("", temp, MAX_STR);
    strcpy(new_contact.social.vk, temp);

    printf("WhatsApp: ");
    safe_input("", temp, MAX_STR);
    strcpy(new_contact.social.whatsapp, temp);

    printf("Instagram: ");
    safe_input("", temp, MAX_STR);
    strcpy(new_contact.social.instagram, temp);

    int id = add_contact_to_tree(pb, &new_contact);
    if (id >= 0) {
        printf("Контакт успешно добавлен! ID: %d\n", id);
    } else {
        printf("Не удалось добавить контакт.\n");
    }
}


// Удаление узла по ID
TreeNode* delete_node(TreeNode* node, int id) {
    if (!node) return node;

    if (id < node->data.id) {
        node->left = delete_node(node->left, id);
    } else if (id > node->data.id) {
        node->right = delete_node(node->right, id);
    } else {
        // Нашли узел для удаления
        if (!node->left) {
            TreeNode* temp = node->right;
            free(node);
            return temp;
        } else if (!node->right) {
            TreeNode* temp = node->left;
            free(node);
            return temp;
        }

        // Узел имеет двух потомков — находим минимальный в правом поддереве
        TreeNode* min_node = node->right;
        while (min_node->left)
            min_node = min_node->left;

        // Копируем данные минимального узла в текущий
        node->data = min_node->data;

        // Удаляем минимальный узел
        node->right = delete_node(node->right, min_node->data.id);
    }

    update_height(node);
    return node;
}
void delete_contact(PhoneBookTree* pb) {
    if (!pb) return;
    int id = get_int_input("Введите ID контакта для удаления: ");
    if (id < 0) {
        printf("Неверный ID!\n");
        return;
    }

    TreeNode* old_root = pb->root;
    TreeNode* new_root = delete_node(old_root, id);
    if (new_root == old_root) {
        printf("Контакт с ID %d не найден.\n", id);
        return;
    }

    pb->root = new_root;
    pb->size--;
    pb->change_count++;  // ← удалили — изменили дерево!

    // ОБНОВЛЯЕМ ВЫСОТЫ ВСЕГО ДЕРЕВА после удаления
    if (pb->root) {
        update_heights_to_root(pb->root);
    }

    if (pb->change_count % 4 == 0) {
        printf("Периодическая полная перебалансировка дерева...\n");
        pb->root = full_rebalance(pb->root);
    }

    printf("Контакт с ID %d удалён.\n", id);
}

// Поиск узла по ID
TreeNode* find_node(TreeNode* node, int id) {
    if (!node || node->data.id == id)
        return node;

    if (id < node->data.id)
        return find_node(node->left, id);
    else
        return find_node(node->right, id);
}
void edit_contact(PhoneBookTree* pb) {
    if (!pb) return;
    int id = get_int_input("Введите ID контакта для редактирования: ");
    if (id < 0) {
        printf("Неверный ID!\n");
        return;
    }

    TreeNode* node = find_node(pb->root, id);
    if (!node) {
        printf("Контакт с ID %d не найден.\n", id);
        return;
    }

    Contact* c = &node->data;
    printf("\n--- Редактирование контакта (ID: %d) ---\n", id);

    char temp[MAX_STR];
    #define EDIT_FIELD(field, name) \
        printf("%s (текущее: %s): ", name, c->field); \
        safe_input("", temp, MAX_STR); \
        if (*temp) strcpy(c->field, temp);

    EDIT_FIELD(surname, "Фамилия");
    EDIT_FIELD(name, "Имя");
    if (!*c->surname || !*c->name) {
        printf("Ошибка: фамилия и имя обязательны!\n");
        return;
    }

    EDIT_FIELD(workplace, "Место работы");
    EDIT_FIELD(position, "Должность");
    EDIT_FIELD(phone, "Телефон(ы)");
    EDIT_FIELD(email, "Email(ы)");

    EDIT_FIELD(social.telegram, "Telegram");
    EDIT_FIELD(social.vk, "VK");
    EDIT_FIELD(social.whatsapp, "WhatsApp");
    EDIT_FIELD(social.instagram, "Instagram");

    printf("Контакт успешно обновлён!\n");

    pb->change_count++;  // ← отредактировали — изменили дерево!

    if (pb->change_count % 4 == 0) {
        printf("🔄 Периодическая полная перебалансировка дерева...\n");
        pb->root = full_rebalance(pb->root);
    }
}

// Сбор всех узлов в массив
void collect_nodes(TreeNode* node, TreeNode** arr, int* index) {
    if (node) {
        collect_nodes(node->left, arr, index);
        arr[(*index)++] = node;
        collect_nodes(node->right, arr, index);
    }
}

// Построение идеально сбалансированного дерева из отсортированного массива
TreeNode* build_balanced_tree(TreeNode** nodes, int start, int end) {
    if (start > end) return NULL;

    int mid = (start + end) / 2;
    TreeNode* root = nodes[mid];  // используем существующий узел — не копируем!
    root->left = build_balanced_tree(nodes, start, mid - 1);
    root->right = build_balanced_tree(nodes, mid + 1, end);
    update_height(root);  // обновляем высоту
    return root;
}





//################################################################
// Главная функция перебалансировки
TreeNode* full_rebalance(TreeNode* root) {
    if (!root) return NULL;

    // Подсчитаем количество узлов
    int count = 0;
    TreeNode* temp = root;
    // Можно пройти по всему дереву, чтобы узнать размер
    // Но проще — собрать в массив и посчитать
    TreeNode** nodes = malloc(sizeof(TreeNode*) * 1000); // достаточно для 1000 контактов
    int index = 0;
    collect_nodes(root, nodes, &index);

    if (index == 0) {
        free(nodes);
        return NULL;
    }

    // Перестраиваем дерево
    TreeNode* new_root = build_balanced_tree(nodes, 0, index - 1);

    // Удаляем временный массив (не узлы!)
    free(nodes);

    printf("Дерево переустроено в идеально сбалансированное (высота: %d)\n", height(new_root));
    return new_root;
}





// Главное меню
void menu() {
    printf("\n=== Телефонная книга (бинарное-дерево)===\n");
    printf("1. Добавить контакт\n");
    printf("2. Показать краткий список (ID + ФИО)\n");
    printf("3. Просмотреть контакт по ID\n");   // ← новый пункт
    printf("4. Редактировать контакт\n");
    printf("5. Удалить контакт\n");
     printf("6. Показать структуру дерева\n");
    printf("0. Выход\n");
    printf("Выберите действие: ");
}




// Функция для красивого вывода дерева в консоли
// Простой вариант с правильными ветвями
void print_tree_simple_with_depth(TreeNode* node, char* prefix, int is_tail, int depth) {
    if (node == NULL) return;
    
    printf("%s", prefix);
    printf("%s", is_tail ? "└── " : "├── ");
    printf("ID: %d | %s %s (d:%d)\n", node->data.id, node->data.surname, 
           node->data.name, depth);
    
    char new_prefix[256];
    snprintf(new_prefix, sizeof(new_prefix), "%s%s", prefix, 
             is_tail ? "    " : "│   ");
    
    // Сначала правые (верхние), затем левые (нижние)
    if (node->right) {
        print_tree_simple_with_depth(node->right, new_prefix, (node->left == NULL), depth + 1);
    }
    if (node->left) {
        print_tree_simple_with_depth(node->left, new_prefix, 1, depth + 1);
    }
}

void show_tree_structure_simple_with_depth(PhoneBookTree* pb) {
    if (!pb || !pb->root) {
        printf("Дерево пустое.\n");
        return;
    }
    
    printf("\n--- Структура дерева ---\n");
    print_tree_simple_with_depth(pb->root, "", 1, 0);
    printf("------------------------\n");
}