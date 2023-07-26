#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "helper_functions/common.h"
#include "helper_functions/configuration_reader.h"
#include "thread_functions/thread_functions.h"

#define CLI_PARAM_FILE "file"
#define CLI_PARAM_STRING "cli"

extern int test_implementation(void);

int main(int argc, char const *argv[])
{
    (void) argv;
    if (argc == 1){
        printf("no arguments provided, testing implementation\n");
        return test_implementation();
    }else if (argc != 3){
    print_usage:
        fprintf(stderr, "usage: %s %s <filename> | %s %s <configurationText>\n", argv[0], CLI_PARAM_FILE, argv[0], CLI_PARAM_STRING);
        return EXIT_FAILURE;
    }


    struct updater_data *updater_data = NULL;
    
    if(!strcmp(argv[1], CLI_PARAM_FILE)){
        //@todo config via file
    }else if (!strcmp(argv[1], CLI_PARAM_STRING)){
        updater_data = read_config_from_string(argv[2]);
    }else{
        goto print_usage;
    }

    //now start the following threads:
    // - updater, waits for conditional variable
    // - timed trigger, sets the conditional variable each 5 mins
    // - http server, receives either a GET -> responds with status or triggers depending on path of GET, or a POST -> introduces new configuration
    // 

    pthread_t updater_thread, timer_thread, http_server_thread;

    errorIf(pthread_create(&updater_thread, NULL, updater_func, (void *) updater_data), "error creating updater thread\n");
    errorIf(pthread_create(&timer_thread, NULL, timer_func, (void *) updater_data), "error creating timer thread\n");
    errorIf(pthread_create(&http_server_thread, NULL, http_server_func, (void *) updater_data), "error creating http server thread\n");

    free_config(updater_data);
    return EXIT_SUCCESS;
}
