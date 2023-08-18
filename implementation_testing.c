#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define DEBUG_LEVEL 2
#include "helper_functions/debug.h"

#include "helper_functions/common.h"

#include "helper_functions/ipv4_getters/ipv4_getters.h"
#include "helper_functions/ipv6_getters/ipv6_getters.h"
#include "helper_functions/linked_list.h"
#include "helper_functions/circular_array.h"
#include "helper_functions/configuration_reader.h"
#include "helper_functions/configuration_reader_common.h"
#include "thread_functions/thread_functions.h"

#define LINKED_LIST_TYPE int
#define ITERATOR_PREFIX INT
#include "helper_functions/linked_list_comfort.h"


int linked_list_testing(void);
int configuration_reader_common_testing(void);
int configuration_reader_testing(void);
int ipv4_address_testing(void);
int ipv6_address_testing(void);
int circular_array_testing(void);
int updater_testing(void);
int timer_testing(void);
int cloudflare_testing(void);

int test_implementation(void){
    int ret = 0;

    ret += linked_list_testing();
    ret += configuration_reader_common_testing();
    ret += configuration_reader_testing();
    ret += ipv6_address_testing();
    ret += ipv4_address_testing();
    ret += circular_array_testing();
    ret += cloudflare_testing();
#ifdef TEST_TIMER
    ret += updater_testing();
    ret += timer_testing();
#endif

    if(!ret){
        printf("\n***no errors occured, everything works fine, good job.\n\n");
    }else{
        printf("\n***!!! ERRORs occured during testing  .\n\n");
    }
    return ret;
}

int cloudflare_testing(void){
    struct dns_data testing_data = {.current_data = NULL, .dns_class = "", .dns_name = "cloudflare.com", .dns_type = "AAAA", .entry_state = STATE_UNDEFINED, .provider_data = NULL, .ttl = 360};
    DEBUG_PRINT_1("testing cloudflare dns servers, check wether outputs are okay\n");
    cloudflare_get_dns_state(&testing_data);
    assert(testing_data.current_data != NULL);
    free(testing_data.current_data);
    testing_data.current_data = NULL;

    testing_data.dns_type[1] = '\0';
    cloudflare_get_dns_state(&testing_data);
    assert(testing_data.current_data != NULL);
    free(testing_data.current_data);

    DEBUG_PRINT_1("testing update_dns with dummy api key, this should fail\n");
    struct cloudflare_provider_data dummy_provider_data = {.api_key="0123456789012345678901234567890123456789"};
    strcpy(testing_data.dns_name, "test.test");
    testing_data.provider_data = (void*) &dummy_provider_data;
    char *error_msg = cloudflare_update_dns(&testing_data, "1.2.3.4");
    if(error_msg != NULL){
        DEBUG_PRINT_1("cloudflare dns error message: %s", error_msg);
        free(error_msg);
        return RETURN_ERROR;
    }
    return RETURN_SUCCESS;
}

int timer_testing(void){
    printf("waiting 3 secs for timer...\n");
    pthread_t timer_thread;
    struct updater_data data;
    data.ipc_data.cond_update_shutdown_requested = (pthread_cond_t) PTHREAD_COND_INITIALIZER;
    data.ipc_data.mutex_update_shutdown_requested = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    data.ipc_data.update_requested = 0;
    data.ipc_data.shutdown_requested = 0;

    pthread_create(&timer_thread, NULL, timer_func, (void*) &data);
    while(!data.ipc_data.update_requested){
        sched_yield();
    }
    expect_fine(pthread_mutex_lock(&data.ipc_data.mutex_update_shutdown_requested));
    data.ipc_data.shutdown_requested = 1;
    expect_fine(pthread_mutex_unlock(&data.ipc_data.mutex_update_shutdown_requested));

    pthread_join(timer_thread, NULL);
    printf("proof weather timer has set the update_requested flag");
    assert(data.ipc_data.update_requested);

    return RETURN_SUCCESS;
}

struct in6_addr ip6addr_dummy;

int ip4_dummy_updater_count;
int ip6_dummy_updater_count;

struct in_addr * ip4_dummy_func(){
    static struct in_addr ip4addr_dummy = {.s_addr = 1};
    if(ip4addr_dummy.s_addr != 3){
        ip4addr_dummy.s_addr++;
    }
    DEBUG_PRINT_1("returning adress %u\n", ip4addr_dummy.s_addr);
    struct in_addr *ip4_malloced_dummy = malloc(sizeof(*ip4_malloced_dummy));
    *ip4_malloced_dummy = ip4addr_dummy;
    return ip4_malloced_dummy;
}

struct in6_addr * ip6_dummy_func(){
    static struct in6_addr ip6addr_dummy = {.__in6_u.__u6_addr32 = {0,0,0,0}};
    if(ip6addr_dummy.__in6_u.__u6_addr8[15] != 3){
        ip6addr_dummy.__in6_u.__u6_addr8[15]++;        
    }
    DEBUG_PRINT_1("returning adress ending with %u\n", ip6addr_dummy.__in6_u.__u6_addr32[0]);
    struct in6_addr *ip6_malloced_dummy = malloc(sizeof(*ip6_malloced_dummy));
    *ip6_malloced_dummy = ip6addr_dummy;
    return ip6_malloced_dummy;
}

static void free_deref(void* ptr){
    void ** ptr_to_ptr_to_free = (void**) ptr;
    DEBUG_PRINT_2("freeing %p, stored at %p\n", *ptr_to_ptr_to_free, ptr);
    free(*ptr_to_ptr_to_free);
}

int dummy_updates = 0;

char *dummy_updater(struct dns_data* dns_data, char * new_rdata){
    (void) dns_data;
    dummy_updates++;
    DEBUG_PRINT_1("updating dns entry with %s\n", new_rdata);
    return NULL;
}

int updater_testing(void){
    struct updater_data data;
    data.config.enable_http_server = 0;
    data.config.get_ipv4_address = ip4_dummy_func;
    data.config.get_ipv6_address = ip6_dummy_func;
    
    data.ipc_data.cond_update_shutdown_requested = (pthread_cond_t) PTHREAD_COND_INITIALIZER;
    data.ipc_data.info.ip4_state = STATE_UNDEFINED;
    data.ipc_data.info.ip6_state = STATE_UNDEFINED;
    errorIf(NULL == circular_array_init(&data.ipc_data.info.logging_array, sizeof(char*), 50, free_deref), "error init circular array\n");
    data.ipc_data.mutex_dns_list = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    data.ipc_data.mutex_info = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    data.ipc_data.mutex_update_shutdown_requested = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    data.ipc_data.shutdown_requested = data.ipc_data.update_requested = 0;
    data.managed_dns_list = malloc(sizeof(*data.managed_dns_list));
    linked_list_init(data.managed_dns_list);

    struct managed_dns_entry dummy_entry={.dns_data = {.current_data = NULL, .dns_class = "IN",
                                        .dns_name = "test.test.com", .dns_type= "A",
                                        .entry_state = STATE_UNDEFINED, .provider_data = NULL, .ttl = 360}};

    dummy_entry.provider.get_dns_state = NULL;
    dummy_entry.provider.read_provider_data = NULL;
    dummy_entry.provider.update_dns = dummy_updater;

    assert(dns_linked_list_insert(data.managed_dns_list, dummy_entry) == 0);

    struct managed_dns_entry dummy_entry_6 = dummy_entry;
    sprintf(dummy_entry_6.dns_data.dns_type, "AAAA");

    assert(dns_linked_list_insert(data.managed_dns_list, dummy_entry_6) == 1);

    pthread_t updater_thread;
    errorIf(pthread_create(&updater_thread, NULL, updater_func, (void*) &data), "error creating thread\n");

    DEBUG_PRINT_1("1. sleeping for 1s to try to make the updater wait\n");
    sleep(1);
    sched_yield();

    expect_fine(pthread_mutex_lock(&data.ipc_data.mutex_update_shutdown_requested));
    data.ipc_data.update_requested = 1;
    expect_fine(pthread_mutex_unlock(&data.ipc_data.mutex_update_shutdown_requested));
    expect_fine(pthread_cond_signal(&data.ipc_data.cond_update_shutdown_requested));

    DEBUG_PRINT_1("2. sleeping for 1s to try to make the updater wait\n");
    sleep(1);
    sched_yield();

    expect_fine(pthread_mutex_lock(&data.ipc_data.mutex_update_shutdown_requested));
    DEBUG_PRINT_1("locked mutex 2, requesting update\n");
    data.ipc_data.update_requested = 1;
    expect_fine(pthread_mutex_unlock(&data.ipc_data.mutex_update_shutdown_requested));
    expect_fine(pthread_cond_signal(&data.ipc_data.cond_update_shutdown_requested));

    DEBUG_PRINT_1("3. sleeping for 1s to try to make the updater wait\n");
    sleep(1);
    sched_yield();

    expect_fine(pthread_mutex_lock(&data.ipc_data.mutex_update_shutdown_requested));
    data.ipc_data.update_requested = 1;
    expect_fine(pthread_mutex_unlock(&data.ipc_data.mutex_update_shutdown_requested));
    expect_fine(pthread_cond_signal(&data.ipc_data.cond_update_shutdown_requested));

    DEBUG_PRINT_1("sleeping for 1s to try to make the updater wait\n");
    sleep(1);

    expect_fine(pthread_mutex_lock(&data.ipc_data.mutex_update_shutdown_requested));
    data.ipc_data.update_requested = 1;
    expect_fine(pthread_mutex_unlock(&data.ipc_data.mutex_update_shutdown_requested));
    expect_fine(pthread_cond_signal(&data.ipc_data.cond_update_shutdown_requested));

    DEBUG_PRINT_1("sleeping for 1s to try to make the updater wait\n");
    sleep(1);

    expect_fine(pthread_mutex_lock(&data.ipc_data.mutex_update_shutdown_requested));
    data.ipc_data.shutdown_requested = 1;
    expect_fine(pthread_mutex_unlock(&data.ipc_data.mutex_update_shutdown_requested));
    expect_fine(pthread_cond_signal(&data.ipc_data.cond_update_shutdown_requested));


    errorIf(pthread_join(updater_thread, NULL), "error joining updater_thread\n");

    DEBUG_PRINT_1("number of dummy updates received: %d\n", dummy_updates);
    assert(dummy_updates == 5);

    free_config(&data);

    return RETURN_SUCCESS;
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
    iterator array_iter;
    circular_array_iterator_init(&testing_array, &array_iter);
    while(ITERATOR_HAS_NEXT(&array_iter)){
        char ** element = ITERATOR_NEXT(&array_iter);
        assert(**element == 'q');
    }
    circular_array_iterator_free(&array_iter);

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
    free(config_data);


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
    free(config_data);

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