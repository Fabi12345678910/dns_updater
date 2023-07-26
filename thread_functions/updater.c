// thread_functions:
#include "../helper_functions/common.h"
#include "string.h"

void * updater_func(void * arg){
    struct updater_data * data = arg;
    expect_fine(pthread_mutex_lock(&data->ipc_data.mutex_update_requested));
    while(!data->ipc_data.update_requested){
        DEBUG_PRINT_1("no update requested, awaiting condition variable\n");
        expect_fine(pthread_cond_wait(&data->ipc_data.cond_update_requested, &data->ipc_data.mutex_update_requested));
    }
    DEBUG_PRINT_1("update requested, performing update steps\n");
    data->ipc_data.update_requested = 0;
    expect_fine(pthread_mutex_unlock(&data->ipc_data.mutex_update_requested));

    //get current ipv4 and ipv6



    expect_fine(pthread_mutex_lock(&data->ipc_data.mutex_dns_list));

    struct iterator_struc dns_entries_iterator;
    linked_list_iterator_init(data->managed_dns_list, &dns_entries_iterator);

    struct managed_dns_entry *dns_entry;
    while(ITERATOR_HAS_NEXT(&dns_entries_iterator)){
        dns_entry = DNS_ITERATOR_NEXT(&dns_entries_iterator);
        if(!strcmp(dns_entry->dns_data.dns_type, "A")){
            //do ipv4 update
        }else if (!strcmp(dns_entry->dns_data.dns_type, "AAAA")){
            //do ipv6 update
        }
    }

    expect_fine(pthread_mutex_unlock(&data->ipc_data.mutex_dns_list));

    return NULL;
}