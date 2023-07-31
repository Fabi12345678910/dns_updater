// thread_functions:
#include "updater.h"

#include "../helper_functions/common.h"
#include "string.h"

static void ipv4Update(struct updater_data * data, struct managed_dns_entry *dns_entry, struct in_addr *new_ip4addr){
    (void) data;
    
    struct in_addr * current_ip4addr = (struct in_addr *) dns_entry->dns_data.current_data;
    if(new_ip4addr == NULL){
        //TODO:LOG update failed
        return;
    }
    if(current_ip4addr != NULL && new_ip4addr->s_addr == current_ip4addr->s_addr){
        //TODO:LOG no update necessary
        return;
    }

    char ipv4_string[4 * 3 + 3 + 1]; //build ipv4string 4 octetts plus 3 dots plus \n
    uint8_t * adress_bytes = (uint8_t *) new_ip4addr;

    if (snprintf(ipv4_string, sizeof(ipv4_string), "%hhu.%hhu.%hhu.%hhu\n", 
                    adress_bytes[3], adress_bytes[2], adress_bytes[1], adress_bytes[0])
                            >= (int) sizeof(ipv4_string)){
        //result was truncated, error
        //TODO:LOG invalid new ipv4 adress
        return;
    }

    char* error_msg = dns_entry->provider.update_dns(&dns_entry->dns_data, ipv4_string);

    if(error_msg == NULL){
        //TOOO:LOG update somehow
        *current_ip4addr = *new_ip4addr;
    }else{
        //TODO:LOG error_msg
    }
}

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
    struct in_addr  *ip4addr = data->config.get_ipv4_address();
    struct in6_addr *ip6addr = data->config.get_ipv6_address();

    if(ip4addr == NULL){
        //TODO:LOG no ip4update possible, log somehow
    }

    if(ip6addr == NULL){
        //TODO:LOG no ip6update possible, log somehow
    }

    expect_fine(pthread_mutex_lock(&data->ipc_data.mutex_dns_list));

    struct iterator_struc dns_entries_iterator;
    linked_list_iterator_init(data->managed_dns_list, &dns_entries_iterator);

    struct managed_dns_entry *dns_entry;
    while(ITERATOR_HAS_NEXT(&dns_entries_iterator)){
        dns_entry = DNS_ITERATOR_NEXT(&dns_entries_iterator);
        if(!strcmp(dns_entry->dns_data.dns_type, "A")){
            ipv4Update(data, dns_entry, ip4addr);
            //do ipv4 update
        }else if (!strcmp(dns_entry->dns_data.dns_type, "AAAA")){
            //do ipv6 update
        }
    }

    expect_fine(pthread_mutex_unlock(&data->ipc_data.mutex_dns_list));

    return NULL;
}