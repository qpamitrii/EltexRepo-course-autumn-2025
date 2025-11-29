#include <stdio.h>
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
    char surname[MAX_STR];
    char name[MAX_STR];
    char workplace[MAX_STR];
    char position[MAX_STR];
    char phone[MAX_STR];
    char email[MAX_STR];
    SocialMedia social;
    int used; // флаг: 1 — контакт занят, 0 — свободен
} Contact;

void init_phonebook();
void safe_input(char* prompt, char* buffer, size_t size);
void show_contact_list_brief();
void view_contact_details();

void UI_addContact();
int add_contact_to_book(Contact* book, int max_size, const Contact* contact);

void edit_contact();
void delete_contact();
void menu();