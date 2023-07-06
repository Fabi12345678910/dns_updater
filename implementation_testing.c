#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


#define DEBUG_LEVEL 1
#include "helper_functions/debug.h"

#include "helper_functions/linked_list.h"
#include "helper_functions/configuration_reader.h"
#include "helper_functions/configuration_reader_common.h"

#define LINKED_LIST_TYPE int
#define ITERATOR_PREFIX INT
#include "helper_functions/linked_list_comfort.h"

int linked_list_testing(void);
int configuration_reader_common_testing(void);
int configuration_reader_testing(void);
int test_implementation(void){
    int ret = 0;

    ret += linked_list_testing();
    ret += configuration_reader_common_testing();
    return ret;
}

int linked_list_testing(void){
    printf("\n***linked list testing...\n\n");
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

int configuration_reader_common_testing(void){
    printf("\n***testing commons from configuration reader\n\n");

    int target_bool = 0;
    const char* string = "  \n  true";
    skip_empty_chars(&string);
    assert(*string == 't');

    const char* current_ptr = "    \ntrue\t  \n   false   ;;";
    printf("String1: %s\n", current_ptr);
    read_value_to_bool(&current_ptr, &target_bool);
    assert(target_bool);
    printf("String1: %s\n", current_ptr);

    read_value_to_bool(&current_ptr, &target_bool);
    assert(!target_bool);

    assert (!expect_char(&current_ptr, ';'));
    assert (!expect_char(&current_ptr, ';'));
    assert (expect_char(&current_ptr, ';'));

    current_ptr = "   \nky4:    \r\n value   \t\r\n";
    key_data key = read_key(&current_ptr);
    assert(key.length == 3);
    assert(*key.start == 'k');
    assert(key.start[1] == 'y');
    assert(key.start[2] == '4');

    assert(key_matches(key, "ky4"));
    assert(!key_matches(key, "ky"));
    assert(!key_matches(key, "key4"));

    char string_target[6] = {'a', 'a', 'a', 'a', 'a', 'a'};
    read_value_to_string(&current_ptr, string_target, sizeof(string_target));
    
    assert(!strcmp("value", string_target));
    assert(strcmp("valu6", string_target));

    current_ptr = "   \ncloudflare}  ";
    struct provider_functions provider;
    provider.read_provider_data = NULL;

    assert(provider.read_provider_data == NULL);
    read_provider(&current_ptr, &provider);
    assert(provider.read_provider_data != NULL);

    assert(provider.get_dns_state == cloudflare_get_dns_state);
    assert(provider.get_dns_state != cloudflare_update_dns);
    assert(provider.read_provider_data == cloudflare_read_provider_data);
    assert(provider.update_dns == cloudflare_update_dns);

    return RETURN_SUCCESS;
}

int configuration_reader_testing(void){
    
    return RETURN_SUCCESS;
}