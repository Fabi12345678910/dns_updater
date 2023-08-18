//#define DEBUG_LEVEL 2
#include "cloudflare.h"

#include "../configuration_reader_common.h"
#include "../call_exec.h"

//unused currently because im dumb
//#define API_BASE_URL "https://api.cloudflare.com/client/v4"

#define GET_ZONE_ID_URL_TEMPLATE "https://api.cloudflare.com/client/v4/zones?name=%s"
#define GET_DNS_ID_URL_TEMPLATE "https://api.cloudflare.com/client/v4/zones/%s/dns_records?type=%s&name=%s&match=all"
#define UPDATE_DNS_URL_TEMPLATE "https://api.cloudflare.com/client/v4/zones/%s/dns_records/%s"
#define UPDATE_DNS_JSON_TEMPLATE "{\"type\":\"%s\",\"name\":\"%s\",\"content\":\"%s\",\"ttl\":3600,\"proxied\":false}"

#define DUMMY_ZONE_ID_RESPONCE "{\"result\":[{\"id\":\"0123456789abcdef0123456789abcdef\",\"name\":\"test.test\", \
    \"status\":\"active\",\"paused\":false,\"type\":\"full\",\"development_mode\":0,\"name_servers\": \
    [\"jaziel.ns.cloudflare.com\",\"ryleigh.ns.cloudflare.com\"],\"original_name_servers\":[\"nameserver1\",\"nameserver2\" \
    ,\"nameserver3\"],\"original_registrar\":null,\"original_dnshost\":null,\"modified_on\":\"2022-07-06T20:45:24.430806Z\" \
    ,\"created_on\":\"2022-07-01T20:16:48.518942Z\",\"activated_on\":\"2022-07-06T20:45:24.430806Z\",\"meta\":{\"step\":2, \
    \"custom_certificate_quota\":0,\"page_rule_quota\":3,\"phishing_detected\":false,\"multiple_railguns_allowed\":false}, \
    \"owner\":{\"id\":null,\"type\":\"user\",\"email\":null},\"account\":{\"id\":\"ffff\", \
    \"name\":\"Dedacted\"},\"tenant\":{\"id\":null,\"name\":null},\"tenant_unit\":{\"id\":null},\"permissions\": \
    [\"#dns_records:edit\",\"#dns_records:read\",\"#zone:read\"],\"plan\":{\"id\":\"0feeeeeeeeeeeeeeeeeeeeeeeeeeeeee\", \
    \"name\":\"Free Website\",\"price\":0,\"currency\":\"USD\",\"frequency\":\"\",\"is_subscribed\":false,\"can_subscribe\" \
    :false,\"legacy_id\":\"free\",\"legacy_discount\":false,\"externally_managed\":false}}],\"result_info\":{\"page\":1,\" \
    per_page\":20,\"total_pages\":1,\"count\":1,\"total_count\":1},\"success\":true,\"errors\":[],\"messages\":[]}"

#define DUMMY_DNS_ID_RESPONCE "{\"result\":[{\"id\":\"fedcba9876543210fedcba9876543210\",\"zone_id\": \
    \"0123456789abcdef0123456789abcdef\",\"zone_name\":\"test.test\",\"name\": \
    \"subdomain.test.test\",\"type\":\"A\",\"content\":\"1.2.3.4\", \
    \"proxiable\":true,\"proxied\":false,\"ttl\":1,\"locked\":false,\"meta\":{\"auto_added\":false \
    ,\"managed_by_apps\":false,\"managed_by_argo_tunnel\":false,\"source\":\"primary\"},\"comment\":null,\"tags\":[], \
    \"created_on\":\"2023-06-08T22:46:14.329416Z\",\"modified_on\":\"2023-06-08T22:46:14.329416Z\"}],\"success\":true, \
    \"errors\":[],\"messages\":[],\"result_info\":{\"page\":1,\"per_page\":100,\"count\":1,\"total_count\":1, \
    \"total_pages\":1}}"

#define DUMMY_UPDATE_DNS_RESPONCE "{\"result\":{\"id\":\"fedcba9876543210fedcba9876543210\",\"zone_id\" \
    :\"0123456789abcdef0123456789abcdef\",\"zone_name\":\"test.test\",\"name\" \
    :\"subdomain.test.test\",\"type\":\"A\",\"content\":\"4.3.2.1\",\"proxiable\":true, \
    \"proxied\":false,\"ttl\":3600,\"locked\":false,\"meta\":{\"auto_added\":false,\"managed_by_apps\":false, \
    \"managed_by_argo_tunnel\":false,\"source\":\"primary\"},\"comment\":null,\"tags\":[],\"created_on\": \
    \"2023-06-08T22:46:14.329416Z\",\"modified_on\":\"2023-08-18T20:25:13.638696Z\"},\"success\":true,\"errors\":[], \
    \"messages\":[]}"

#define AUTHORIZATION_HEADER_TEMPLATE "Authorization: Bearer %s"
static char *get_authorization_header(struct cloudflare_provider_data *cloudflare_data){
    size_t auth_header_size = sizeof(AUTHORIZATION_HEADER_TEMPLATE) + sizeof(cloudflare_data->api_key);
    char *auth_header = malloc(auth_header_size);
    if(auth_header == NULL) return NULL;
    expect_fine(snprintf(auth_header, auth_header_size, AUTHORIZATION_HEADER_TEMPLATE, cloudflare_data->api_key) >= (int) auth_header_size);
    return auth_header;
}

static const char *get_zone_name_from_dns_name(const char * dns_name){
    const char *zone_name_start = &dns_name[strlen(dns_name) - 1];
    if(*zone_name_start == '.'){
        //filter last dot
        zone_name_start--;
    }
    //filter 2 dots

    int found_dots = 0;
    while(zone_name_start > dns_name){
        if(*zone_name_start == '.'){
            found_dots++;
            if(found_dots == 2){
                //went one too far back and reached a subname's dot
                zone_name_start++;
                break;
            }
        }
        zone_name_start--;
    }
    if(found_dots >= 1){
        //yeah, were good, found the zone_name
        DEBUG_PRINT_1("got zone name %s from dns name %s\n", zone_name_start, dns_name);
        return zone_name_start;
    }else{
        //meh, there were not enough dots
        return NULL;
    }
}

#define ID_KEY_BEGIN "result\":[{\"id\":\""
static char *get_zone_id(struct dns_data *data, char* auth_header, char **zone_id){
    //get zone name simply filter on 2 or 3 (depending on ending) dots
    if(strlen(data->dns_name) < 1) malloc_and_return_error_msg("no dns name provided");
    const char *zone_name = get_zone_name_from_dns_name(data->dns_name);
    if(zone_name == NULL) malloc_and_return_error_msg("error extracting zoneName from dnsName");

    size_t update_url_size = sizeof(GET_ZONE_ID_URL_TEMPLATE) + strlen(zone_name);
    char update_url[update_url_size];
    expect_fine(snprintf(update_url, update_url_size, GET_ZONE_ID_URL_TEMPLATE, zone_name) >= (int) update_url_size);

    DEBUG_PRINT_1("calling curl with GET on %s\n", update_url);
    char *curl_result = call_exec("curl", (char *const []) {"curl", "-s", "-X", "GET", update_url, "-H", auth_header, "-H", "Content-Type: application/json", NULL});

    #ifdef TEST_CLOUDFLARE
        free_or_null(curl_result);
        curl_result = malloc(sizeof(DUMMY_ZONE_ID_RESPONCE));
        strcpy(curl_result, DUMMY_ZONE_ID_RESPONCE);
        DEBUG_PRINT_1("replaced curl result with dummy: %s\n", curl_result);
    #endif
    
    if(curl_result == NULL) malloc_and_return_error_msg("error executing curl");
    char *id_begin = strstr(curl_result, ID_KEY_BEGIN);
    if(id_begin == NULL) {
        free(curl_result);
        malloc_and_return_error_msg("no zone id found");
    }
    id_begin += (sizeof(ID_KEY_BEGIN) - 1);
    DEBUG_PRINT_2("begin of zone id: %s\n", id_begin);
    size_t id_size = 0;
    while(id_begin[id_size] != '\"'){
        if(id_begin[id_size] == '\0'){
            free(curl_result);
            malloc_and_return_error_msg("error reading zone id, string ends unexpectedly");
        }
        id_size++;
    }
    DEBUG_PRINT_2("id_size: %zu\n", id_size);
    *zone_id = malloc(id_size + 1);
    expect_fine(*zone_id == NULL);
    memcpy(*zone_id, id_begin, id_size);
    (*zone_id)[id_size] = '\0';
    DEBUG_PRINT_1("got zone id %s\n", *zone_id);
    
    free(curl_result);
    return NULL;
}

static char *get_dns_id(struct dns_data *data, char* auth_header, char *zone_id, char **dns_id){
    if(strlen(data->dns_type) < 1) malloc_and_return_error_msg("no dns name provided");
    size_t update_url_size = sizeof(GET_DNS_ID_URL_TEMPLATE) + strlen(zone_id) + strlen(data->dns_type) + strlen(data->dns_name);
    char update_url[update_url_size];
    expect_fine(snprintf(update_url, update_url_size, GET_DNS_ID_URL_TEMPLATE, zone_id, data->dns_type, data->dns_name) >= (int) update_url_size);

    DEBUG_PRINT_1("calling curl with GET on %s\n", update_url);
    char *curl_result = call_exec("curl", (char *const []) {"curl", "-s", "-X", "GET", update_url,
                                                                    "-H", auth_header,
                                                                    "-H", "Content-Type: application/json", NULL});

    #ifdef TEST_CLOUDFLARE
        free_or_null(curl_result);
        curl_result = malloc(sizeof(DUMMY_DNS_ID_RESPONCE));
        strcpy(curl_result, DUMMY_DNS_ID_RESPONCE);
        DEBUG_PRINT_1("replaced curl result with dummy: %s\n", curl_result);
    #endif

    if(curl_result == NULL) malloc_and_return_error_msg("error executing curl");
    char *id_begin = strstr(curl_result, ID_KEY_BEGIN);
    if(id_begin == NULL) {
        free(curl_result);
        malloc_and_return_error_msg("no dns id found");
    }
    id_begin += (sizeof(ID_KEY_BEGIN) - 1);
    DEBUG_PRINT_2("begin of dns id: %s\n", id_begin);
    size_t id_size = 0;
    while(id_begin[id_size] != '\"'){
        if(id_begin[id_size] == '\0'){
            free(curl_result);
            malloc_and_return_error_msg("error reading dns id, string ends unexpectedly");
        }
        id_size++;
    }
    DEBUG_PRINT_2("id_size: %zu\n", id_size);
    *dns_id = malloc(id_size + 1);
    expect_fine(*dns_id == NULL);
    memcpy(*dns_id, id_begin, id_size);
    (*dns_id)[id_size] = '\0';
    DEBUG_PRINT_1("got dns id %s\n", *dns_id);

    free(curl_result);
    return NULL;
}

static char *update_dns_entry(struct dns_data *data, char* auth_header, char *zone_id, char *dns_id, char *new_rdata){
    size_t update_url_size = sizeof(UPDATE_DNS_URL_TEMPLATE) + strlen(zone_id) + strlen(dns_id);
    char update_url[update_url_size];
    expect_fine(snprintf(update_url, update_url_size, UPDATE_DNS_URL_TEMPLATE, zone_id, dns_id) >= (int) update_url_size);

    size_t update_json_size = sizeof(UPDATE_DNS_JSON_TEMPLATE) + strlen(data->dns_type) + strlen(data->dns_name) + strlen(new_rdata);
    char update_json[update_json_size];
    expect_fine(snprintf(update_json, update_json_size, UPDATE_DNS_JSON_TEMPLATE, data->dns_type, data->dns_name, new_rdata) >= (int) update_json_size);

    DEBUG_PRINT_1("calling curl with PUT on %s with body %s\n", update_url, update_json);
    char *curl_result = call_exec("curl", (char *const []) {"curl", "-s", "-X", "PUT", update_url, 
                                                                    "-H", auth_header, 
                                                                    "-H", "Content-Type: application/json",
                                                                    "--data", update_json, NULL});

    #ifdef TEST_CLOUDFLARE
        free_or_null(curl_result);
        curl_result = malloc(sizeof(DUMMY_UPDATE_DNS_RESPONCE));
        strcpy(curl_result, DUMMY_UPDATE_DNS_RESPONCE);
        DEBUG_PRINT_1("replaced curl result with dummy: %s\n", curl_result);
    #endif
    if(strstr(curl_result, "\"success\":true") != NULL){
        free(curl_result);
        return NULL;
    }else{
        free(curl_result);
        malloc_and_return_error_msg("cloudflare error calling api endpoint for dns update");
    }
}

char *cloudflare_update_dns(struct dns_data *data, char *new_rdata){
    if(data->provider_data == NULL) malloc_and_return_error_msg("no api key provided");
    char *auth_header = get_authorization_header((struct cloudflare_provider_data*) data->provider_data);
    if(auth_header == NULL) malloc_and_return_error_msg("error building authorization header");

    char *zone_id, *dns_id;
    char *error_msg = get_zone_id(data, auth_header, &zone_id);
    if(error_msg != NULL){
        free(auth_header);
        return error_msg;
    };

    error_msg = get_dns_id(data, auth_header, zone_id, &dns_id);
    if(error_msg != NULL){
        free(zone_id);
        free(auth_header);
        return error_msg;
    };


    error_msg = update_dns_entry(data, auth_header, zone_id, dns_id, new_rdata);
    if(error_msg != NULL){
        free(dns_id);
        free(zone_id);
        free(auth_header);
        return error_msg;
    };

    free(dns_id);
    free(zone_id);
    free(auth_header);
    return NULL;
}

static void cloudflare_get_ip6(struct dns_data *data){
    char *lookup_result = call_exec("dig", (char *const []) {"dig", "+short", "+unknownformat", "-t", data->dns_type, data->dns_name, "@1.1.1.1", NULL});
    DEBUG_PRINT_1("result from dig command: %s\n", lookup_result);
    if(lookup_result == NULL){
        DEBUG_PRINT_1("error executing dig\n");
        free_or_null(data->current_data);
        return;
    }
    struct in6_addr returned_ipv6_addr = IN6ADDR_ANY_INIT;
    #define ip6 returned_ipv6_addr.__in6_u.__u6_addr8

    int scan_result = sscanf(lookup_result, "\\# 16 %2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",
    ip6+0, ip6+1, ip6+2, ip6+3,
    ip6+4, ip6+5, ip6+6, ip6+7,
    ip6+8, ip6+9, ip6+10, ip6+11,
    ip6+12, ip6+13, ip6+14, ip6+15
    );
    free(lookup_result);
    if(scan_result != 16){
    }else{
        if(data->current_data != NULL) free(data->current_data);
        data->current_data = malloc(sizeof(returned_ipv6_addr));
        if(data->current_data == NULL) return;
        *((struct in6_addr*) data->current_data) = returned_ipv6_addr;
        #if (DEBUG_LEVEL >= 1)
            char ip6_string[40];
            write_ip6_to_string(*((struct in6_addr*) data->current_data), ip6_string);
            DEBUG_PRINT_1("retrieved dns state IPv6: %s\n", ip6_string);
        #endif
    }
}

static void cloudflare_get_ip4(struct dns_data *data){
    char *lookup_result = call_exec("dig", (char *const []) {"dig", "+short", "-t", data->dns_type, data->dns_name, "@1.1.1.1", NULL});
    DEBUG_PRINT_1("result from dig command: %s\n", lookup_result);
    if(lookup_result == NULL){
        DEBUG_PRINT_1("error executing dig\n");
        free_or_null(data->current_data);
        return;
    }
    struct in_addr returned_ipv4_addr = {.s_addr = INADDR_ANY};
    uint8_t *adress_bytes = (uint8_t *) &returned_ipv4_addr.s_addr;
    int scan_result = sscanf(lookup_result, "%hhu.%hhu.%hhu.%hhu", adress_bytes+3, adress_bytes+2, adress_bytes+1, adress_bytes+0);
    free(lookup_result);
    if(scan_result != 4){
        free_or_null(data->current_data);
        DEBUG_PRINT_1("error converting dig result to IPv4 address, converted_bytes: %d\n", scan_result);
    }else{
        if(data->current_data != NULL) free(data->current_data);
        data->current_data = malloc(sizeof(returned_ipv4_addr));
        if(data->current_data == NULL) return;
        *((struct in_addr*) data->current_data) = returned_ipv4_addr;
        DEBUG_PRINT_1("retrieved dns state Ipv4: %hhu.%hhu.%hhu.%hhu\n", adress_bytes[3], adress_bytes[2], adress_bytes[1], adress_bytes[0]);
    }
}

void cloudflare_get_dns_state(struct dns_data *data){
    if(!strcmp(data->dns_type, "AAAA")){
        cloudflare_get_ip6(data);
    }else if(!strcmp(data->dns_type, "A")){
        cloudflare_get_ip4(data);
    }else{
        //just call dig and let it put whatever it reads, if it succeeds
        data->current_data = call_exec("dig", (char *const []) {"dig", "+short", "-t", data->dns_type, data->dns_name, "@1.1.1.1"});
    }
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