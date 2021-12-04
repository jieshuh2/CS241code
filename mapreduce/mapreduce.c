/**
 * mapreduce
 * CS 241 - Spring 2021
 */
#include "utils.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    // Create an input pipe for each mapper.
    if (argc != 6) {
        print_usage();
        exit(1);
    }
    int num_map = atoi(argv[argc - 1]);
    if (num_map == 0) {
        print_usage();
        exit(1);
    }
    int pipes[num_map][2];
    for (int i = 0; i < num_map; i++) {
        // int fildes[2];
        // pipes[i] = fildes;
        pipe2(pipes[i], O_CLOEXEC);
    }
    
    // Create one input pipe for the reducer.
    int repipe [2];
    pipe2(repipe, O_CLOEXEC);
    // Open the output file.
    int file = open(argv[2], O_CREAT | O_TRUNC | O_RDWR, S_IWUSR | S_IRUSR);
    pid_t splitpid[num_map];
    pid_t mappid[num_map];
    // pid_t redpid[num_map];
    // Start a splitter process for each mapper.
    for (int i = 0; i < num_map; i++) {
        pid_t child = fork();
        splitpid[i] = child;
        if(child == 0) {
            // Child
            
            // Make the stdout of the process, the write end
            dup2(pipes[i][1], 1);
            
            // Exec! Don't forget the cast
            char idx[3];
            sprintf(idx, "%d", i);
            execlp("./splitter", "./splitter", argv[1], argv[argc - 1], idx, (char*)NULL);
            printf("execlpfail\n");
            exit(-1);
        }
        //close parent's write end
        close(pipes[i][1]);
    }

    // Start all the mapper processes.
    for (int i = 0; i < num_map; i++) {
        mappid[i] = fork();
        if (!mappid[i]) {
            dup2(pipes[i][0], 0);
            dup2(repipe[1], 1);
            execlp(argv[3], argv[3], (char*)NULL);
            printf("execlpfail");
            exit(-1);
        }
        //close parent's write end
    }
    close(repipe[1]);
    // Start the reducer process.
    
    pid_t child = fork();
    if (child == 0) {
        dup2(repipe[0], 0);
        dup2(file, 1);
        execlp(argv[4], argv[4], (char*)NULL);
        exit(-1);
    }
    // setvbuf(stdout, NULL, _IONBF, 0);
    // Wait for the reducer to finish.
    for(int i = 0; i < num_map; i++) {
        int status;
        waitpid(splitpid[i], &status, 0);
        if (splitpid[i] != -1 && WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            if (exit_status != 0) {
                print_nonzero_exit_status("/splitter", exit_status);
            }
        } 
        waitpid(mappid[i], &status, 0);
        if (mappid[i] != -1 && WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            if (exit_status != 0) {
                print_nonzero_exit_status(argv[3], exit_status);
            }
        } 
        
    }
    int status2;
    waitpid(child, &status2, 0);
        if (child != -1 && WIFEXITED(status2)) {
            int exit_status = WEXITSTATUS(status2);
            if (exit_status != 0) {
                print_nonzero_exit_status(argv[4], exit_status);
        }
    } 
    // Print nonzero subprocess exit codes.
    // for (int i = 0; i < num_map; i++) {
    //     close(pipes[i][0]);
    // }
    // close(file);
    // Count the number of lines in the output file.
    print_num_lines(argv[2]);
    return 0;
}
