#include <unistd.h>

#include "call_curl.h"
#include "common.h"

#define PIPE_READ 0
#define PIPE_WRITE 1

#define INITIAL_BUFFER_SIZE 128

void child_action(int pipe[], char *const argv[]){
    // modify stdout to pipe write end
    dup2(pipe[PIPE_WRITE], STDOUT_FILENO);
    errorIf(close(pipe[PIPE_READ]), "error closing pipe\n");
    errorIf(close(pipe[PIPE_WRITE]), "error closing pipe\n");

    // then use execvp to call curl
    // perhaps use a method to static link this, so we can run without curl, purely using the kernel
    execvp("curl", argv);
}

char * call_curl(char *const argv[]){
    // create pipe
    int stdout_pipe[2];
    errorIf(pipe(stdout_pipe), "error creating pipe\n");
    // fork
    pid_t pid = fork();
    errorIf(pid == -1, "error creating curl process\n");
    if(pid == 0){
        child_action(stdout_pipe, argv);
    }
    // parent continues from here

    errorIf(close(stdout_pipe[PIPE_WRITE]), "error closing pipe\n");

    // build string from pipe
    size_t buffer_size = INITIAL_BUFFER_SIZE;
    size_t total_read_bytes = 0;
    ssize_t read_bytes = 0;
    char *buffer = malloc(INITIAL_BUFFER_SIZE);
    errorIf(buffer == NULL, "curl: error getting buffer space\n");

    //wait for curl to end, detect errors

    //string building
    while(1){
        read_bytes = read(stdout_pipe[PIPE_READ], buffer+total_read_bytes, buffer_size-total_read_bytes);
        DEBUG_PRINT_1("read %zd bytes\n", read_bytes);
        errorIf(read_bytes < 0, "error reading curl output\n");
        if(read_bytes == 0){
            //reached EOF
            break;
        }
        total_read_bytes += read_bytes;

        if (total_read_bytes >= buffer_size){
            DEBUG_PRINT_1("increasing buffer size from %zu to %zu\n", buffer_size, buffer_size * 2);
            buffer = realloc(buffer, buffer_size * 2);
            errorIf(buffer == NULL, "curl: error getting buffer space\n");
            buffer_size *= 2;
        }
    }

    buffer[total_read_bytes] = '\0';
    DEBUG_PRINT_1("*** begin curl output ***\n%s\n*** end curl output ***\n", buffer);

    errorIf(close(stdout_pipe[PIPE_READ]), "error closing pipe\n");
    return buffer;
}