#ifndef CLOUDFLARE_H
#define CLOUDFLARE_H

    #include "../common.h"
    #define CONFIG_VALUE_PROVIDER_CLOUDFLARE "cloudflare"

    struct cloudflare_provider_data{
        char api_key[41];
    };

    void cloudflare_update_dns(struct dns_data*);

    void cloudflare_get_dns_state(struct dns_data*);

    void *cloudflare_read_provider_data(char const **current_ptr);

#endif