// thread_functions:
#include "http_server.h"

#include "../helper_functions/common.h"

void * http_server_func(void * arg){
    struct updater_data * data = arg;
    (void) data;
    return NULL;
}
