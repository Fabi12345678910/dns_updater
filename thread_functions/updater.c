// thread_functions:
#include "updater.h"

#include "../helper_functions/common.h"
#include <string.h>

#ifndef LOG_UNCHANGED
    #define LOG_UNCHANGED 1
#endif

static void ipv4Update(struct updater_data * data, struct managed_dns_entry *dns_entry, struct in_addr *new_ip4addr){
    (void) data;

    struct in_addr * current_ip4addr = (struct in_addr *) dns_entry->dns_data.current_data;
    if(new_ip4addr == NULL){
        LOG_PRINTF(data, 300, "ipv4Update ERROR for %s: no ipv4 provided", dns_entry->dns_data.dns_name);
        dns_entry->dns_data.entry_state = STATE_ERROR;
        return;
    }
    if(current_ip4addr != NULL && new_ip4addr->s_addr == current_ip4addr->s_addr){
        if(LOG_UNCHANGED){
            LOG_PRINTF(data, 300, "ipv4Update OKAY for %s: no ip change detected", dns_entry->dns_data.dns_name);
        }
        dns_entry->dns_data.entry_state = STATE_OKAY;
        return;
    }

    char ipv4_string[4 * 3 + 3 + 1]; //build ipv4string 4 octetts plus 3 dots plus \0
    uint8_t * adress_bytes = (uint8_t *) new_ip4addr;

    if (snprintf(ipv4_string, sizeof(ipv4_string), "%hhu.%hhu.%hhu.%hhu", 
                    adress_bytes[3], adress_bytes[2], adress_bytes[1], adress_bytes[0])
                            >= (int) sizeof(ipv4_string)){
        LOG_PRINTF(data, 320, "ipv4Update ERROR for %s: new ip adress longer for 16 chars", dns_entry->dns_data.dns_name)
        dns_entry->dns_data.entry_state = STATE_ERROR;
        return;
    }

    char* error_msg = dns_entry->provider.update_dns(&dns_entry->dns_data, ipv4_string);

    if(error_msg == NULL){
        LOG_PRINTF(data, 330, "ipv4Update SUCCESS for %s: dns entry updated to %s", dns_entry->dns_data.dns_name, ipv4_string);
        dns_entry->dns_data.entry_state = STATE_OKAY;
        if(current_ip4addr == NULL){

            dns_entry->dns_data.current_data = malloc(sizeof(*current_ip4addr));
            errorIf(dns_entry->dns_data.current_data == NULL, "error getting space for current ip4addr\n");
            current_ip4addr = dns_entry->dns_data.current_data;
        }
        *current_ip4addr = *new_ip4addr;
    }else{
        LOG_PRINTF(data, 400, "ipv4Update provider ERROR for %s: %s", dns_entry->dns_data.dns_name, error_msg);
        free(error_msg);
        dns_entry->dns_data.entry_state = STATE_ERROR;
    }
}

static int ipv6_adress_equals(struct in6_addr *addr1, struct in6_addr *addr2){
    return (addr1->__in6_u.__u6_addr32[0] == addr2->__in6_u.__u6_addr32[0] &&
            addr1->__in6_u.__u6_addr32[1] == addr2->__in6_u.__u6_addr32[1] &&
            addr1->__in6_u.__u6_addr32[2] == addr2->__in6_u.__u6_addr32[2] &&
            addr1->__in6_u.__u6_addr32[3] == addr2->__in6_u.__u6_addr32[3]);
}

static void ipv6Update(struct updater_data * data, struct managed_dns_entry *dns_entry, struct in6_addr *new_ip6addr){
    (void) data;
    
    struct in6_addr * current_ip6addr = (struct in6_addr *) dns_entry->dns_data.current_data;
    if(new_ip6addr == NULL){
        LOG_PRINTF(data, 300, "ipv6Update ERROR for %s: no ipv6 provided", dns_entry->dns_data.dns_name);
        dns_entry->dns_data.entry_state = STATE_ERROR;
        return;
    }
    if(current_ip6addr != NULL && ipv6_adress_equals(current_ip6addr, new_ip6addr)){
        if(LOG_UNCHANGED){
            LOG_PRINTF(data, 300, "ipv6Update OKAY for %s: no ip change detected", dns_entry->dns_data.dns_name);
        }

        dns_entry->dns_data.entry_state = STATE_OKAY;
        return;
    }

    char ipv6_string[8 * 4 + 7 + 1]; //build ipv6string 8 * 4 hexadecimal numbers plus 7 ':' plus \0
    write_ip6_to_string(*new_ip6addr, ipv6_string);
    DEBUG_PRINT_1("new ipv6 string: %s\n", ipv6_string);

    char* error_msg = dns_entry->provider.update_dns(&dns_entry->dns_data, ipv6_string);

    if(error_msg == NULL){
        LOG_PRINTF(data, 330, "ipv6Update SUCCESS for %s: dns entry updated to %s", dns_entry->dns_data.dns_name, ipv6_string);
        dns_entry->dns_data.entry_state = STATE_OKAY;
        if(current_ip6addr == NULL){
            dns_entry->dns_data.current_data = malloc(sizeof(*current_ip6addr));
            errorIf(dns_entry->dns_data.current_data == NULL, "error getting space for current ip6addr\n");
            current_ip6addr = dns_entry->dns_data.current_data;
        }
        *current_ip6addr = *new_ip6addr;
    }else{
        LOG_PRINTF(data, 400, "ipv6Update provider ERROR for %s: %s", dns_entry->dns_data.dns_name, error_msg);
        free(error_msg);
        dns_entry->dns_data.entry_state = STATE_ERROR;
    }
}

void * updater_func(void * arg){

    struct updater_data * data = arg;
    while(!data->ipc_data.shutdown_requested){
        expect_fine(pthread_mutex_lock(&data->ipc_data.mutex_update_shutdown_requested));
        while(!data->ipc_data.update_requested && !data->ipc_data.shutdown_requested){
            DEBUG_PRINT_1("updater: no update requested, awaiting condition variable\n");
            expect_fine(pthread_cond_wait(&data->ipc_data.cond_update_shutdown_requested, &data->ipc_data.mutex_update_shutdown_requested));
            DEBUG_PRINT_1("updater: woken up\n");
        }
        if(data->ipc_data.shutdown_requested){
            expect_fine(pthread_mutex_unlock(&data->ipc_data.mutex_update_shutdown_requested));
            DEBUG_PRINT_1("updater: shutdown requested, exiting\n");
            break;
        }

        DEBUG_PRINT_1("updater: update requested, performing update steps\n");
        data->ipc_data.update_requested = 0;
        expect_fine(pthread_mutex_unlock(&data->ipc_data.mutex_update_shutdown_requested));

        //get current ipv4 and ipv6
        struct in_addr *ip4addr = NULL;
        struct in6_addr *ip6addr = NULL;
        if(data->ipc_data.info.ip4_state != STATE_UNUSED){
            ip4addr = data->config.get_ipv4_address();
            if(ip4addr == NULL){
                LOG_PRINTF(data, 40, "updater: no ipv4 address available");
                data->ipc_data.info.ip4_state = STATE_ERROR;
            }else{
                data->ipc_data.info.ip4_state = STATE_OKAY;
            }
        }

        if(data->ipc_data.info.ip6_state != STATE_UNUSED){
            ip6addr = data->config.get_ipv6_address();
            if(ip6addr == NULL){
                LOG_PRINTF(data, 40, "updater: no ipv6 address available");
                data->ipc_data.info.ip6_state = STATE_ERROR;
            }else{
                data->ipc_data.info.ip6_state = STATE_OKAY;
            }
        }

        expect_fine(pthread_mutex_lock(&data->ipc_data.mutex_dns_list));

        struct iterator_struc dns_entries_iterator;
        linked_list_iterator_init(data->managed_dns_list, &dns_entries_iterator);

        struct managed_dns_entry *dns_entry;
        while(ITERATOR_HAS_NEXT(&dns_entries_iterator)){
            dns_entry = DNS_ITERATOR_NEXT(&dns_entries_iterator);
            if(!strcmp(dns_entry->dns_data.dns_type, "A")){
                if(data->ipc_data.info.ip4_state != STATE_UNUSED){
                    ipv4Update(data, dns_entry, ip4addr);
                }
            }else if (!strcmp(dns_entry->dns_data.dns_type, "AAAA")){
                if(data->ipc_data.info.ip6_state != STATE_UNUSED){
                    ipv6Update(data, dns_entry, ip6addr);
                }
            }
        }
        expect_fine(pthread_mutex_unlock(&data->ipc_data.mutex_dns_list));
        if(ip4addr != NULL) free(ip4addr);
        if(ip6addr != NULL) free(ip6addr);

    }
    DEBUG_PRINT_0("updater: returning\n");
    return NULL;
}