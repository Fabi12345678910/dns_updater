#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>

#include "helper_functions/common.h"
#include "helper_functions/configuration_reader.h"
#include "thread_functions/thread_functions.h"

#define CLI_PARAM_FILE "file"
#define CLI_PARAM_STRING "cli"

static volatile sig_atomic_t sigint_shutdown = 0;
extern int test_implementation(void);
static void main_sigint_handler(int signal);

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
        char* config_string = read_file_to_string(argv[2]);
        if(config_string == NULL){
            return RETURN_ERROR;
        }
        updater_data = read_config_from_string(config_string);
        free(config_string);
    }else if (!strcmp(argv[1], CLI_PARAM_STRING)){
        updater_data = read_config_from_string(argv[2]);
    }else{
        goto print_usage;
    }

    //now start the following threads:
    // - updater, waits for conditional variable
    // - timed trigger, sets the conditional variable each 5 mins
    // - http server, receives either a GET -> responds with status or triggers update depending on path of GET, or a POST -> introduces new configuration
    // 

    pthread_t updater_thread, timer_thread, http_server_thread;

    errorIf(pthread_create(&updater_thread, NULL, updater_func, (void *) updater_data), "error creating updater thread\n");
    errorIf(pthread_create(&timer_thread, NULL, timer_func, (void *) updater_data), "error creating timer thread\n");
    if(updater_data->config.enable_http_server){
        errorIf(pthread_create(&http_server_thread, NULL, http_server_func, (void *) updater_data), "error creating http server thread\n");
    }

    //register sigint handler to gracefully shut down the updater
    struct sigaction new_action;
    new_action.sa_handler = main_sigint_handler;
    new_action.sa_flags = 0;
    expect_fine(sigemptyset(&new_action.sa_mask));
    errorIf(sigaction(SIGINT, &new_action, NULL), "error registering signal handler\n");

    while(1){
        pause();
        DEBUG_PRINT_1("checking for received sigint\n");
        if(sigint_shutdown){

            //i would like to not use the broadcast, but without broadcasting/signaling
            //the updater_thread would not wake up from pthread_cond_wait
            expect_fine(pthread_mutex_lock(&updater_data->ipc_data.mutex_update_shutdown_requested));
            updater_data->ipc_data.shutdown_requested = 1;
            expect_fine(pthread_mutex_unlock(&updater_data->ipc_data.mutex_update_shutdown_requested));
            expect_fine(pthread_cond_broadcast(&updater_data->ipc_data.cond_update_shutdown_requested));
            
            pthread_kill(updater_thread, SIGINT);
            pthread_kill(timer_thread, SIGINT);
            if(updater_data->config.enable_http_server){
                pthread_kill(http_server_thread, SIGINT);
            }
//            expect_fine(pthread_cond_broadcast(&updater_data->ipc_data.cond_update_shutdown_requested));

            expect_fine(pthread_join(updater_thread, NULL));
            expect_fine(pthread_join(timer_thread, NULL));
            if(updater_data->config.enable_http_server){
                expect_fine(pthread_join(http_server_thread, NULL));
            }
            break;
        }
    }

    free_config(updater_data);
    free(updater_data);
    return EXIT_SUCCESS;
}

static void main_sigint_handler(int signal){
    if(signal == SIGINT){
        #if (DEBUG_LEVEL >= 1)
        #define SIGHANDLER_MESSAGE "\nmain thread signal_handler: caught SIGINT, exiting program\n"
        (void) (write(STDOUT_FILENO, SIGHANDLER_MESSAGE, sizeof(SIGHANDLER_MESSAGE)) != sizeof(SIGHANDLER_MESSAGE));
        #endif
        sigint_shutdown = 1;
    }
}