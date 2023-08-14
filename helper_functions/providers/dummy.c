#include "dummy.h"

#include "string.h"

#define ERROR_MSG "unexpected error occured"

char *dummy_update_dns(struct dns_data* data, char * new_rdata){
    (void) data;
    (void) new_rdata;
    static int error = 1;
    error = !error;
    if(error){
        char* error_msg = malloc(sizeof(ERROR_MSG));
        memcpy(error_msg, ERROR_MSG, sizeof(ERROR_MSG));
        return error_msg;
    }else{
        return NULL;
    }
}

void dummy_get_dns_state(struct dns_data* data){
    (void) data;
}

void *dummy_read_provider_data(char const **current_ptr){
    (void) current_ptr;
    error("nothing to read into dummy data\n");
}