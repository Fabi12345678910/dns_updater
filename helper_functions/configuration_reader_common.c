#include "configuration_reader_common.h"


#define current_ptr (*ptr_to_current_ptr)

void skip_empty_chars(char const** ptr_to_current_ptr){
    while(*current_ptr == ' ' || *current_ptr == '\n' || *current_ptr == '\r' || *current_ptr == '\t' || *current_ptr == ','){
        current_ptr++;
    }
}

unsigned read_alphanumeric_chars(char const** ptr_to_current_ptr){
    unsigned length = 0;
    while((*current_ptr >= 'A' && *current_ptr <= 'Z') 
       || (*current_ptr >= 'a' && *current_ptr <= 'z')
       || (*current_ptr >= '0' && *current_ptr <= '9')
       || (*current_ptr == '.')
       || (*current_ptr == '_')
       || (*current_ptr == '-')){
        length++;
        current_ptr++;
    }
    return length;
}

/// @brief
//compare parts of a string to another string
/// @param string_start a string to compare
/// @param string_length the length of the string
/// @param comparing_string the string to compare against, must be null-terminated
/// @return 0 if successfull, otherwise not 0
int compare_string(char const* string_start, size_t string_length, char const* comparing_string){
    if(string_length != strlen(comparing_string)){
        return 0;
    }
    return !memcmp(string_start, comparing_string, string_length);
}

//sets *value_begin to the begin of the value, if value != NULL
//returns length of value
unsigned read_simple_value(char const** ptr_to_current_ptr, char const **value_begin){
    //int length = 0;
    skip_empty_chars(ptr_to_current_ptr);
    if (value_begin != NULL){
        *value_begin = current_ptr;
    }
    return read_alphanumeric_chars(ptr_to_current_ptr);
}

int read_value_to_bool(char const** ptr_to_current_ptr, int *bool_ptr){
    char const *value_begin;
    unsigned length = read_simple_value(ptr_to_current_ptr, &value_begin);
    if (compare_string(value_begin, length, CONFIG_VALUE_TRUE))
    {
        *bool_ptr = 1;
    }else if(compare_string(value_begin, length, CONFIG_VALUE_FALSE)){
        *bool_ptr = 0;
    }else{
        return RETURN_ERROR;
    }
    return RETURN_SUCCESS;    
}

int read_value_to_string(char const ** ptr_to_current_ptr, char *string_ptr, size_t string_size){
    char const *value_begin;
    unsigned length = read_simple_value(ptr_to_current_ptr, &value_begin);

    if(length >= string_size){
        return RETURN_ERROR;
    }
    memcpy(string_ptr, value_begin, length);
    string_ptr[length] = '\0';
    return RETURN_SUCCESS;
}

int read_provider(char const ** ptr_to_current_ptr, struct provider_functions* provider){
    char const *value_begin;
    unsigned length = read_simple_value(ptr_to_current_ptr, &value_begin);
    if(compare_string(value_begin, length, CONFIG_VALUE_PROVIDER_CLOUDFLARE)){
        provider->get_dns_state = cloudflare_get_dns_state;
        provider->read_provider_data = cloudflare_read_provider_data;
        provider->update_dns = cloudflare_update_dns;
    }
    #ifndef DISABLE_DUMMY_PROVIDER
    else if(compare_string(value_begin, length, CONFIG_VALUE_PROVIDER_DUMMY)){
        provider->get_dns_state = dummy_get_dns_state;
        provider->read_provider_data = dummy_read_provider_data;
        provider->update_dns = dummy_update_dns;
    }
    #endif
    //add other providers here
    else{
        return RETURN_ERROR;
    }
    return RETURN_SUCCESS;
}

int expect_char(const char ** ptr_to_current_ptr, const char expected_char){
    skip_empty_chars(ptr_to_current_ptr);

    if(*current_ptr != expected_char){
        return RETURN_ERROR;
    }
    current_ptr++;
    return RETURN_SUCCESS;
}

key_data read_key(char const ** ptr_to_current_ptr){
    key_data key;
    key.length = read_simple_value(ptr_to_current_ptr, &key.start);
    //TODO fail if not expected
    expect_char(ptr_to_current_ptr, ':');
    return key;
}

int key_matches(key_data key, char const *config_key){
    return compare_string(key.start, key.length, config_key);
}

#undef current_ptr