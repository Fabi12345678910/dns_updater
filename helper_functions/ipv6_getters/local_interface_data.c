#include "local_interface_data.h"
#include <stdlib.h>
#include <ifaddrs.h>
#include <string.h>       

#include <linux/if_addr.h>

//#define DEBUG_LEVEL 2
#include "../common.h"


#define INTERNET_DEVICE "wlp4s0"

//#include "../../helper_functions/common.h"

struct in6_addr* get_ipv6_address_local_interface(){
    struct in6_addr* ret_addr = malloc(sizeof(*ret_addr));
    if(ret_addr == NULL){
        return NULL;
    }

    struct ifaddrs *ifaddr;
    errorIf(getifaddrs(&ifaddr), "error receiving ipv6 adresses\n");
    DEBUG_PRINT_2("received ifaddr data\n");

    for(struct ifaddrs * addr = ifaddr; addr != NULL; addr = addr->ifa_next){
        if(addr->ifa_addr == NULL) continue;
        DEBUG_PRINT_2("addr_ptr: %p\n", (void*) addr);
        DEBUG_PRINT_2("ifa_addr_ptr: %p\n", (void*) addr->ifa_addr);
        DEBUG_PRINT_2("addr_family: %hu\n", addr->ifa_addr->sa_family);
        if(addr->ifa_addr->sa_family == AF_INET6){
            struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *) addr->ifa_addr;
            
            DEBUG_PRINT_1("found a ipv6 adress\n");

            //debug print ipv6 adress
            for(int i = 0; i<16; i++){
                if(i % 2 == 0 && i != 0){
                    DEBUG_PRINT_1(":");
                }
                DEBUG_PRINT_1("%.2x", addr6->sin6_addr.__in6_u.__u6_addr8[i]);
            }
            DEBUG_PRINT_1("\n");

            char ipv6_string[40];
            write_ip6_to_string(addr6->sin6_addr, ipv6_string);
            DEBUG_PRINT_1("ip6 converted by macro: %s\n", ipv6_string);

            //check for a specific internet device
            if (strcmp(addr->ifa_name, INTERNET_DEVICE)){
                DEBUG_PRINT_1("wrong internet device, ignoring...\n\n")
                continue;
            }

            //ensure the adress does not start with 'f'
            uint8_t f_prefix = 0xf0;
            if((addr6->sin6_addr.__in6_u.__u6_addr8[0] & f_prefix) == f_prefix){
                DEBUG_PRINT_1("found a f* adress, ignoring...\n");
                continue;
            }

            //ensure it is a non-temporary adress
            //this does currently not work, the temp and permanent adress seems identical, one needs to disable privacy extensions
/*            if (addr->ifa_flags & IFA_F_TEMPORARY){
                DEBUG_PRINT_2("address data: %p\n", addr->ifa_data);
                DEBUG_PRINT_2("adress flags: %x\n", addr->ifa_flags);
                DEBUG_PRINT_1("adress is temporary, ignoring...\n")
                continue;
            }*/
            
            DEBUG_PRINT_1("found the exact ipv6 adress we're looking for\n");
            //clean up and return
            *ret_addr = addr6->sin6_addr;
            freeifaddrs(ifaddr);
            return ret_addr;
        }else
        {
            DEBUG_PRINT_2("found something else\n");
        }
        DEBUG_PRINT_2("next addr_ptr: %p\n", (void*) addr->ifa_next);
        
    }

    DEBUG_PRINT_1("end of interface loop, no valid adress found\n");

    freeifaddrs(ifaddr);
    free(ret_addr);

    return NULL;
}