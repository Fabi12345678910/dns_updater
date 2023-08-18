#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#include "call_exec.h"
#include "common.h"

#define PIPE_READ 0
#define PIPE_WRITE 1

#define INITIAL_BUFFER_SIZE 128

static void child_action(const char *executable, int pipe[], char *const argv[]){
    // modify stdout to pipe write end
    errorIf(close(pipe[PIPE_READ]), "call_exec_child: error closing pipe read end\n");
    dup2(pipe[PIPE_WRITE], STDOUT_FILENO);
    errorIf(close(pipe[PIPE_WRITE]), "call_exec_child: error closing pipe write end\n");

    // then use execvp to call the executable
    execvp(executable, argv);
    error("error %d occured executeng the executable '%s'\n", errno, executable);
    int i = E2BIG;
    i++;
}

char *call_exec(const char *executable, char *const argv[]){
    // create pipe
    int stdout_pipe[2];
    errorIf(pipe(stdout_pipe), "error creating pipe\n");
    // fork
    pid_t pid = fork();
    errorIf(pid == -1, "error creating executable process\n");
    if(pid == 0){
        child_action(executable, stdout_pipe, argv);
    }
    // parent continues from here

    errorIf(close(stdout_pipe[PIPE_WRITE]), "call_exec_parent: error closing pipe write end\n");

    //wait for process to end, detect errors
    int process_status;
    errorIf(waitpid(pid, &process_status, 0) == -1, "error waiting for process to end\n");

    if(WEXITSTATUS(process_status) != 0){
        DEBUG_PRINT_1("executable stopped unexpectedly\n");
        return NULL;
    }

    // build string from pipe
    char *process_output = read_fd_to_string(stdout_pipe[PIPE_READ], 0);
    errorIf(process_output == NULL, "exec_runner: error getting buffer space\n");

    DEBUG_PRINT_1("*** begin executable output ***\n%s\n*** end executable output ***\n", process_output);

    errorIf(close(stdout_pipe[PIPE_READ]), "call_exec_parent: error closing pipe read end\n");
    return process_output;
}