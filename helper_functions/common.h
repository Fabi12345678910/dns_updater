#ifndef COMMON_H
    #include <stdint.h>
    #define COMMON_H

    struct dns_data{
        char dns_name[256];
        char dns_type[5];
        char dns_class[3];
        int32_t ttl;
        char *rdata;

        //holds data for the provider, e.g. authentication data
        void *provider_data;
    };

    struct provider_functions{
        void (*update_dns)(struct dns_data*);
        void (*get_dns_state)(struct dns_data*);
        void *(*read_provider_data)(char const *current_ptr);
    };

    struct managed_dns_entry{
        struct dns_data dns_data;
        struct provider_functions provider;
    };


    #include "linked_list.h"
    #define LINKED_LIST_TYPE struct managed_dns_entry
    #define LINKED_LIST_PREFIX dns
    #define ITERATOR_PREFIX DNS
    #include "linked_list_comfort.h"

    struct global_configuration{
        int enable_http_server;
    };

    struct updater_data{
        struct global_configuration config;
        linked_list *managed_dns_list;
    };

#endif
