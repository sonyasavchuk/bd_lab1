#include "db.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

void show_command_list() {
    printf("\nCommand list:\n");
    printf("\t#-1  save and close \n");
    printf("\t#-2  show command list \n");
    printf("\t#1  print M data file \n");
    printf("\t#2  print S data file \n");
    printf("\t#3  insert M item \n");
    printf("\t#4  insert S item \n");
    printf("\t#5  remove M item \n");
    printf("\t#6  remove S item \n");
    printf("\t#7  remove S item of exact M item \n");
    printf("\t#8  update M item \n");
    printf("\t#9  update S item \n");
    printf("\t#10 update S item of exact M item\n");
    printf("\t#11 get M item \n");
    printf("\t#12 get S item \n");
    printf("\t#13 get S item of exact M item \n");
    printf("\t#14 count M items \n");
    printf("\t#15 count S items \n");
    printf("\t#16 count S items of exact M item \n");
    printf("\t#17 defragment M file \n");
    printf("\t#18 defragment S file \n");
}

void console_interface() {
    int command = -1;
    struct Student *student = malloc(sizeof(struct Student));
    struct Information *inf = malloc(sizeof(struct Information));
    char input1[32], input2[32], input3[32], input4[32];

    while (true) {
        command = -1;
        printf("\n----------\n");
        printf("Enter command:\n");
        printf("[#-2 - show command list)]\n");
        printf("\n#");
        scanf("%s", input1);
        command = strtol(input1, NULL, 10);
        switch (command) {
            case -1:
                return;
            case -2:
                show_command_list();
                break;
            case 1:
                printf("Printing M data file...\n\n");
                ut_m(true);
                printf("\n");
                break;
            case 2:
                printf("Printing S data file...\n\n");
                ut_s(true);
                printf("\n");
                break;
            case 3:
                printf("student's name: ");
                scanf("%s", input1);
                printf("student's surname: ");
                scanf("%s", input2);
                printf("student's grade: ");
                scanf("%s", input3);
                if (insert_m(input1, input2, input3)) {
                    printf("Insertion failed, items limit reached\n");
                } else {
                    printf("Item inserted\n");
                }
                break;
            case 4:
                printf("id of M item: ");
                scanf("%s", input1);
                printf("subject: ");
                scanf("%s", input2);
                printf("mark in this subject: ");
                scanf("%s", input3);
                if (insert_s(strtol(input1, NULL, 10), input2, strtof(input3, NULL))) {
                    printf("Insertion failed, M item was not found\n");
                } else {
                    printf("Item inserted\n");
                }
                break;
            case 5:
                printf("id of M item: ");
                scanf("%s", input1);
                if (del_m(strtol(input1, NULL, 10))) {
                    printf("Removal failed, item was not found\n");
                } else {
                    printf("Item removed\n");
                }
                break;
            case 6:
                printf("id of S item: ");
                scanf("%s", input1);

                if (del_s(strtol(input1, NULL, 10))) {
                    printf("Removal failed, item was not found\n");
                } else {
                    printf("Item removed\n");
                }
                break;
            case 7:
                printf("id of M item: ");
                scanf("%s", input1);
                printf("id of S item: ");
                scanf("%s", input2);
                if (del_s_of_m(strtol(input1, NULL, 10), strtol(input2, NULL, 10))) {
                    printf("Removal failed, either M or S item was not found\n");
                } else {
                    printf("Item removed\n");
                }
                break;
            case 8:
                printf("id of M item: ");
                scanf("%s", input1);
                printf("new name: ");
                scanf("%s", input2);
                printf("new surname: ");
                scanf("%s", input3);
                printf("new grade: ");
                scanf("%s", input4);
                if (update_m(strtol(input1, NULL, 10), input2, input3, input4)) {
                    printf("Update failed, item was not found\n");
                } else {
                    printf("Item updated\n");
                }
                break;
            case 9:
                printf("id of S item: ");
                scanf("%s", input1);
                printf("new subject: ");
                scanf("%s", input2);
                printf("new mark: ");
                scanf("%s", input3);
                if (update_s(strtol(input1, NULL, 10), input2, strtof(input3, NULL))) {
                    printf("Update failed, M item was not found\n");
                } else {
                    printf("Item updated\n");
                }
                break;
            case 10:
                printf("id of M item: ");
                scanf("%s", input1);
                printf("id of S item: ");
                scanf("%s", input2);
                printf("new subject: ");
                scanf("%s", input3);
                printf("new mark: ");
                scanf("%s", input4);
                if (update_s_of_m(strtol(input1, NULL, 10), strtol(input2, NULL, 10), input3, strtof(input4, NULL))) {
                    printf("Update failed, M item was not found\n");
                } else {
                    printf("Item updated\n");
                }
                break;
            case 11:
                printf("id of M item: ");
                scanf("%s", input1);
                student = get_m(strtol(input1, NULL, 10));
                if (student) {
                    printf("id: %d \n", student->id);
                    printf("name: %s \n", student->name);
                    printf("surname: %s \n", student->surname);
                    printf("grade: %s \n", student->grade);
                } else {
                    printf("Item was not found\n");
                }
                break;
            case 12:
                printf("id of S item: ");
                scanf("%s", input1);
                inf = get_s(strtol(input1, NULL, 10));
                if (student) {
                    printf("id: %d \n", inf->id);
                    printf("subject: %s \n", inf->subject);
                    printf("subject mark: %f \n", inf->gpa);
                } else {
                    printf("Item was not found\n");
                }
                break;
            case 13:
                printf("id of M item: ");
                scanf("%s", input1);
                printf("id of S item: ");
                scanf("%s", input2);
                inf = get_s_of_m(strtol(input1, NULL, 10), strtol(input1, NULL, 10));
                if (student) {
                    printf("id: %d \n", inf->id);
                    printf("subject: %s \n", inf->subject);
                    printf("grade in this subject: %f \n", inf->gpa);
                } else {
                    printf("Item was not found\n");
                }
                break;
            case 14:
                printf("Number of valid M items: %d\n", size_m());
                break;
            case 15:
                printf("Number of valid S items: %d\n", size_s());
                break;
            case 16:
                printf("id of M item: ");
                scanf("%s", input1);
                int res = size_s_of_m(strtol(input1, NULL, 10));
                if (res != -1) {
                    printf("Number of valid S items: %d\n", res);
                } else {
                    printf("Item was not found\n");
                }
                break;
            case 17:
                defragment_m();
                printf("Defragmentation of M file done\n");
                break;
            case 18:
                defragment_s();
                printf("Defragmentation of S file done\n");
                break;
            default:
                break;
        }
    }
}


