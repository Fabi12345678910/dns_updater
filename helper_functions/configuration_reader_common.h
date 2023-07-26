#ifndef CONFIGURATION_READER_COMMON_H
    #define CONFIGURATION_READER_COMMON_H
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

    //functions for reading various dataTypes
    int read_value_to_bool(char const ** ptr_to_current_ptr, int *bool_ptr);
    int read_value_to_string(char const ** ptr_to_current_ptr, char *string_ptr, size_t string_size);
    int read_provider(char const ** ptr_to_current_ptr, struct provider_functions* provider);
    int expect_char(const char ** ptr_to_current_ptr, const char expected_char);
    void skip_empty_chars(char const ** ptr_to_current_ptr);

    typedef struct{
        char const *start;
        unsigned length;
    } key_data;

    key_data read_key(char const ** ptr_to_current_ptr);
    int key_matches(key_data key, char const *config_key);

#endif