/**
 * utilities_unleashed
 * CS 241 - Spring 2021
 */
#include <unistd.h>
#include "format.h"
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_time_usage();
        exit(1);
    }
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    pid_t pid = fork();
    if (pid < 0) { // fork failure
        print_fork_failed();
        exit(1);
    } else if (pid > 0) {
        //parent
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            clock_gettime(CLOCK_MONOTONIC, &end);
            double duration = (end.tv_sec - start.tv_sec) * 1e9; 
            duration = (duration + (end.tv_nsec - start.tv_nsec)) * 1e-9;
            display_results(argv, duration);
        }
    } else {
        execvp(argv[1], argv + 1);
        print_exec_failed();
        exit(1); // For safety.
    }
    return 0;
}
