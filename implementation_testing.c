#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


#define DEBUG_LEVEL 1
#include "helper_functions/debug.h"

#include "helper_functions/linked_list.h"


#define LINKED_LIST_TYPE int
#define ITERATOR_PREFIX INT
#include "helper_functions/linked_list_comfort.h"

int linked_list_testing();

int test_implementation(void){
    int ret = 0;

    linked_list_testing();
    return ret;
}

int linked_list_testing(void){
    linked_list testing_list;

    linked_list_init(&testing_list);
    assert(linked_list_is_empty(&testing_list));

    //fill up the empty list
    for(int i = 100; i < 120; i++){
        int_linked_list_insert(&testing_list, i);
    }

    //assume it is not empty anymore
    assert(!linked_list_is_empty(&testing_list));
    assert(linked_list_size(&testing_list) == 20);

    //check the contents via an iterator
    iterator iter;
    linked_list_iterator_init(&testing_list, &iter);
    for(int i = 100; i < 120; i++){
        printf("comparing value %d\n", i);
        assert(i == *INT_ITERATOR_NEXT(&iter));
    }
    assert(INT_ITERATOR_NEXT(&iter) == NULL);


    //check invalid accesses
    assert(int_linked_list_get(&testing_list, 20) == NULL);
    assert(int_linked_list_get(&testing_list, 4000) == NULL);

    //free the list, check via valgrind or something elso to make sure there are no memory leaks
    linked_list_free(&testing_list);

    return EXIT_SUCCESS;
}