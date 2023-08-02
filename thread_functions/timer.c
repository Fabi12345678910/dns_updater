// thread_functions:
#include "timer.h"

#include "../helper_functions/common.h"


#ifndef TEST_TIMER
    #define SLEEP_MINUTES 10
    #define SLEEP_SECONDS 60 * SLEEP_MINUTES
#else
    //make the timer real short so we're not wasting minutes waiting for the test result
    #define SLEEP_SECONDS 3
#endif

void * timer_func(void * arg){
    struct updater_data * data = arg;
    //most simple shit, just send an update every SLEEP_SECONDS seconds
    while (!data->ipc_data.shutdown_requested)
    {
        expect_fine(pthread_mutex_lock(&data->ipc_data.mutex_update_shutdown_requested));
        data->ipc_data.update_requested = 1;
        expect_fine(pthread_mutex_unlock(&data->ipc_data.mutex_update_shutdown_requested));
        expect_fine(pthread_cond_signal(&data->ipc_data.cond_update_shutdown_requested));
        sleep(SLEEP_SECONDS);
    }
    
    return NULL;
}
