#include <stdint.h>

struct provider_functions{
    //TODO adjust data types
    void (*update_dns)(struct managed_dns_entry*);
    void (*get_dns_state)(struct managed_dns_entry*);
    void *(*read_provider_data)(char const *current_ptr);
};

struct managed_dns_entry{
    struct provider_functions provider;
    char dns_name[256];
    char dns_type[5];
    char dns_class[3];
    int32_t ttl;
    char *rdata;

    //holds data for the provider, e.g. authentication data
    void *provider_data;
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
