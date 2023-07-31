#include "cloudflare.h"

#include "../configuration_reader_common.h"

char * cloudflare_update_dns (struct dns_data* data, char * new_rdata){
    (void) data;
    (void) new_rdata;
    return NULL;
}

void cloudflare_get_dns_state(struct dns_data* data){
    (void) data;
}

void *cloudflare_read_provider_data(char const **ptr_to_current_ptr){
    struct cloudflare_provider_data *cloudflare_data = malloc(sizeof(*cloudflare_data));
    if(cloudflare_data == NULL){
        fprintf(stderr, "error getting space for cloudflare provider data\n");
        return NULL;
    }
    if(read_value_to_string(ptr_to_current_ptr, cloudflare_data->api_key, sizeof(cloudflare_data->api_key))){
        fprintf(stderr, "error reading cloudflare data\n");
        return NULL;
    }
    return cloudflare_data;
}