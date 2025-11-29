#include "header4.3.h"

int main(){
    setlocale(LC_ALL, "ru-RU");
    

    PhoneBookTree* pb = create_phonebook_tree();
    if (!pb) {
        fprintf(stderr, "Не удалось создать телефонную книгу.\n");
        return 1;
    }

    int choice;

   do {
        menu();
        if (scanf("%d", &choice) != 1) {
            choice = -1; // неверный ввод
        }
        while (getchar() != '\n'); // очистка буфера

        switch (choice) {
            case 1:
                UI_addContact(pb);
                break;
            case 2:
                show_contact_list_brief(pb);
                break;
            case 3:
                show_contact_list_brief(pb);
                view_contact_details(pb);
                break;
            case 4:
                show_contact_list_brief(pb);
                edit_contact(pb);
                break;
            case 5:
                show_contact_list_brief(pb);
                delete_contact(pb);
                break;
            case 6:  // ← НОВЫЙ CASE
                show_tree_structure_simple_with_depth(pb);
                break;
            case 0:
                printf("Выход из программы.\n");
                break;
            default:
                printf("Неверный выбор. Попробуйте снова.\n");
        }
    } while (choice != 0);

    destroy_phonebook_tree(pb->root);
    free(pb);
    return 0;
}