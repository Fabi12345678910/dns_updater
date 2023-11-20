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

    //define to disable the dummy provider
//    #define DISABLE_DUMMY_PROVIDER

    //define TEST_TIMER to enable short 3 sec timer(used by implementation testing)
//    #define TEST_TIMER
//enables dummy responces
//    #define TEST_CLOUDFLARE

    #define ENABLE_LOG_STDOUT 1
    #ifndef DEBUG_LEVEL
        #define DEBUG_LEVEL 0
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
        volatile int shutdown_requested;
        volatile int update_requested;
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
        #define LOG_STDOUT(format, ...) fprintf(stdout, format, __VA_ARGS__);fprintf(stdout, "\n");fflush(stdout);
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

    //free ptr if not null
    #define free_or_null(ptr) do{if(ptr!=NULL) free(ptr); ptr = NULL;}while(0)

    ///@param cond condition to fail if true
    ///@param __VA_ARGS__ error message, like printf
    #define errorIf(cond, ...) if (cond) {error(__VA_ARGS__)}

    #define expect_fine(cond) if (cond) {fprintf(stderr, #cond); exit(1);}

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


    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-function"
    #define FD_TO_STRING_INITIAL_BUFFER_SIZE 64

    static char * read_fd_to_string(const int fd, const int stop_on_double_crlf){
        size_t buffer_size = FD_TO_STRING_INITIAL_BUFFER_SIZE;
        size_t total_read_bytes = 0;
        ssize_t read_bytes = 0;
        char *buffer = malloc(buffer_size);
        while(1){
            read_bytes = read(fd, buffer+total_read_bytes, buffer_size-total_read_bytes);
            DEBUG_PRINT_1("read %zd bytes\n", read_bytes);
            if(read_bytes < 0){
                DEBUG_PRINT_1("error reading curl output\n");
                return NULL;
            }
            if(read_bytes == 0){
                //reached EOF
                break;
            }
            total_read_bytes += read_bytes;
            DEBUG_PRINT_2("last 4 characters: %hhd %hhd %hhd %hhd\n", buffer[total_read_bytes-4], buffer[total_read_bytes-3], buffer[total_read_bytes-2], buffer[total_read_bytes-1]);
            if(stop_on_double_crlf){
                if (buffer[total_read_bytes-4] == '\r' && buffer[total_read_bytes-3] == '\n' && buffer[total_read_bytes-2] == '\r' && buffer[total_read_bytes-1] == '\n'){
                    break;
                }
            }

            if (total_read_bytes >= buffer_size){
                DEBUG_PRINT_1("increasing buffer size from %zu to %zu\n", buffer_size, buffer_size * 2);
                buffer = realloc(buffer, buffer_size * 2);
                errorIf(buffer == NULL, "curl: error getting buffer space\n");
                buffer_size *= 2;
            }
        }
        buffer[total_read_bytes] = '\0';
        DEBUG_PRINT_2("read from fd: %s\n", buffer);
        return buffer;
    }

    #define FAULTY_STATE(state) ((state) == STATE_ERROR || (state) == STATE_UNDEFINED)

    static int any_error_occured(struct updater_data* data){
        if (FAULTY_STATE(data->ipc_data.info.ip4_state)) return 1;
        if (FAULTY_STATE(data->ipc_data.info.ip6_state)) return 1;
        iterator dns_iter;
        linked_list_iterator_init(data->managed_dns_list, &dns_iter);
        while(ITERATOR_HAS_NEXT(&dns_iter)){
            struct managed_dns_entry* dns_entry = DNS_ITERATOR_NEXT(&dns_iter);
            DEBUG_PRINT_1("checking wether %s is faulty\n", dns_entry->dns_data.dns_name);
            if(FAULTY_STATE(dns_entry->dns_data.entry_state)) return 1;
        }

        return 0;
    }

    #define malloc_and_return_error_msg(msg) do{char *error_msg = malloc(sizeof(msg)); strcpy(error_msg, msg); return error_msg;}while(0)

    #pragma GCC diagnostic pop

#endif
