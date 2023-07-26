#ifndef COMMON_H
    #define COMMON_H
    #include <stdint.h>
    #include <pthread.h>
    #include <netinet/in.h>

    #ifndef DEBUG_LEVEL
        #define DEBUG_LEVEL 1
    #endif
    #include "debug.h"

    struct dns_data{
        //the fully clasified dns name (e.g. subdomain.test.de)
        char dns_name[256];

        //e.g. A, AAAA
        char dns_type[5];
        
        //usually "", otherwise "IN", see https://de.wikipedia.org/wiki/Resource_Record
        char dns_class[3];

        //time to live, defaults to 3600s
        int32_t ttl;

        //current state of data
        char *rdata;

        //holds data for the provider, e.g. authentication data
        void *provider_data;
    };

    struct provider_functions{
        void (*update_dns)(struct dns_data*);
        void (*get_dns_state)(struct dns_data*);
        void *(*read_provider_data)(char const **current_ptr);
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
        struct in6_addr* (* get_ipv6_address)();
        struct in_addr* (* get_ipv4_address)();
        int enable_http_server;
    };

    struct process_communication{
        pthread_mutex_t mutex_dns_list;
        int update_requested;
        pthread_mutex_t mutex_update_requested;
        pthread_cond_t cond_update_requested;
    };

    struct updater_data{
        struct global_configuration config;
        linked_list *managed_dns_list;
        struct process_communication ipc_data;
    };


    //internal calls for error handling
    #define error(...) while (1)        \
    {                                   \
        fprintf(stderr, __VA_ARGS__);   \
        exit(EXIT_FAILURE);             \
    }

    ///@param cond condition to fail if true
    ///@param __VA_ARGS__ error message, like printf
    #define errorIf(cond, ...) if (cond) {error(__VA_ARGS__)}

    #define expect_fine(cond) if (cond) {error(#cond)}

#endif
