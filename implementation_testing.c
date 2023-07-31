#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


#define DEBUG_LEVEL 2
#include "helper_functions/debug.h"

#include "helper_functions/ipv4_getters/ipv4_getters.h"
#include "helper_functions/linked_list.h"
#include "helper_functions/circular_array.h"
#include "helper_functions/configuration_reader.h"
#include "helper_functions/configuration_reader_common.h"

#define LINKED_LIST_TYPE int
#define ITERATOR_PREFIX INT
#include "helper_functions/linked_list_comfort.h"

#include "helper_functions/ipv6_getters/local_interface_data.h"

int linked_list_testing(void);
int configuration_reader_common_testing(void);
int configuration_reader_testing(void);
int ipv4_address_testing(void);
int ipv6_address_testing(void);
int circular_array_testing(void);

int test_implementation(void){
    int ret = 0;

    ret += linked_list_testing();
    ret += configuration_reader_common_testing();
    ret += configuration_reader_testing();
    ret += ipv6_address_testing();
    ret += ipv4_address_testing();
    ret += circular_array_testing();


    if(!ret){
        printf("\n***no errors occured, everything works fine, good job.\n\n");
    }else{
        printf("\n***!!! ERRORs occured during testing  .\n\n");
    }
    return ret;
}

static void free_deref(void* ptr){
    void ** ptr_to_ptr_to_free = (void**) ptr;
    DEBUG_PRINT_2("freeing %p, stored at %p\n", *ptr_to_ptr_to_free, ptr);
    free(*ptr_to_ptr_to_free);
}

int circular_array_testing(void){
    circular_array testing_array;
//    assert(circular_array_init(&testing_array, sizeof(char*), 0, free) == NULL);
    assert(circular_array_init(&testing_array, sizeof(char*), 100, free_deref) != NULL);
    assert(testing_array.free_func == free_deref);
    char* ptr = malloc(40);
    testing_array.free_func(&ptr);
    assert(circular_array_is_empty(&testing_array));

    char* testing_char = malloc(4);
    testing_char[0] = 'o';
    testing_char[1] = 'l';
    testing_char[2] = 'a';
    testing_char[3] = '\0';
    assert(circular_array_append(&testing_array, &testing_char) == 0);


    char* testing_char2 = malloc(2);
    testing_char2[0] = '_';
    testing_char2[1] = '\0';
    assert(circular_array_append(&testing_array, &testing_char2) == 1);

    assert(!circular_array_is_empty(&testing_array));
    assert(circular_array_size(&testing_array) == 2);

    //overfill array
    for(int i = 0; i < 1000; i++){
        char* value = malloc(1);
        *value = 'q';
        assert(circular_array_append(&testing_array, &value) >= 0);
    }
    assert(circular_array_size(&testing_array) == 100);

    //test contents with iterator

    circular_array_free(&testing_array);

    return 0;
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

    return RETURN_SUCCESS;
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

    current_ptr = "   \nkY4:    \r\n value   \t\r\n";
    key_data key = read_key(&current_ptr);
    assert(key.length == 3);
    assert(*key.start == 'k');
    assert(key.start[1] == 'Y');
    assert(key.start[2] == '4');

    assert(key_matches(key, "kY4"));
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
//    assert(provider.get_dns_state != cloudflare_update_dns);
    assert(provider.read_provider_data == cloudflare_read_provider_data);
    assert(provider.update_dns == cloudflare_update_dns);

    return RETURN_SUCCESS;
}

int configuration_reader_testing(void){
    printf("\n***testing the configuration reader\n\n");
    const char *string = "  \n\n  \r\n\t  enableHTTPServer:\t \r\n true\n";
    struct updater_data * config_data = read_config_from_string(string);
    assert(config_data->config.enable_http_server);
    free_config(config_data);


    string = "enableHTTPServer: \r\n true, \t\t    dnsEntries: \n[\n  \r\t{  \n  \n type: AAAA, name: test.de}\n  , \t\n\r\n  {type: A, name: TEST5.de, provider: cloudflare, providerData:apiKey}]";
    config_data = read_config_from_string(string);


    assert(linked_list_size(config_data->managed_dns_list) == 2);

    struct managed_dns_entry *dns_entry0 = dns_linked_list_get(config_data->managed_dns_list, 0);
    struct managed_dns_entry *dns_entry1 = dns_linked_list_get(config_data->managed_dns_list, 1);

    assert(!strcmp(dns_entry0->dns_data.dns_type, "AAAA"));
    assert(strcmp(dns_entry0->dns_data.dns_type, "AAAB"));
    assert(!strcmp(dns_entry0->dns_data.dns_name, "test.de"));
    assert(strcmp(dns_entry0->dns_data.dns_name, "tesT.de"));

    assert(!strcmp(dns_entry1->dns_data.dns_type, "A"));
    assert(!strcmp(dns_entry1->dns_data.dns_name, "TEST5.de"));

    struct cloudflare_provider_data *cloudflare_data = dns_entry1->dns_data.provider_data;

    DEBUG_PRINT_1("cloudflare test api_key: %s\n", cloudflare_data->api_key);
    assert(!strcmp(cloudflare_data->api_key, "apiKey"));

    free_config(config_data);

    return RETURN_SUCCESS;
}

int ipv4_address_testing(void){

    struct in_addr *in4addr = get_ipv4_address_whats_my_ip();
    if(in4addr == NULL){
        DEBUG_PRINT_0("no suitable ipv4 found, likely an error...\n");
        return RETURN_ERROR;
    }

    uint8_t * adress_bytes = (uint8_t *) in4addr;

    DEBUG_PRINT_1("found the following ipv4 adress, ensure it's the right one\n");
    DEBUG_PRINT_1("%hhu.%hhu.%hhu.%hhu\n", adress_bytes[3], adress_bytes[2], adress_bytes[1], adress_bytes[0]);


    free(in4addr);
    return RETURN_SUCCESS;
}

int ipv6_address_testing(void){
    struct in6_addr * in6addr = get_ipv6_address_local_interface();
    if(in6addr == NULL){
        DEBUG_PRINT_0("warning: no suitable ipv6 found, skipping...\n");
        return RETURN_SUCCESS;
    }

    DEBUG_PRINT_1("found the following ipv6 adress, ensure it's the right one\n");
    for(int i = 0; i<16; i++){
        if(i % 2 == 0 && i != 0){
            DEBUG_PRINT_1(":");
        }
        DEBUG_PRINT_1("%.2x", in6addr->__in6_u.__u6_addr8[i]);
    }
    DEBUG_PRINT_1("\n");

    free(in6addr);

    return RETURN_SUCCESS;
}