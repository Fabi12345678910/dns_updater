#ifdef DISABLE_DUMMY_PROVIDER
    #define DUMMY_PROVIDER_H
#endif

#ifndef DUMMY_PROVIDER_H
#define DUMMY_PROVIDER_H

    #include "../common.h"
    #define CONFIG_VALUE_PROVIDER_DUMMY "dummy"

    char *dummy_update_dns(struct dns_data* data, char * new_rdata);

    void dummy_get_dns_state(struct dns_data*);

    void *dummy_read_provider_data(char const **current_ptr);

#endif