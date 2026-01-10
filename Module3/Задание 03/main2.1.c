#include "header2.1.h"

int main(){
    setlocale(LC_ALL, "ru-RU");
    init_phonebook();

    loadContacts();

    int choice;

   do {
        menu();
        if (scanf("%d", &choice) != 1) {
            choice = -1; // неверный ввод
        }
        while (getchar() != '\n'); // очистка буфера

        switch (choice) {
            case 1:
                UI_addContact();
                break;
            case 2:
                //showListContact();
                show_contact_list_brief();
                break;
            case 3:
                show_contact_list_brief(); // сначала показываем список
                view_contact_details();    // затем запрашиваем ID
                break;
            case 4:
                //showListContact();
                show_contact_list_brief();
                edit_contact();
                break;
            case 5:
                //showListContact();
                show_contact_list_brief();
                delete_contact();
                break;
            case 0:
                printf("Выход из программы.\n");
                break;
            default:
                printf("Неверный выбор. Попробуйте снова.\n");
        }
    } while (choice != 0);

    saveContacts();
    return 0;
}