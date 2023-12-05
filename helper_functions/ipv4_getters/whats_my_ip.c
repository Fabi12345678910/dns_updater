#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#include "whats_my_ip.h"

#include "../common.h"
#include "../call_exec.h"

int read_formatted(const char * str, const char * fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  int ret = vsscanf(str, fmt, args);
  va_end(args);

  return ret;
}

struct in_addr* get_ipv4_address_whats_my_ip(){
    char * responce = call_exec("curl", (char *const []) {"curl", "-s", "-4", "https://cloudflare.com/cdn-cgi/trace", NULL});

    //TODO what if curl fails?

    struct in_addr* adress = malloc(sizeof(adress));
    errorIf(adress == NULL, "error reserving adress space\n");
    uint8_t *adress_bytes = (uint8_t *) &adress->s_addr;

    char * ip_location = strstr(responce, "\nip=");
    if(ip_location == NULL){
        fprintf(stderr, "no ip= found\n");
        free(responce);
        return NULL;
    }
    ip_location += sizeof("\nip=") - 1;

    int scanf_matches = read_formatted(ip_location, "%hhu.%hhu.%hhu.%hhu", adress_bytes+3, adress_bytes+2, adress_bytes+1, adress_bytes+0);
    if (scanf_matches != 4){
        fprintf(stderr, "unexpected whoami result\n");
        free(responce);
        return NULL;
    }

    DEBUG_PRINT_1("found ip: %hhu.%hhu.%hhu.%hhu\n", adress_bytes[3], adress_bytes[2], adress_bytes[1], adress_bytes[0]);
    DEBUG_PRINT_1("storing: %u\n", adress->s_addr);

    free(responce);
    return adress;
}