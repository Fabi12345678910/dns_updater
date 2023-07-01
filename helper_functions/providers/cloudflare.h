#define CONFIG_VALUE_PROVIDER_CLOUDFLARE "cloudflare"

void cloudflare_update_dns (struct managed_dns_entry*);

void cloudflare_get_dns_state(struct managed_dns_entry*);

void *cloudflare_read_provider_data(char const *current_ptr);