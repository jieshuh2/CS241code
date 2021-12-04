// author: jieshuh2
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
static pid_t child_pid = -1;
extern char **environ;
void handler() {
    //kill child program
    if (child_pid != -1) {
        kill(child_pid, SIGINT);
    }
    exit(1);
}
int main(int argc, char **argv) {
    if (argc < 2) {
        exit(1);
    }
    while(1) {
        int status;
        pid_t child = fork();
        if (child == -1) {
            //fork fail
            fprintf(stderr, "fork fail");
            exit(1);
        }
        if (child > 0) {
            //parent
            child_pid = child;
            //wait child process
            pid_t pid = waitpid(child_pid, &status, 0);
            if (pid == -1) {
                //wait fail
                fprintf(stderr, "wait fail");
                exit(1);
            }
            if (pid != -1 && WIFEXITED(status)) {
                int exit_status = WEXITSTATUS(status);
                return exit_status;
            } else {
                if (WIFSIGNALED(status)){
                // It was terminated by a signal
                    if (WTERMSIG(status) == SIGSEGV) {
                        // It was terminated by a segfault
                        //try again
                        child_pid = -1;
                        continue;
                    }
                }
                //not sigfault return;
                return 1;
            }
                
        } else {
            execvp(argv[1], argv + 1);
            fprintf(stderr, "exec fail");
            exit(1);
        }
    }
    
}