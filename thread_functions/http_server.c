// thread_functions:
#include "http_server.h"

#define _DEFAULT_SOURCE
//#define DEBUG_LEVEL 2
#include "../helper_functions/common.h"
#include "http_macros.h"

#include <string.h>
#include <errno.h>

#ifndef PORT_NUMBER
    #define PORT_NUMBER 8008
#endif

struct connection_handler_data{
    struct updater_data *updater_data;
    int sock;
};
void *connection_handler(void * arg);

void * http_server_func(void * arg){
    struct updater_data * data = arg;

    //do ipv4 socket only, since one should use a reverse proxy or similar anyways to reach the server
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    errorIf(sock == -1, "error creating http socket\n");

    struct sockaddr_in servaddr;
    memset(&servaddr.sin_zero, 0, sizeof(servaddr.sin_zero));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT_NUMBER);    

    errorIf(bind(sock,(struct sockaddr *) &servaddr, sizeof(servaddr)), "error %d binding port\n", errno);
    errorIf(listen(sock, 4), "error listening on socket");

    while(!data->ipc_data.shutdown_requested){
        DEBUG_PRINT_1("http_server: accepting next incoming connection\n");
        struct connection_handler_data handler_data;
        handler_data.sock = accept(sock, NULL, NULL);
        if(handler_data.sock == -1 && errno == EINTR){
            DEBUG_PRINT_1("http_server: got interrupted while accepting\n");
            continue;
        }
        errorIf(handler_data.sock == -1, "error accepting client\n");
        DEBUG_PRINT_0("http_server: answering incoming connection\n");
        handler_data.updater_data = data;
        //use only one thread for the http server as of now, this is way easier to terminate
        connection_handler((void*) &handler_data);
/*        pthread_t handler_thread;
        pthread_attr_t attr;
        errorIf(pthread_attr_init(&attr), "error initializing attributes\n");
        expect_fine(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED));
        errorIf(pthread_create(&handler_thread, &attr, connection_handler, (void*) handler_data), "error creating connection handler thread\n");
*/
    }
    expect_fine(close(sock));
    DEBUG_PRINT_0("http_server: returning\n");
    return NULL;
}

int starts_with(const char * comparison, char ** working_ptr){
    char * original_working_buffer = *working_ptr;
    size_t comparison_length = strlen(comparison);
    for(size_t i = 0; i < comparison_length; i++){
        if(comparison[i] == **working_ptr){
            (*working_ptr)++;
        }else{
            *working_ptr = original_working_buffer;
            return 0;
        }
    }
    return 1;
}

struct http_responce{
    char* responce_head;
    char* content_type;
    char* content;
};

void handle_dns_update(struct connection_handler_data * data);
void handle_health_check(struct connection_handler_data * data);
void handle_state(struct connection_handler_data * data);
void handle_base_path(struct connection_handler_data * data);
void handle_invalid_path(struct connection_handler_data * data);
void handle_malformed_request(struct connection_handler_data * data);

void *connection_handler(void * arg){
    struct connection_handler_data * data = (struct connection_handler_data *) arg;
    DEBUG_PRINT_1("http_connection_handler: reading http request to string\n");
    char* request = read_fd_to_string(data->sock, 1);
    if(request == NULL){
        DEBUG_PRINT_1("error reading request\n");
        expect_fine(close(data->sock));
        return NULL;
    }

    char * work_ptr = request;
    if(starts_with("GET", &work_ptr)){
        if(!starts_with(" ", &work_ptr)){
            //return 400 malformed biiitch
            handle_malformed_request(data);
        }else if(starts_with("/dns-update ", &work_ptr)){
            //trigger dns update and return 200
            handle_dns_update(data);
        }else if (starts_with("/health-check ", &work_ptr)){
            //return 200 if healthy, else return 500
            handle_health_check(data);
        }else if (starts_with("/state ", &work_ptr)){
            //return status page with 200
            handle_state(data);
        }else if (starts_with("/ ", &work_ptr)){
            //return overview page or redirect to /state
            handle_base_path(data);
        }else{
            //return 404
            handle_invalid_path(data);
        }
    }else{
        //return 400 malformed request
        handle_malformed_request(data);
    }

    free(request);
    close(data->sock);
    return NULL;
}

int send_responce(struct http_responce *responce, int sock){
    unsigned content_length = (unsigned) strlen(responce->content);
    return dprintf(sock, HTTP_RESPONCE_BLUEPRINT, responce->responce_head, content_length, responce->content_type, responce->content);
}

void handle_invalid_path(struct connection_handler_data *data){
    struct http_responce responce = {.responce_head = "HTTP/1.0 404 not found", .content_type = "text/html", .content = "<h1>404 Site not found</h1>"};
    send_responce(&responce, data->sock);
}

void handle_base_path(struct connection_handler_data *data){
    dprintf(data->sock, HTTP_RESPONCE_REDIRECT);
}

void handle_dns_update(struct connection_handler_data *data){
    struct http_responce responce = {.responce_head = "HTTP/1.0 200 OK", .content_type = "text/html", .content= "<h1>dns update triggered</h1>"};
    send_responce(&responce, data->sock);
    //sleep for some seconds in case the ip update is not fully done yet
    sleep(5);
    expect_fine(pthread_mutex_lock(&data->updater_data->ipc_data.mutex_update_shutdown_requested));
    data->updater_data->ipc_data.update_requested = 1;
    expect_fine(pthread_mutex_unlock(&data->updater_data->ipc_data.mutex_update_shutdown_requested))
    expect_fine(pthread_cond_signal(&data->updater_data->ipc_data.cond_update_shutdown_requested));
}

void handle_health_check(struct connection_handler_data *data){
    struct http_responce responce = {.responce_head = "HTTP/1.0 200 OK" ,.content_type = "text/html", .content= "<h1>all fine in the hood</h1>"};
    //if error occured
    if(any_error_occured(data->updater_data)){
        responce.responce_head = "HTTP/1.0 500 UNHEALTHY";
        responce.content = "<h1> unhealthy :( </h1>";
    }
    send_responce(&responce, data->sock);
}

void handle_malformed_request(struct connection_handler_data *data){
    struct http_responce responce = {.responce_head = "HTTP/1.0 400 MALFORMED", .content = "<h1>malformed request</h1>", .content_type = "text/html"};
    send_responce(&responce, data->sock);
}

static const char* state_as_style(state stat){
    if(stat == STATE_OKAY) return "okay";
    if(stat == STATE_WARNING) return "warn";
    if(stat == STATE_ERROR) return "error";
    if(stat == STATE_UNUSED) return "unused";
    return "undefined";
}

static const char* state_as_message(state stat){
    if(stat == STATE_OKAY) return "OKAY";
    if(stat == STATE_WARNING) return "WARNING";
    if(stat == STATE_ERROR) return "ERROR";
    if(stat == STATE_UNUSED) return "UNUSED";
    return "UNDEFINED";
}

void handle_state(struct connection_handler_data * data){
    struct http_responce responce = {.responce_head = "HTTP/1.0 200 OK", .content_type = "text/html", .content = NULL};

    state global_health_state = any_error_occured(data->updater_data) ? STATE_ERROR : STATE_OKAY;

    //build dns states
    errorIf(pthread_mutex_lock(&data->updater_data->ipc_data.mutex_dns_list), "handle_state: error acquiring dns list mutex\n");
    char dns_states[1 + 400 * linked_list_size(data->updater_data->managed_dns_list)];
    dns_states[0] = '\0';

    int total_written_bytes = 0;
    int written_bytes;
    iterator iter_dns;
    linked_list_iterator_init(data->updater_data->managed_dns_list, &iter_dns);

    while(ITERATOR_HAS_NEXT(&iter_dns)){
        struct managed_dns_entry * dns_entry = DNS_ITERATOR_NEXT(&iter_dns);
        written_bytes = snprintf(dns_states+total_written_bytes, sizeof(dns_states) - total_written_bytes, "        <li class=\"status %s\">%s(%s) - %s</li>\r\n",
            state_as_style(dns_entry->dns_data.entry_state), dns_entry->dns_data.dns_name, dns_entry->dns_data.dns_type, state_as_message(dns_entry->dns_data.entry_state));
        total_written_bytes += written_bytes;
        if(total_written_bytes == (int) sizeof(dns_states)){
            fprintf(stderr, "missing space for dns entry responce");
            return;
        }
    }

    errorIf(pthread_mutex_unlock(&data->updater_data->ipc_data.mutex_dns_list), "handle_state: error releasing dns list mutex\n");

    //build log messages
    errorIf(pthread_mutex_lock(&data->updater_data->ipc_data.mutex_info), "handle_state: error acquiring logging mutex\n");
    int log_buffer_size = 1;
    char *logs_in_html = malloc(1);
    int write_counter = 0;
    errorIf(logs_in_html == NULL, "handle_state: error getting buffer space\n");
    logs_in_html[0] = '\0';
    iterator iter_log;
    errorIf(circular_array_iterator_init(&data->updater_data->ipc_data.info.logging_array, &iter_log), "handle_state: error initializing logging iterator");
    while(ITERATOR_HAS_NEXT(&iter_log)){
        char ** message = (char **) ITERATOR_NEXT(&iter_log);
        log_buffer_size = log_buffer_size + HTML_ROW_LOG_MESSAGE_SIZE + strlen(*message);
        logs_in_html = realloc(logs_in_html, log_buffer_size);
        errorIf(logs_in_html == NULL, "handle_state: error getting buffer space\n");
        write_counter += sprintf(logs_in_html + write_counter, HTML_ROW_LOG_MESSAGE, *message);
    }

    circular_array_iterator_free(&iter_log);

    char html_content[sizeof(HTML_STATE) + 100 + strlen(dns_states) + strlen(logs_in_html)];
    if(snprintf(html_content, sizeof(html_content), HTML_STATE,
        state_as_style(global_health_state), state_as_message(global_health_state),
        state_as_style(data->updater_data->ipc_data.info.ip4_state), 
            state_as_message(data->updater_data->ipc_data.info.ip4_state),
        state_as_style(data->updater_data->ipc_data.info.ip6_state), 
            state_as_message(data->updater_data->ipc_data.info.ip6_state),
        dns_states,
        logs_in_html
        ) >= (int) sizeof(html_content)){
        fprintf(stderr, "error writing html state responce\n");
        return;
    }
    errorIf(pthread_mutex_unlock(&data->updater_data->ipc_data.mutex_info), "handle_state: error acquiring logging mutex\n");

    responce.content = html_content;

    send_responce(&responce, data->sock);
    free(logs_in_html);
}