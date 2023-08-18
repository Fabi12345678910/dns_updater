#include "configuration_reader.h"
#include "providers/all_providers.h"
#include "configuration_reader_common.h"
#include "ipv4_getters/ipv4_getters.h"
#include "ipv6_getters/ipv6_getters.h"

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define CONFIG_KEY_ENABLE_HTTP "enableHTTPServer"
#define CONFIG_KEY_ENABLE_IPV4 "enableIPv4"
#define CONFIG_KEY_ENABLE_IPV6 "enableIPv6"
#define CONFIG_KEY_IPV6_SOURCE "ipv4Source"
#define CONFIG_KEY_IPV4_SOURCE "ipv6Source"
#define CONFIG_KEY_DNS_SOURCES "dnsEntries"


#define CONFIG_KEY_DNS_SOURCE_PROVIDER "provider"
#define CONFIG_KEY_DNS_SOURCE_PROVIDER_DATA "providerData"
// examples: A, AAAA, MX, TXT
#define CONFIG_KEY_DNS_SOURCE_TYPE "type"
#define CONFIG_KEY_DNS_SOURCE_NAME "name"


#define CONFIG_VALUE_TRUE "true"
#define CONFIG_VALUE_FALSE "false"


#define MAX_LOG_SIZE 10UL

#define ENABLE_DEBUG_CONFIG_READER 1
#define DEBUG_PRINT_1_CONFIG_READER(...) DEBUG_PRINT_1_CONDITIONAL(ENABLE_DEBUG_CONFIG_READER, __VA_ARGS__);

char *read_file_to_string(char const* pathname){
    int fd = open(pathname, O_RDONLY);
    if(fd == -1){
        fprintf(stderr, "error opening file %s\n", pathname);
        return NULL;
    }
    char* string = read_fd_to_string(fd, 0);
    expect_fine(close(fd));
    return string;
}

linked_list * read_dns_sources(const char ** ptr_to_current_ptr);

void config_free_dns_data(struct managed_dns_entry * entry){
    if (entry->dns_data.provider_data != NULL){
        free(entry->dns_data.provider_data);
    }
    if (entry->dns_data.current_data != NULL){
        free(entry->dns_data.current_data);
    }
}

void free_config(struct updater_data *config){
    dns_linked_list_free_with_function(config->managed_dns_list, config_free_dns_data);
    free(config->managed_dns_list);
    circular_array_free(&(config->ipc_data.info.logging_array));
}

void init_dns_data(struct dns_data *dns_data){
    dns_data->dns_class[0] = '\0';
    dns_data->dns_name[0] = '\0';
    dns_data->dns_type[0] = '\0';
    dns_data->provider_data = NULL;
    dns_data->current_data = NULL;
    dns_data->ttl= 3600;
    dns_data->entry_state = STATE_UNDEFINED;
}

void init_dns_provider(struct provider_functions *provider){
    provider->get_dns_state = NULL;
    provider->read_provider_data = NULL;
    provider->update_dns = NULL;
}

void init_dns_entry(struct managed_dns_entry *dns_entry){
    init_dns_data(&dns_entry->dns_data);
    init_dns_provider(&dns_entry->provider);
}

static void free_deref(void* ptr){
    void ** ptr_to_ptr_to_free = (void**) ptr;
    DEBUG_PRINT_2("freeing %p, stored at %p\n", *ptr_to_ptr_to_free, ptr);
    free(*ptr_to_ptr_to_free);
}

void init_updater_data(struct updater_data* data){
    data->config.enable_http_server = 1;
    data->config.get_ipv4_address = get_ipv4_address_whats_my_ip;
    data->config.get_ipv6_address = get_ipv6_address_local_interface;

    data->ipc_data.update_requested = 0;
    data->ipc_data.shutdown_requested = 0;
    data->ipc_data.cond_update_shutdown_requested = (pthread_cond_t) PTHREAD_COND_INITIALIZER;
    data->ipc_data.mutex_update_shutdown_requested = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    data->ipc_data.mutex_dns_list = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    data->ipc_data.mutex_info = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    data->ipc_data.info.ip4_state = STATE_UNDEFINED;
    data->ipc_data.info.ip6_state = STATE_UNDEFINED;
    circular_array_init(&data->ipc_data.info.logging_array,sizeof(char*), MAX_LOG_SIZE, free_deref);

    data->managed_dns_list = NULL;
}

int read_enabled_into_state(const char ** ptr_to_current_ptr, state* stat){
    int bool_val;
    if(read_value_to_bool(ptr_to_current_ptr, &bool_val)) return RETURN_ERROR;
    if(bool_val){
        *stat = STATE_UNDEFINED;
    }else{
        *stat = STATE_UNUSED;
    }
    return RETURN_SUCCESS;
}

struct updater_data *read_config_from_string(char const* string){
    struct updater_data *updater_data = malloc(sizeof(*updater_data));
    if (updater_data == NULL){
        error("not enough space\n");
    }

    init_updater_data(updater_data);

    //init linked list
//    updater_data->managed_dns_list = NULL;
    char const* current_ptr = string;

    skip_empty_chars(&current_ptr);
    while(*current_ptr != '\0'){
        DEBUG_PRINT_1_CONFIG_READER("reading a key\n");
        DEBUG_PRINT_1_CONFIG_READER("current_ptr: %s\n", current_ptr);

        key_data key = read_key(&current_ptr);
        //read value depending on key
        if(key_matches(key, CONFIG_KEY_ENABLE_HTTP)){
            errorIf(read_value_to_bool(&current_ptr, &updater_data->config.enable_http_server), "error reading http_enable bool\n");
        }else if(key_matches(key, CONFIG_KEY_ENABLE_IPV4)){
            errorIf(read_enabled_into_state(&current_ptr, &updater_data->ipc_data.info.ip4_state), "error reading enable ipv4 value\n")
        }else if(key_matches(key, CONFIG_KEY_ENABLE_IPV6)){
            errorIf(read_enabled_into_state(&current_ptr, &updater_data->ipc_data.info.ip6_state), "error reading enable ipv6 value\n")
        }else if(key_matches(key, CONFIG_KEY_DNS_SOURCES)){
            //this is the most interesting part, all the dns entries are loaded
            updater_data->managed_dns_list = read_dns_sources(&current_ptr);
        }else{
            error("invalid base key found\n");
        }
        skip_empty_chars(&current_ptr);
    }

    //create the list if it has not been initialized yet
    if (updater_data->managed_dns_list == NULL){
        updater_data->managed_dns_list = malloc (sizeof *updater_data->managed_dns_list);
        if (updater_data->managed_dns_list == NULL){
            error("not enough space\n");
        }
        linked_list_init(updater_data->managed_dns_list);
    }

    return updater_data;
}
//read a single dns source, called by read_dns_sources
int read_dns_source(struct managed_dns_entry * dns_config, const char ** ptr_to_current_ptr){
//    dns_config->dns_data.provider_data = NULL;

    errorIf(expect_char(ptr_to_current_ptr, '{'), "expected {");
    while(**ptr_to_current_ptr != '}'){
        //read key and value pairs just like described earlier

        key_data key = read_key(ptr_to_current_ptr);

        if(key_matches(key, CONFIG_KEY_DNS_SOURCE_NAME)){
            read_value_to_string(ptr_to_current_ptr, dns_config->dns_data.dns_name, sizeof(dns_config->dns_data.dns_name));
        }else if(key_matches(key, CONFIG_KEY_DNS_SOURCE_TYPE)){
            read_value_to_string(ptr_to_current_ptr, dns_config->dns_data.dns_type, sizeof(dns_config->dns_data.dns_type));
        }else if(key_matches(key, CONFIG_KEY_DNS_SOURCE_PROVIDER)){
            read_provider(ptr_to_current_ptr, &dns_config->provider);
        }else if (key_matches(key, CONFIG_KEY_DNS_SOURCE_PROVIDER_DATA)){
            if(dns_config->provider.read_provider_data == NULL){
                fprintf(stderr, "unset provider, set provider first before setting providerData\n");
                return RETURN_ERROR;
            }
            dns_config->dns_data.provider_data = dns_config->provider.read_provider_data(ptr_to_current_ptr);
        }else{
            DEBUG_PRINT_1_CONFIG_READER("current_ptr: %s\n", *ptr_to_current_ptr);
            fprintf(stderr, "unknown key set in dnsSource\n");
            return RETURN_ERROR;
        }

        skip_empty_chars(ptr_to_current_ptr);
    }
    //skip the '}' previously detected by leaving the loop
    (*ptr_to_current_ptr)++;

    return RETURN_SUCCESS;
}

linked_list * read_dns_sources(const char ** ptr_to_current_ptr){
    linked_list* list = malloc(sizeof(* list));
    if(list == NULL){
        return NULL;
    }

    linked_list_init(list);

    errorIf(expect_char(ptr_to_current_ptr, '['), "expected [\n");
    while(**ptr_to_current_ptr != ']'){

        skip_empty_chars(ptr_to_current_ptr);

        if(**ptr_to_current_ptr == '{'){
            struct managed_dns_entry dns_entry;
            init_dns_entry(&dns_entry);
            errorIf(read_dns_source(&dns_entry, ptr_to_current_ptr),"error reading dns source\n");
            errorIf(dns_linked_list_insert(list, dns_entry) == RETURN_ERROR, "error inserting dns entry\n");
            skip_empty_chars(ptr_to_current_ptr);
        }else if (**ptr_to_current_ptr == ','){
            continue;
        }
    }
    //skip the ']' previously detected by leaving the loop
    (*ptr_to_current_ptr)++;

    return list;
}