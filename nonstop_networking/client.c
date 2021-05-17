/**
 * nonstop_networking
 * CS 241 - Spring 2021
 */
#include "format.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>

#include "common.h"
int connect_to_server(const char *host, const char *port);
char **parse_args(int argc, char **argv);
verb check_args(char **args);
int main(int argc, char **argv) {
    // Good luck!
    // setvbuf(stdout, NULL, _IONBF, 0);
    char** arg = parse_args(argc, argv);
    verb ver = check_args(arg);
    char* host = arg[0];
    char* port = arg[1];
    char* remotefile = arg[3];
    char* localfile = arg[4];
    int socket = connect_to_server(host, port);
    LOG("CONNECT");
    if (ver == LIST) {
        write_all_to_socket(socket, "LIST\n", 5);
        if (shutdown(socket, SHUT_WR) != 0) {
            LOG("failtoshutdown WR");
        }
        char* line = NULL;
        size_t linesize = 0;
        int numread = mygetline(&line, &linesize, socket, -1);
        if (numread == -1) {
            print_invalid_response();
        } else if (strcmp(line, "OK\n") == 0) {
            size_t size;
            ssize_t read_bytes =read_all_from_socket(socket, (char *)&size, sizeof(size_t));
            if (read_bytes < (ssize_t)sizeof(size_t)) {
                print_invalid_response();
            } else {
                char buf[size + 1];
                buf[size] = '\0';
                // read_bytes = read_all_from_socket(socket, buf, size);
                // if(read_bytes != (ssize_t)size) {
                //     LOG("LIST FAIL");
                //     LOG("read bytes: %zd, size: %zu", read_bytes, size);
                // } else {
                //     printf("%s", buf);
                // }
                // char buf[1024];
                // LOG("%d, file", local);
                // while(1) {
                //     memset(buf, 0, 1024);
                //     ssize_t read_bytes = read_all_from_socket(socket, buf, 1024);
                //     if (read_bytes == 0) {
                //         break;
                //     }
                //     if (read_bytes == -1) {
                //         LOG("ERROR IN READ");
                //         break;
                //     }
                //     write(1, buf, read_bytes);
                // }
                ssize_t read_bytes = read_all_from_socket(socket, buf, size);
                if(read_bytes != (ssize_t)size) {
                    LOG("LIST FAIL");
                    // LOG("read bytes: %zd, size: %zu", read_bytes, size);
                } else {
                    printf("%s", buf);
                }
            }
        } else if (strcmp(line, "ERROR\n") == 0) {
            numread = mygetline(&line, &linesize, socket, -1);
            if (numread == -1) {
                print_invalid_response();
            } else {
                print_error_message(line);
            }
        } else {
            print_invalid_response();
        }
        free(line);
        line = NULL;
        // if (shutdown(socket, SHUT_RD) != 0) {
        //     LOG("failtoshutdown RD");
        // }
    }
    if (ver == GET) {
        size_t len = 3 + 1 + strlen(remotefile) + 1;
        char mes[len];
        memcpy(mes, "GET ", 4);
        memcpy(mes + 4, remotefile, strlen(remotefile));
        mes[len - 1] = '\n';
        write_all_to_socket(socket, mes, len);
        if (shutdown(socket, SHUT_WR) != 0) {
            LOG("failtoshutdown WR");
        }
        char* line = NULL;
        size_t linesize = 0;
        int numread = mygetline(&line, &linesize, socket, -1);
        if (numread == -1) {
            print_invalid_response();
        } else if (strcmp(line, "OK\n") == 0) {
            size_t size;
            ssize_t read_bytes = read_all_from_socket(socket, (char *)&size, sizeof(size_t));
            LOG("size read %zu", size);
            if (read_bytes < (ssize_t)sizeof(size_t)) {
                print_invalid_response();
            } else {
                size_t count = 0;
                char buf[1024];
                int local = open(localfile, O_CREAT|O_TRUNC|O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
                // LOG("%d, file", local);
                while(1) {
                    memset(buf, 0, 1024);
                    ssize_t read_bytes = read_all_from_socket(socket, buf, 1024);
                    if (read_bytes == 0) {
                        break;
                    }
                    if (read_bytes == -1) {
                        LOG("ERROR IN READ");
                        break;
                    }
                    count += read_bytes;
                    write_all_to_socket(local, buf, read_bytes);
                }
                LOG("bytes get %zu count", count);
                if (count < size) {
                    print_too_little_data();
                }
                if (count > size) {
                    print_received_too_much_data();
                }
                close(local);
            }
        } else if (strcmp(line, "ERROR\n") == 0) {
            // LOG("ERROR1");
            int errorbyte = mygetline(&line, &linesize, socket, -1);
            if (errorbyte == -1) {
                print_invalid_response();
            } else {
                print_error_message(line);
            }
        } else {
            LOG("ERROR");
            print_invalid_response();
        }
        free(line);
        line = NULL;
        // if (shutdown(socket, SHUT_RD) != 0) {
        //     LOG("failtoshutdown RD");
        // }
    }
    if (ver == PUT) {
        struct stat statbuf;
        if (stat(localfile, &statbuf) != 0) {
            LOG("NO FILE: %s", localfile);
            shutdown(socket, SHUT_RDWR);
            exit(1);
        } 
        int local = open(localfile, O_RDONLY);
        if (local == -1) {
            LOG("FAIL TO OPEN");
            shutdown(socket, SHUT_RDWR);
            exit(1);
        }
        size_t size = statbuf.st_size;
        size_t len = 3 + 1 + strlen(remotefile) + 1 + sizeof(size_t);
        char mes[len];
        memcpy(mes, "PUT ", 4);
        memcpy(mes + 4, remotefile, strlen(remotefile));
        mes[3 + 1 + strlen(remotefile)] = '\n';
        // LOG("%s", mes);
        memcpy(mes + 3 + 1 + strlen(remotefile) + 1, (char*)&size, sizeof(size_t));
        // LOG("%s", mes);
        LOG("size is %zu", size);
        write_all_to_socket(socket, mes, len);
        //starting sending file
        char buf[1024];
        ssize_t bytes_write = 0;
        while(1) {
            memset(buf, 0, 1024);
            ssize_t read_bytes = read_all_from_socket(local, buf, 1024);
            if (read_bytes == 0) {
                break;
            }
            if (read_bytes == -1) {
                // LOG("ERROR IN READ");
                perror(NULL);
                break;
            }
            bytes_write += write_all_to_socket(socket, buf, read_bytes);
        }
        close(local);
        LOG("%zu", bytes_write);
        if (shutdown(socket, SHUT_WR) != 0) {
            LOG("failtoshutdown WR");
            perror(NULL);
        }
        char* line = NULL;
        size_t linesize = 0;
        int numread = mygetline(&line, &linesize, socket, -1);
        if (numread == -1) {
            print_invalid_response();
        } else if (strcmp(line, "OK\n") == 0) {
            print_success();
        } else if (strcmp(line, "ERROR\n") == 0) {
            numread = mygetline(&line, &linesize, socket, -1);
            if (numread == -1) {
                print_invalid_response();
            } else {
                print_error_message(line);
            }
        } else {
            print_invalid_response();
        }
        free(line);
        line = NULL;
        // if (shutdown(socket, SHUT_RD) != 0) {
        //     LOG("failtoshutdown RD");
        // }
    }
    if (ver == DELETE) {
        size_t len = 6 + 1 + strlen(remotefile) + 1;
        char mes[len];
        memcpy(mes, "DELETE ", 7);
        memcpy(mes + 7, remotefile, strlen(remotefile));
        mes[len - 1] = '\n';
        LOG("%s", mes);
        write_all_to_socket(socket, mes, len);
        if (shutdown(socket, SHUT_WR) != 0) {
            LOG("failtoshutdown WR");
        }
        char* line = NULL;
        size_t linesize = 0;
        int numread = mygetline(&line, &linesize, socket, -1);
        if (numread == -1) {
            print_invalid_response();
        } else if (strcmp(line, "OK\n") == 0) {
            print_success();
        } else if (strcmp(line, "ERROR\n") == 0) {
            numread = mygetline(&line, &linesize, socket, -1);
            LOG("FAIL TO DELETE %s", remotefile);
            if (numread == -1) {
                print_invalid_response();
            } else {
                print_error_message(line);
            }
        } else {
            print_invalid_response();
        }
        free(line);
        line = NULL;
        // if (shutdown(socket, SHUT_RD) != 0) {
        //     LOG("failtoshutdown RD");
        // }
    }
    print_connection_closed();
    close(socket);
    free(arg);
}

/**
 * Given commandline argc and argv, parses argv.
 *
 * argc argc from main()
 * argv argv from main()
 *
 * Returns char* array in form of {host, port, method, remote, local, NULL}
 * where `method` is ALL CAPS
 */
char **parse_args(int argc, char **argv) {
    if (argc < 3) {
        return NULL;
    }

    char *host = strtok(argv[1], ":");
    char *port = strtok(NULL, ":");
    if (port == NULL) {
        return NULL;
    }

    char **args = calloc(1, 6 * sizeof(char *));
    args[0] = host;
    args[1] = port;
    args[2] = argv[2];
    char *temp = args[2];
    while (*temp) {
        *temp = toupper((unsigned char)*temp);
        temp++;
    }
    if (argc > 3) {
        args[3] = argv[3];
    }
    if (argc > 4) {
        args[4] = argv[4];
    }

    return args;
}

/**
 * Validates args to program.  If `args` are not valid, help information for the
 * program is printed.
 *
 * args     arguments to parse
 *
 * Returns a verb which corresponds to the request method
 */
verb check_args(char **args) {
    if (args == NULL) {
        print_client_usage();
        exit(1);
    }

    char *command = args[2];

    if (strcmp(command, "LIST") == 0) {
        return LIST;
    }

    if (strcmp(command, "GET") == 0) {
        if (args[3] != NULL && args[4] != NULL) {
            return GET;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "DELETE") == 0) {
        if (args[3] != NULL) {
            return DELETE;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "PUT") == 0) {
        if (args[3] == NULL || args[4] == NULL) {
            print_client_help();
            exit(1);
        }
        return PUT;
    }

    // Not a valid Method
    print_client_help();
    exit(1);
}

int connect_to_server(const char *host, const char *port) {
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; 
    hints.ai_socktype = SOCK_STREAM; 
    int s = getaddrinfo(host, port, &hints, &result);
    if(s != 0) {
        fprintf(stderr,"%s, \n", gai_strerror(s));
        exit(1);
    }
    int sock =  socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    int ok = connect(sock, result->ai_addr, result->ai_addrlen);
    if (ok == -1) {
        LOG("failtoconntect");
        freeaddrinfo(result);
        exit(1);
    }
    freeaddrinfo(result);
    return sock;
}


