#include "db.h"
#include <stdio.h>
int main() {
    load_db();

    insert_m("Instrument 1", "Type 1", "Country 1");
    insert_m("Instrument 2", "Type 2", "Country 2");
    insert_m("Instrument 3", "Type 3", "Country 3");

    insert_s(1, "Material 1", 11.1f);
    insert_s(1, "Material 2", 22.2f);
    insert_s(1, "Material 3", 33.3f);
    insert_s(3, "Material 4", 44.4f);

    printf("\n[[INITIAL INSERTION]]\n");
    ut_m(true);
    printf("=====\n");
    ut_s(true);
    printf("=====\n=====\n");

    del_m(1);
    del_s_of_m(4, 5);

    printf("\n[[REMOVAL]]\n");
    ut_m(true);
    printf("=====\n");
    ut_s(true);
    printf("=====\n=====\n");

    defragment_m();
    defragment_s();

    printf("\n[[DEFRAGMENTATION]]\n");
    ut_m(true);
    printf("=====\n");
    ut_s(true);
    printf("=====\n=====\n");

    insert_m("Instrument 6", "Type 6", "Country 6");
    insert_s(6, "Material 7", 77.7f);

    printf("\n[[INSERTION]]\n");
    ut_m(true);
    printf("=====\n");
    ut_s(true);
    printf("=====\n=====\n");

    update_m(2, "New Instrument 2", "New Type 2", "New Country 2");
    update_s_of_m(4, 6, "New Material 6", 88.8f);

    printf("\n[[UPDATE]]\n");
    ut_m(true);
    printf("=====\n");
    ut_s(true);
    printf("=====\n=====\n");

    onclose_db();
    return 0;
}

