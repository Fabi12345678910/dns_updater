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
#define CONFIG_KEY_DNS_SOURCE_TYPE "type"
#define CONFIG_KEY_DNS_SOURCE_NAME "name"


#define CONFIG_VALUE_TRUE "true"
#define CONFIG_VALUE_FALSE "false"


/*char* read_file_to_string(char* filename){

}*/

linked_list * read_dns_sources(const char ** ptr_to_current_ptr);

struct updater_data *read_config_from_string(char const* string){
    struct updater_data *updater_data = malloc(sizeof(*updater_data));
    if (updater_data == NULL){
        //TODO print error
        return NULL;
    }

    char const* current_ptr = string;
    while(*current_ptr != '\0'){
        key_data key = read_key(&current_ptr);

        //read value depending on key
        if(key_matches(key, CONFIG_KEY_ENABLE_HTTP)){
            //TODO check return value and perhaps fail
            read_value_to_bool(&current_ptr, &updater_data->config.enable_http_server);
        }else if(key_matches(key, CONFIG_KEY_DNS_SOURCES)){
            //this is the most interesting part, all the dns entries are loaded
            updater_data->managed_dns_list = read_dns_sources(&current_ptr);
        }else{
            //TODO invalid key
        }


    }

    return NULL;
}
//read a single dns source, called by read_dns_sources
int read_dns_source(struct managed_dns_entry * dns_config, const char ** ptr_to_current_ptr){
    const char *current_ptr = *ptr_to_current_ptr;

    skip_empty_chars(ptr_to_current_ptr);
    while(*current_ptr != '}'){
        skip_empty_chars(ptr_to_current_ptr);
        //read key and value pairs just like described earlier

        key_data key = read_key(ptr_to_current_ptr);

        if(key_matches(key, CONFIG_KEY_DNS_SOURCE_NAME)){
            read_value_to_string(ptr_to_current_ptr, dns_config->dns_data.dns_name, sizeof(dns_config->dns_data.dns_name));
        }else if(key_matches(key, CONFIG_KEY_DNS_SOURCE_TYPE)){
            read_value_to_string(ptr_to_current_ptr, dns_config->dns_data.dns_type, sizeof(dns_config->dns_data.dns_type));
        }else if(key_matches(key, CONFIG_KEY_DNS_SOURCE_PROVIDER)){
            
        }else if (key_matches(key, CONFIG_KEY_DNS_SOURCE_PROVIDER_DATA)){

        }else{
            //TODO invalid key
        }

        skip_empty_chars(ptr_to_current_ptr);
    }
    return RETURN_SUCCESS;
}

linked_list * read_dns_sources(const char ** ptr_to_current_ptr){
    linked_list* list = malloc(sizeof(* list));
    if(list == NULL){
        return NULL;
    }

    linked_list_init(list);

    skip_empty_chars(ptr_to_current_ptr);
    expect_char(ptr_to_current_ptr, '[');
    while(**ptr_to_current_ptr != ']'){

        skip_empty_chars(ptr_to_current_ptr);
        
        if(**ptr_to_current_ptr == '{'){
            (*ptr_to_current_ptr)++;
            struct managed_dns_entry dns_entry;
            read_dns_source(&dns_entry, ptr_to_current_ptr);
            dns_linked_list_insert(list, dns_entry);
        }
    }
    return list;
}