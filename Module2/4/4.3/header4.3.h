#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#define MAX_CONTACTS 100
#define MAX_STR 100

typedef struct {
    char telegram[MAX_STR];
    char vk[MAX_STR];
    char whatsapp[MAX_STR];
    char instagram[MAX_STR];
} SocialMedia;

typedef struct
{
    int id;
    char surname[MAX_STR];
    char name[MAX_STR];
    char workplace[MAX_STR];
    char position[MAX_STR];
    char phone[MAX_STR];
    char email[MAX_STR];
    SocialMedia social;
    int used; // флаг: 1 — контакт занят, 0 — свободен
} Contact;



typedef struct TreeNode {
    Contact data;               // данные контакта
    int id;                     // уникальный ID (можно использовать как ключ)
    struct TreeNode* left;
    struct TreeNode* right;
    int height;                 // для AVL-балансировки
} TreeNode;

typedef struct {
    TreeNode* root;
    int size;                   // текущее количество элементов
    int change_count;  // ← новый счётчик: вставка + удаление + редактирование
} PhoneBookTree;




// ——— Создание/удаление дерева ———
PhoneBookTree* create_phonebook_tree(void);
void destroy_phonebook_tree(TreeNode* node);

// ——— Работа с деревом ———
int add_contact_to_tree(PhoneBookTree* pb, Contact* contact);
TreeNode* find_node(TreeNode* node, int id);
TreeNode* delete_node(TreeNode* node, int id);
void delete_contact(PhoneBookTree* pb);
void edit_contact(PhoneBookTree* pb);
void view_contact_details(PhoneBookTree* pb);
void show_contact_list_brief(PhoneBookTree* pb);

// ——— UI и ввод ———
void UI_addContact(PhoneBookTree* pb);
int get_int_input(const char* prompt);
void safe_input(const char* prompt, char* buffer, size_t size);

// ——— AVL вспомогательные ———
int height(TreeNode* node);
void update_height(TreeNode* node);
TreeNode* rotate_right(TreeNode* y);
TreeNode* rotate_left(TreeNode* x);
TreeNode* insert_node(TreeNode* node, Contact* contact, int id);

// ——— Полная перебалансировка ———
void collect_nodes(TreeNode* node, TreeNode** arr, int* index);
TreeNode* build_balanced_tree(TreeNode** nodes, int start, int end);
TreeNode* full_rebalance(TreeNode* root);

void menu();

void update_heights_to_root(TreeNode* root);


void show_tree_structure_simple_with_depth(PhoneBookTree* pb);
void print_tree_simple_with_depth(TreeNode* node, char* prefix, int is_tail, int depth);