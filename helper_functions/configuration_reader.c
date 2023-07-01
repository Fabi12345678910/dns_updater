#include "configuration_reader.h"
#include "providers/all_providers.h"

#include <stdlib.h>
#include <string.h>

#define CONFIG_KEY_ENABLE_HTTP "enableHTTPServer"
#define CONFIG_KEY_IPV6_SOURCE "ipv4Source"
#define CONFIG_KEY_IPV4_SOURCE "ipv6Source"
#define CONFIG_KEY_DNS_SOURCES "dnsEntries"


#define CONFIG_KEY_DNS_SOURCE_PROVIDER "provider"
#define CONFIG_KEY_DNS_SOURCE_PROVIDER_DATA "providerData"
#define CONFIG_KEY_DNS_SOURCE_TYPE "type"
#define CONFIG_KEY_DNS_SOURCE_NAME "name"


#define CONFIG_VALUE_TRUE "true"
#define CONFIG_VALUE_FALSE "false"


char* read_file_to_string(char* filename){

}



linked_list * read_dns_sources(const char *current_ptr);

#define compare_read_key(key) compare_string(key_start, key_length, key)
struct updater_data *read_config_from_string(char const* string){
    struct updater_data *updater_data = malloc(sizeof(*updater_data));
    if (updater_data == NULL){
        //TODO print error
        return NULL;
    }

    char const* current_ptr = string;
    while(*current_ptr != '\0'){
        skip_empty_chars(current_ptr);
        //read key

        if(*current_ptr == '\0'){
            //TODO reached regular end of string, return appropriately
            return NULL;
        }

        char const* key_start = current_ptr;
        int key_length = read_alphanumeric_chars(current_ptr);

        //TODO check return value
        expect_char(current_ptr, ':');


        //read value depending on key
        if(compare_read_key(CONFIG_KEY_ENABLE_HTTP)){
            //TODO check return value and perhaps fail
            read_value_to_bool(current_ptr, &updater_data->config.enable_http_server);
        }else if(compare_read_key(CONFIG_KEY_DNS_SOURCES)){
            //this is the most interesting part, all the dns entries are loaded
            updater_data->managed_dns_list = read_dns_sources(current_ptr);
        }else{
            //TODO invalid key
        }


    }

    return NULL;
}
//read a single dns source, called by read_dns_sources
int read_dns_source(struct managed_dns_entry * dns_config, const char *current_ptr){

    skip_empty_chars(current_ptr);
    while(*current_ptr != '}'){
        skip_empty_chars(current_ptr);
        //read key and value pairs just like described earlier

        char const *key_start;
        unsigned key_length = read_simple_value(current_ptr, &key_start);
        expect_char(current_ptr, ':');
        if(compare_read_key(CONFIG_KEY_DNS_SOURCE_NAME)){
            read_value_to_string(current_ptr, dns_config->dns_name, sizeof(dns_config->dns_name));
        }else if (compare_read_key(CONFIG_KEY_DNS_SOURCE_TYPE)){
            read_value_to_string(current_ptr, dns_config->dns_type, sizeof(dns_config->dns_type));
        }else if (compare_read_key(CONFIG_KEY_DNS_SOURCE_PROVIDER)){
            
        }else if (compare_read_key(CONFIG_KEY_DNS_SOURCE_PROVIDER_DATA)){

        }else{
            //invalid key
        }
    }
    
}
#undef compare_read_key

linked_list * read_dns_sources(const char *current_ptr){
    linked_list* list = malloc(sizeof(* list));
    if(list == NULL){
        return NULL;
    }

    skip_empty_chars(current_ptr);
    expect_char(current_ptr, '[');
    while(current_ptr != ']'){
        skip_empty_chars(current_ptr);
        if(*current_ptr == '{'){
            current_ptr++;
            struct managed_dns_entry dns_entry;
            read_dns_source(&dns_entry, current_ptr);
            dns_linked_list_insert(list, dns_entry);
        }
    }
}