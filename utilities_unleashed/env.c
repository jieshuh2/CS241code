/**
 * utilities_unleashed
 * CS 241 - Spring 2021
 */
#include <unistd.h>
#include "format.h"
#include <stdlib.h>
#include <stdio.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

extern char ** environ;
int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_env_usage();
    }
    fflush(stdin); 
    pid_t pid = fork();
    if (pid < 0) { // fork failure
        print_fork_failed();
        exit(1);
    } else if (pid > 0) {
        //parent
        int status;
        waitpid(pid, &status, 0);
    } else {
        int finda = 0;
        char** ptr = argv + 1;
        while(*ptr) {
            if (strcmp(*ptr, "--") == 0) {
                finda = 1;
                break;
            }
            char* enval = *ptr;
            size_t len = strlen(enval);
            char str[len + 1];
            strcpy(str, enval);
            char* key = str;
            char* value = str;
            int find = 0;
            for (size_t i = 0; i < len; i++) {
                if (str[i] == '=') {
                    str[i] = '\0';
                    value = str + i + 1;
                    find = 1;
                    break;
                }
            }
            if (find == 0) {
                print_env_usage();
            }
            if (value[0] == '%') {
                value = getenv(value + 1);
            }
            int set = setenv(key,value,1);
            if (set != 0) {
                print_environment_change_failed();
                exit(1);
            }
            ptr ++;
        } 
        if (finda) {
            ptr++;
            if (*ptr == NULL) {
                print_env_usage();
            }
            execvp(ptr[0], ptr);
            print_exec_failed();
            exit(1); // For safety.
        } else {
            print_env_usage();
        }
    }
    return 0;
}


