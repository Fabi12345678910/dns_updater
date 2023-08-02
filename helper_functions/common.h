#ifndef COMMON_H
    #define COMMON_H
    #include <stdint.h>
    #include <pthread.h>
    #include <netinet/in.h>
    #include <stdarg.h>

    #ifndef _DEFAULT_SOURCE
        #define _DEFAULT_SOURCE
    #endif
    #include <unistd.h>

    #define ENABLE_LOG_STDOUT 1
    #ifndef DEBUG_LEVEL
        #define DEBUG_LEVEL 1
    #endif
    #include "debug.h"
    #include "circular_array.h"

    typedef int state;
    #define STATE_UNDEFINED 0
    #define STATE_OKAY 1
    #define STATE_WARNING 2
    #define STATE_ERROR 3
    #define STATE_UNUSED 4

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
        void *current_data;

        //holds data for the provider, e.g. authentication data
        void *provider_data;

        //the state of the dns entry
        state entry_state;
    };

    struct provider_functions{
        char * (*update_dns)(struct dns_data*, char * new_rdata);
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

    struct state_information{
        state ip4_state;
        state ip6_state;
        circular_array logging_array;
    };

    struct process_communication{
        pthread_mutex_t mutex_dns_list;
        int shutdown_requested;
        int update_requested;
        pthread_mutex_t mutex_update_shutdown_requested;
        pthread_cond_t cond_update_shutdown_requested;
        struct state_information info;
        pthread_mutex_t mutex_info;
    };


    struct updater_data{
        struct global_configuration config;
        linked_list *managed_dns_list;
        struct process_communication ipc_data;
    };


    #if (ENABLE_LOG_STDOUT)
        #define LOG_STDOUT(format, ...) fprintf(stdout, format, __VA_ARGS__);fprintf(stdout, "\n")
    #else
        #define LOG_STDOUT
    #endif

    #define LOG_RAW(data, message) circular_array_append(&((data)->ipc_data.info.logging_array), &(message))
    #define LOG_PRINTF(data, max_size, ...) do                          \
    {                                                                           \
        char *msg_ptr = malloc(max_size);                                       \
        if(snprintf(msg_ptr, max_size, __VA_ARGS__) >= (int)max_size){  \
            msg_ptr[max_size-1] = '\0';                                         \
        }                                                                       \
        LOG_STDOUT("%s", msg_ptr);                                              \
                                                                                \
        LOG_RAW(data, msg_ptr);                                                 \
    } while (0);

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

    #define four_bit_to_hex(value) ((value) <= 9 ? '0' + (value) : 'a' + ((value) - 10))

    ///@param ip6addr, must be struct in6_addr
    ///@param string, a character array of at least 40 length
    #define write_ip6_to_string(ip6addr, string) do{    \
        int write_head = 0;                                 \
        for(int i = 0; i<16; i++){                          \
            if(i % 2 == 0 && i != 0){                       \
                string[write_head++] = ':';                 \
            }                                               \
            string[write_head++] = four_bit_to_hex((ip6addr).__in6_u.__u6_addr8[i] >> 4);       \
            string[write_head++] = four_bit_to_hex((ip6addr).__in6_u.__u6_addr8[i] & 0x0F);     \
        }                                                   \
        string[write_head] = '\0';                          \
    } while(0)

#endif
