#include <unistd.h>
#include <sys/wait.h>

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

    //wait for curl to end, detect errors
    int curl_status;
    errorIf(waitpid(pid, &curl_status, 0) == -1, "error waiting for curl\n");

    if(WEXITSTATUS(curl_status) != 0){
        //curl stopped unexpectedly, returning NULL
        DEBUG_PRINT_1("curl stopped unexpectedly\n");
        return NULL;
    }
//    errorIf()

    // build string from pipe
    char *curl_output = read_fd_to_string(stdout_pipe[PIPE_READ], 0);
    errorIf(curl_output == NULL, "curl: error getting buffer space\n");

    DEBUG_PRINT_1("*** begin curl output ***\n%s\n*** end curl output ***\n", curl_output);

    errorIf(close(stdout_pipe[PIPE_READ]), "error closing pipe\n");
    return curl_output;
}