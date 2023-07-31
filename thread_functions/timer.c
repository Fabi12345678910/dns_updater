// thread_functions:
#include "timer.h"

#include "../helper_functions/common.h"

void * timer_func(void * arg){
    struct updater_data * data = arg;
    (void) data;
    return NULL;
}
