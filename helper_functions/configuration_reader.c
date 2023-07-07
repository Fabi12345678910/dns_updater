#include "configuration_reader.h"
#include "providers/all_providers.h"
#include "configuration_reader_common.h"

#include <stdlib.h>
#include <string.h>

#define CONFIG_KEY_ENABLE_HTTP "enableHTTPServer"
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

#define ENABLE_DEBUG_CONFIG_READER 1
#define DEBUG_PRINT_1_CONFIG_READER(...) DEBUG_PRINT_1_CONDITIONAL(ENABLE_DEBUG_CONFIG_READER, __VA_ARGS__);

/*char* read_file_to_string(char* filename){

}*/

linked_list * read_dns_sources(const char ** ptr_to_current_ptr);

void free_config(struct updater_data *config){
    linked_list_free(config->managed_dns_list);
    free(config->managed_dns_list);
    free(config);
}

struct updater_data *read_config_from_string(char const* string){
    struct updater_data *updater_data = malloc(sizeof(*updater_data));
    if (updater_data == NULL){
        error("not enough space\n");
    }

    //init linked list
    updater_data->managed_dns_list = NULL;
    char const* current_ptr = string;

    skip_empty_chars(&current_ptr);
    while(*current_ptr != '\0'){
        DEBUG_PRINT_1_CONFIG_READER("reading a key\n");
        DEBUG_PRINT_1_CONFIG_READER("current_ptr: %s\n", current_ptr);

        key_data key = read_key(&current_ptr);
        //read value depending on key
        if(key_matches(key, CONFIG_KEY_ENABLE_HTTP)){
            //TODO check return value and perhaps fail
            read_value_to_bool(&current_ptr, &updater_data->config.enable_http_server);
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
            dns_config->provider.read_provider_data(ptr_to_current_ptr);
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
            errorIf(read_dns_source(&dns_entry, ptr_to_current_ptr),"error reading dns source\n");
            dns_linked_list_insert(list, dns_entry);
            skip_empty_chars(ptr_to_current_ptr);
        }else if (**ptr_to_current_ptr == ','){
            continue;
        }
    }
    //skip the ']' previously detected by leaving the loop
    (*ptr_to_current_ptr)++;

    return list;
}