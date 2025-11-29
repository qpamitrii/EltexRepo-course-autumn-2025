#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <stdlib.h>

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
    char surname[MAX_STR];
    char name[MAX_STR];
    char workplace[MAX_STR];
    char position[MAX_STR];
    char phone[MAX_STR];
    char email[MAX_STR];
    SocialMedia social;
    int used; // флаг: 1 — контакт занят, 0 — свободен
} Contact;

typedef struct Node {
    Contact data;
    struct Node* prev;
    struct Node* next;
} Node;

//Node* head = NULL;
// — только ОБЪЯВЛЕНИЕ (не определение!)
extern Node* head;   // ← extern = "где-то есть определение, не здесь"

void safe_input(char* prompt, char* buffer, size_t size);
void show_contact_list_brief();

Node* get_node_by_id(int id);
void view_contact_details();

// Создание нового узла
Node* create_node(const Contact* contact);
// Сравнение двух контактов (по фамилии, затем по имени)
int compare_contacts(const Contact* a, const Contact* b);
// Вставка контакта в упорядоченный список
void insert_toGlobal(Contact* contact);
// Добавление контакта
void UI_addContact();

void edit_contact();

// Удаление узла по указателю
void delete_node(Node** head_ref, Node* node_to_delete);
void delete_contact();
void menu();