/**
 * shell
 * CS 241 - Spring 2021
 */
#include "format.h"
#include "shell.h"
#include "vector.h"
#include "sstring.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <linux/limits.h>
#include <string.h>
#include <dirent.h> 
#include <ctype.h>
#include <signal.h>

#include <sys/stat.h>
#include <fcntl.h>

extern char ** environ;
extern char *optarg;
extern int optind, opterr, optopt;
typedef struct process {
    char *command; //use to store current directory
    pid_t pid;
} process;

char ** toarguments(char* input, size_t* size);
void freeArg(char** arg);
int external(char* cmd, char** arguments, size_t size);
void background(char* cmd, char** arguments, size_t size, char* command);
int cd (char* comand, char** arguments);
void history(vector * commands);
void firstn(vector * commands, char * num, char* directory, FILE* toread, FILE* towrite, bool keep);
void runcommand(vector * commands, char* directory, char* input, FILE* toread, FILE* towrite, bool keep);
int run (char* directory, char** arguments, size_t size);
void execute(process * processname, char* directory, vector* commands, FILE* toread, FILE* towrite, bool keep);
void prefix(vector * commands, char* directory, char* prefix, FILE* toread, FILE* towrite, bool keep);
void ps();
process_info* info(const process* proc);
void destroy_info(process_info * infostruct);
static pid_t child = -1;
static vector* processes;
void handler() {
    if (child != -1) {
        kill(child, SIGINT);
    }
}
void reapchildren() {
    int status;
    while (1) {
        pid_t clearedzombie = waitpid((pid_t) (-1), &status, WNOHANG);
        if (clearedzombie <= 0) {
            break;
        }
        for (size_t i = 0; i < vector_size(processes); i++) {
            process* proc = (process*)vector_get(processes, i);
            if (proc->pid == clearedzombie) {
                vector_erase(processes, i);
                break;
            }
            free(proc->command);
            free(proc);
        }
    }
}
int shell(int argc, char *argv[]) {
    signal(SIGINT, handler);
    // TODO: This is the entry point for your shell.
    int opt;
    int h = 0;
    int f = 0;
    char* hfile_name = NULL;
    char* ffile_name = NULL;
    while ((opt = getopt(argc, argv, "h:f:")) != -1) {
        switch (opt) {
        case 'h':
            h = 1;
            hfile_name = strdup(optarg);
            break;
        case 'f':
            f = 1;
            ffile_name = strdup(optarg);
            break;
        default: 
            print_usage();
            exit(1);
        }
    }
    char* directory = malloc(PATH_MAX * sizeof(char));//need free
    process *this = (process*) malloc(sizeof(process));//need free
    vector * commands = string_vector_create();//need free

    this->pid = getpid();
    this->command = malloc(sizeof(char*)*100);
    strcpy(this->command, argv[0]);
    for (int i = 1; i < argc; i++) {
        strcat(this->command, argv[i]);
    }

    FILE* ffile = NULL;
    FILE* hfile = NULL;
    if (h && f) {
        ffile = fopen(ffile_name, "r");
        if (ffile == NULL) {
           print_script_file_error();
           free(directory);
           exit(1);
       }
       hfile = fopen(hfile_name, "a");
        if (hfile == NULL) {
           print_history_file_error();
           free(directory);
           exit(1);
       }
       free(ffile_name);
       free(hfile_name);
       execute(this, directory, commands, ffile, hfile, true);
    }
    if (f) {
        ffile = fopen(ffile_name, "r");
        if (ffile == NULL) {
           print_script_file_error();
           free(directory);
           exit(1);
       }
       free(ffile_name);
       free(hfile_name);
       execute(this, directory, commands, ffile, NULL, false);
    }

    if (h) {
        hfile = fopen(hfile_name, "a");
        if (hfile == NULL) {
           print_history_file_error();
           free(directory);
           exit(1);
       }
       free(ffile_name);
       free(hfile_name);
       execute(this, directory, commands, stdin, hfile, true);
       //useless after it
       for (size_t i = 0; i < vector_size(commands); i++) {
           fprintf(hfile, "%s\n", vector_get(commands, i));
        }
        fclose(hfile);
        hfile = NULL;
    } 
    //no mode
    free(ffile_name);
    free(hfile_name);
    execute(this, directory, commands, stdin, NULL, false);

    return 0;
}


void execute(process * processname, char* directory, vector* commands, FILE* toread, FILE* towrite, bool keep) {
    char* input = NULL;
    size_t inputsize = 0;
    processes = shallow_vector_create();
    vector_push_back(processes, processname);
    while(1) {
        reapchildren();
        getcwd(directory, PATH_MAX);
        print_prompt(directory, processname->pid);
        //todo:handle ctr-D signal here; 
        int inputlen = getline(&input,&inputsize,toread); 
        if (inputlen == -1) {
            if (toread != stdin) {
                fclose(toread);
            }
            if (keep) {
                for (size_t i = 0; i < vector_size(commands); i++) {
                    fprintf(towrite, "%s\n", vector_get(commands, i));
                }
                fclose(towrite);
            }
            free(directory);
            free(processname);
            vector_destroy(commands);
            exit(0);
        }
        if (toread != stdin) {
            if (input[strlen(input) - 1] == '\n') {
                input[strlen(input) - 1] = '\0';
            }
            print_command(input);
        }          
        runcommand(commands, directory, input, toread, towrite, keep);
    }
}


char ** toarguments(char* input, size_t * size) { //convert char* input to char** arguments
    // int inputlen = getline(&input, size, toread);
    if (input[strlen(input) - 1] == '\n') {
        input[strlen(input) - 1] = '\0';
    }
    while(input[strlen(input) - 1] == ' ') {
        input[strlen(input) - 1] = '\0';
    }
    while (input[0] == ' ') {
        input++;
    }
    sstring* str = cstr_to_sstring(input); //need to free
    vector *args = sstring_split(str, ' '); //need to free
    sstring_destroy(str);
    str = NULL;
    size_t argn = vector_size(args);
    char** arguments = malloc(sizeof(char*) * (1 + argn));
    *size = argn;
    arguments[argn] = NULL;
    for (size_t i = 0; i < argn; i++) {
        arguments[i] = strdup(vector_get(args, i));
    }
    vector_destroy(args);
    args = NULL;
    return arguments;
}
int external(char* cmd, char** arguments, size_t size) {
    int status;
    pid_t child_pid = fork();
    child = child_pid;
    if (child_pid == -1) {
        print_fork_failed();
         //Failed
        exit(1);
    }
    if (child_pid > 0) {
    // Parent, wait for child to finish
        // signal(SIGINT, forground);
        pid_t pid = waitpid(child_pid, &status, 0);
        if (pid == -1) {
            print_wait_failed();
        }
        if (pid != -1 && WIFEXITED(status)) {
            child = -1;
            int exit_status = WEXITSTATUS(status);
            return exit_status;
        } 
        child = -1;
            
    } else {
    // Child, do something interesting
        setvbuf(stdout, NULL, _IONBF, 0);
        pid_t pid = getpid();
        print_command_executed(pid);
        int pfd = -1;
        if (size > 2) {
            for (size_t i = 0; i < size - 1; i++) {
                if (strcmp(arguments[i], ">") == 0) {
                    pfd = open(arguments[i + 1], O_CREAT | O_TRUNC | O_WRONLY, S_IWUSR |  S_IRUSR);
                    if (pfd == -1) {
                        print_redirection_file_error();
                        exit(1);
                    }
                    for (size_t j = i; j < size; j++) {
                        free(arguments[j]);
                        arguments[j] = NULL;
                    }
                    close(1);
                    int successdup = dup(pfd);
                    if (successdup == -1) {
                        print_redirection_file_error();
                        exit(1);
                    }
                    break;
                }
                if (strcmp(arguments[i], ">>") == 0) {
                    pfd = open(arguments[i + 1], O_CREAT | O_APPEND | O_WRONLY, S_IWUSR | S_IRUSR);
                    if (pfd == -1) {
                        print_redirection_file_error();
                        exit(1);
                    }
                    for (size_t j = i; j < size; j++) {
                        free(arguments[j]);
                        arguments[j] = NULL;
                    }
                    close(1);
                    int successdup = dup(pfd);
                    if (successdup == -1) {
                        print_redirection_file_error();
                        exit(1);
                    }
                    break;
                }
                if (strcmp(arguments[i], "<") == 0) {
                    pfd = open(arguments[i + 1], O_RDONLY, S_IRUSR);
                    if (pfd == -1) {
                        print_redirection_file_error();
                        exit(1);
                    }
                    size_t j = i;
                    while(arguments[j] != NULL) {
                        free(arguments[j]);
                        arguments[j] = NULL;
                        j++;
                    }
                    close(0);
                    int successdup = dup(pfd);
                    if (successdup == -1) {
                        print_redirection_file_error();
                        exit(1);
                    }
                    break;
                }
            }
        }
        execvp(cmd, arguments); // "ls ."
        print_exec_failed(cmd);
        exit(1);
    }
    return 0;
}
void background(char* cmd, char** arguments, size_t size, char* command) {
    //I want to delete the "&" in the arguments;
    free(arguments[size - 1]);
    arguments[size - 1] = NULL;
    pid_t child_pid = fork();
    if (child_pid == -1) { 
            print_fork_failed();
            //Failed
            exit(1);
    }

    if (child_pid == 0) {
        // Do background stuff e.g. call exec
        pid_t pid = getpid();
        print_command_executed(pid);
        //redirect
        int pfd = -1;
        if (size > 2) {
            for (size_t i = 0; i < size - 1; i++) {
                if (strcmp(arguments[i], ">") == 0) {
                    pfd = open(arguments[i + 1], O_CREAT | O_TRUNC | O_WRONLY, S_IWUSR |  S_IRUSR);
                    if (pfd == -1) {
                        print_redirection_file_error();
                        exit(1);
                    }
                    for (size_t j = i; j < size; j++) {
                        free(arguments[j]);
                        arguments[j] = NULL;
                    }
                    close(1);
                    int successdup = dup(pfd);
                    if (successdup == -1) {
                        print_redirection_file_error();
                        exit(1);
                    }
                    break;
                }
                if (strcmp(arguments[i], ">>") == 0) {
                    pfd = open(arguments[i + 1], O_CREAT | O_APPEND | O_WRONLY, S_IWUSR | S_IRUSR);
                    if (pfd == -1) {
                        print_redirection_file_error();
                        exit(1);
                    }
                    for (size_t j = i; j < size; j++) {
                        free(arguments[j]);
                        arguments[j] = NULL;
                    }
                    close(1);
                    int successdup = dup(pfd);
                    if (successdup == -1) {
                        print_redirection_file_error();
                        exit(1);
                    }
                    break;
                }
                if (strcmp(arguments[i], "<") == 0) {
                    pfd = open(arguments[i + 1], O_RDONLY, S_IRUSR);
                    if (pfd == -1) {
                        print_redirection_file_error();
                        exit(1);
                    }
                    size_t j = i;
                    while(arguments[j] != NULL) {
                        free(arguments[j]);
                        arguments[j] = NULL;
                        j++;
                    }
                    close(0);
                    int successdup = dup(pfd);
                    if (successdup == -1) {
                        print_redirection_file_error();
                        exit(1);
                    }
                    break;
                }
            }
        }
        execvp(cmd, arguments); // "ls ."
        print_exec_failed(cmd);
        exit(1);
    } else { /* I'm the parent! */
        process* process = malloc(sizeof(process));
        process->pid = child_pid;
        process->command = strdup(command);
        vector_push_back(processes, process);
        int success = setpgid(child_pid, 0);
        if (success == -1) {
            print_setpgid_failed();
        }
        return;
    }
}

void freeArg(char** arg) {
    if (arg == NULL) {
        return;
    }
    char** ptr = arg;
    while (*ptr != NULL) {
        free(*ptr);
        *ptr = NULL;
        ptr++;
    }
    free(arg);
}

void runcommand(vector * commands, char* directory, char* input, FILE* toread, FILE* towrite, bool keep) {//todo:logical operation.
    
    // char *first_part = strtok(line, ";"); //first_part points to "user"
    // char *sec_part = strtok(NULL, ";");
    size_t* size = (size_t*) malloc(sizeof(size_t));
    char* iinput = strdup(input);
    size_t len = strlen(iinput);
    if (iinput[len - 1] == '\n') {
        iinput[len - 1] = '\0';
    }
    //build in commands
    if (strcmp(iinput, "\0") == 0) {
        free(size);
        free(iinput);
        return;
    }
    if (strcmp(iinput, "exit") == 0) {
        if (toread != stdin) {
            fclose(toread);
        }
        if (keep) {
            for (size_t i = 0; i < vector_size(commands); i++) {
                fprintf(towrite, "%s\n", vector_get(commands, i));
            }
            fclose(towrite);
        }
        
        free(directory);
        vector_destroy(commands);
        free(iinput);
        free(size);
        exit(0);
    }
    if (strcmp(iinput, "!history") == 0) {
        //history. only for h mode
        history(commands);
        free(size);
        free(iinput);
        return;
    } else if (strcmp(iinput, "ps") == 0) {
        vector_push_back(commands, iinput);
        ps();
        free(size);
        free(iinput);
        return;
    } else if (iinput[0] == '#') {
        //#n
        firstn(commands, iinput + 1, directory, toread, towrite, keep);
        free(size);
        free(iinput);
        return;
    } else if (iinput[0] == '!') {
        //prefix. only for h mode
        prefix(commands, directory, iinput + 1, toread, towrite, keep);
        free(size);
        free(iinput);
        return;
    } 
    //logical
    char* firstarg = iinput;
    char* secondarg = NULL;
    bool both = false;
    bool and = false;
    bool or = false;

    for (size_t i = 0; i < len; i++) {
        if(iinput[i] == ';') {
            both = true;
            vector_push_back(commands, iinput);
            iinput[i] = '\0';
            secondarg = iinput + i + 1;
            break;
        }
        if (iinput[i] == '&' && i < len - 1 && iinput[i + 1] == '&') {
            and = true;
            vector_push_back(commands, iinput);
            iinput[i] = '\0';
            iinput[i + 1] = '\0';
            secondarg = iinput + i + 2;
            break;
        }
        if (iinput[i] == '|' && i < len - 1 && iinput[i + 1] == '|') {
            or = true;
            vector_push_back(commands, iinput);
            iinput[i] = '\0';
            iinput[i + 1] = '\0';
            secondarg = iinput + i + 2;
            break;
        }
    }
    char** first = NULL;
    char** second = NULL;
    //try find ;
    if (both) {
        //runfirst
        first = toarguments(firstarg, size);
        fflush(toread);
        if (towrite != NULL) {
            fflush(towrite);
        }
        run(directory, first, *size);
        freeArg(first);
        //runsecond
        second = toarguments(secondarg, size);
        fflush(toread);
        if (towrite != NULL) {
            fflush(towrite);
        }
        run(directory, second, *size);
        freeArg(second);
        free(iinput);
        free(size);
        return;
    }
    //try find &&
    if (and) {
        first = toarguments(firstarg, size);
        fflush(toread);
        if (towrite != NULL) {
            fflush(towrite);
        }
        int success1 = run(directory, first, *size);
        freeArg(first);
        //runsecond
        second = toarguments(secondarg, size);
        if (success1 == 0) {
            fflush(toread);
            if (towrite != NULL) {
                fflush(towrite);
            }
            run(directory, second, *size);
        }
        
        free(iinput);
        freeArg(second);
        free(size);
        return;
    }
    //try find ||
    // first_part = strtok(line, "||"); //first_part points to "user"
    // sec_part = strtok(NULL, "||");
    if (or) {
        first = toarguments(firstarg, size);
        fflush(toread);
        if (towrite != NULL) {
            fflush(towrite);
        }
        int success1 = run(directory, first, *size);
        freeArg(first);
        //runsecond
        second = toarguments(secondarg, size);
        if (success1 != 0) {
            fflush(toread);
            if (towrite != NULL) {
                fflush(towrite);
            }
            run(directory, second, *size);
        }
        free(iinput);
        freeArg(second);
        free(size);
        return;
    }
    //single argument
    char** arguments = toarguments(iinput, size);//need to destroy arguments
    if (strcmp(arguments[0], "kill") == 0) {
        if (arguments[1] == NULL) {
            print_invalid_command(iinput);
        } else {
            process* tokill = NULL;
            bool findproc = false;
            for (size_t i = 0; i < vector_size(processes); i++) {
                tokill = (process*)vector_get(processes, i);
                if (tokill->pid == atoi(arguments[1])) {
                    findproc = true;
                    break;
                }
            }
            if (findproc == false) {
                print_no_process_found(atoi(arguments[1]));
            } else {
                int success = kill(tokill->pid, SIGKILL);
                if (success == 0) {
                    print_killed_process(tokill->pid, tokill->command);
                }
            }
        }
    } else if (strcmp(arguments[0], "stop") == 0) {
        if (arguments[1] == NULL) {
            print_invalid_command(iinput);
        } else {
            process* tokill = NULL;
            bool findproc = false;
            for (size_t i = 0; i < vector_size(processes); i++) {
                tokill = (process*)vector_get(processes, i);
                if (tokill->pid == atoi(arguments[1])) {
                    findproc = true;
                    break;
                }
            }
            if (findproc == false) {
                print_no_process_found(atoi(arguments[1]));
            } else {
                int success = kill(tokill->pid, SIGSTOP);
                if (success == 0) {
                    print_killed_process(tokill->pid, tokill->command);
                }
            }
        }
    } else if (strcmp(arguments[0], "cont") == 0) {
        if (arguments[1] == NULL) {
            print_invalid_command(iinput);
        } else {
            process* tokill = NULL;
            bool findproc = false;
            for (size_t i = 0; i < vector_size(processes); i++) {
                tokill = (process*)vector_get(processes, i);
                if (tokill->pid == atoi(arguments[1])) {
                    findproc = true;
                    break;
                }
            }
            if (findproc == false) {
                print_no_process_found(atoi(arguments[1]));
            } else {
                int success = kill(tokill->pid, SIGCONT);
                if (success == 0) {
                    print_killed_process(tokill->pid, tokill->command);
                }
            }
        }
    } else if (strcmp(arguments[*size - 1], "&") == 0) {
        vector_push_back(commands, iinput);
        fflush(toread);
        if (towrite != NULL) {
            fflush(towrite);
        }
        background(arguments[0], arguments, *size, iinput);
    } else {
        vector_push_back(commands, iinput);
        fflush(toread);
        if (towrite != NULL) {
            fflush(towrite);
        }
        fflush(stdin);
        fflush(stdout);
        run(directory, arguments, *size);
    }
    free(iinput);
    freeArg(arguments);
    free(size);
}

int run (char* directory, char** arguments, size_t size) {
     if (strcmp(arguments[0], "cd") == 0) {
        return cd(directory, arguments);
        
    }  else {
        fflush(stdin);
        fflush(stdout);
        return external(arguments[0], arguments, size);
    }
    return 0;
}

int cd (char* comand, char** arg) {
    if (arg[1] == NULL) {
        strcpy(comand, getenv("HOME"));
    } else {
        char* str = arg[1];
        if (str[0] == '/') {
            strcpy(comand, str);
        } else {
            strcat(comand, "/");
            strcat(comand, str);
        }
    }
    if (chdir(comand) != 0) {
        print_no_directory(comand);
        getcwd(comand, PATH_MAX);
        return 1;
    }
    getcwd(comand, PATH_MAX);
    return 0;
}
void history(vector * commands) {
    for (size_t i = 0; i < vector_size(commands); i++) {
        print_history_line(i, vector_get(commands, i));
    }
}
void firstn(vector * commands, char * num, char* directory, FILE* toread, FILE* towrite, bool keep) {
    int number = atoi(num);
    if ((size_t)number > vector_size(commands)) {
        print_invalid_index();
        return;
    }
    char * input = strdup(vector_get(commands, number));//need free
    print_command(input);
    //wait to check which pid it is whether this or children.
    runcommand(commands, directory, input, toread, towrite, keep);
    free(input);
}
void prefix(vector * commands, char* directory, char* prefix, FILE* toread, FILE* towrite, bool keep) {
    if (strcmp(prefix, "\0") == 0) {
        //notice that commands is a dinamic vector that will change when runcommand. Therefore, we need to record commands before run
        // vector* thispoint = string_vector_create();
        // for (size_t i = 0; i < vector_size(commands); i++) {
        //     vector_push_back(thispoint, vector_get(commands, i));
        // }
        // for (size_t i = 0; i < vector_size(thispoint); i++) {
        //     char* input = strdup(vector_get(thispoint, i));//need free
        //     print_command(input);
        //     //wait to check which pid it is whether this or children.
        //     runcommand(commands, directory, input, toread, towrite, keep);
        //     free(input);
        // }
        // vector_destroy(thispoint);
        char* input = strdup(vector_get(commands, vector_size(commands) - 1));
        print_command(input);
        //wait to check which pid it is whether this or children.
        runcommand(commands, directory, input, toread, towrite, keep);
        free(input);
        return;
    }
    char * input = NULL;
    for (size_t i = 0; i < vector_size(commands); i++) {
        if (strncmp(prefix, vector_get(commands, vector_size(commands) - i - 1), strlen(prefix)) == 0) {
            input = strdup(vector_get(commands, vector_size(commands) - i - 1));
            break;
        }
    }
    if (input == NULL) {
        print_no_history_match();
    } else {
        print_command(input);
        //wait to check which pid it is whether this or children.
        runcommand(commands, directory, input, toread, towrite, keep);
        free(input);
    }
}
void ps() {
    print_process_info_header();
    for (size_t i = 0; i < vector_size(processes); i++) {
        process_info* infostruct = info(vector_get(processes, i));
        print_process_info(infostruct);
        destroy_info(infostruct);
    }
}
process_info* info(const process* proc) {
    setvbuf(stdout, NULL, _IONBF, 0);
    char* statfile = NULL;
    int pid = (int)proc->pid;
    int numread = asprintf(&statfile, "/proc/%d/stat", pid);
    if (numread == -1) {
        printf("fail to get path to stat\n");
        return NULL;
    }
    FILE* stat = fopen(statfile, "r");
    if (stat == NULL) {
        printf("fail to open stat file\n");
        return NULL;
    }
    process_info* infostuct = malloc(sizeof(process_info));
    infostuct -> pid = proc->pid;
    infostuct->command = strdup(proc->command);
    infostuct->start_str = malloc(sizeof(char)*10);
    infostuct->time_str = malloc(sizeof(char)*10);
    unsigned long utime;
    unsigned long stime;
    unsigned long long starttime;
    unsigned long long boottime = 0;
    FILE* statboot = fopen("/proc/stat", "r");
    if (stat == NULL) {
        printf("fail to open /proc/stat file\n");
        return NULL;
    }
    char* line = NULL;
    size_t size;
    while(getline(&line,&size,statboot) != -1) {
        if (strncmp(line, "btime", 5) == 0) {
            char* ptr = line + 6;
            char *eptr;
            boottime = strtoll(ptr, &eptr, 10);
            break;
        }
    }
    if (boottime == 0) {
        printf("fail to extract boottime");        
    }
    free(line);
    fscanf(stat, "%d %*s %c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu %lu %lu %*ld %*ld %*ld %*ld %ld %*ld %llu %lu", &infostuct->pid, &infostuct->state, &utime, &stime, 
    &infostuct->nthreads, &starttime, &infostuct->vsize);
    infostuct->vsize /= 1024;
    time_t startime = starttime/sysconf (_SC_CLK_TCK) + boottime;
    struct tm * start_time = gmtime(&startime);
    // size_t startn = 
    time_struct_to_string(infostuct->start_str, 10, start_time);
    time_t usetime = (utime + stime)/sysconf (_SC_CLK_TCK);
    struct tm * use_time = gmtime(&usetime);
    // size_t usen = 
    execution_time_to_string(infostuct->time_str, 10, use_time->tm_min, use_time->tm_sec);
    fclose(stat);
    return infostuct;
}
void destroy_info(process_info * infostruct) {
    free(infostruct->command);
    free(infostruct->start_str);
    free(infostruct->time_str);
    free(infostruct);
}