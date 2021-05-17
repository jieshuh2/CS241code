/**
 * nonstop_networking
 * CS 241 - Spring 2021
 */
//use epoll structure in https://github.com/mikedilger/epoll_demo/commit/969691ef479b540c727d2d2ffb280b22029754e1
#include <stdio.h>
#include "format.h"
#include "common.h"
#include "includes/dictionary.h"
#include "includes/vector.h"
#include <signal.h>
#include <sys/epoll.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
static vector* files;
static dictionary* clients;
static int endsession;
static char* directory;
static int serverSocket;
static int epollfd;
int readformserver(int socket);
int deletefiles();

void handler() {
    endsession = 1;
    LOG("handler");
}
void cleanup() {
    deletefiles();
    vector_destroy(files);
    dictionary_destroy(clients);
    int successrm = rmdir(directory);
    if (successrm != 0) {
        perror("fail to rmdir:");
    }
    // shutdown(serverSocket, SHUT_RDWR);
    close(epollfd);
    close(serverSocket);
}

typedef struct client{
    int state;
    int request;
    char* filename;
    size_t filesize;
    size_t bytesfin;
}client;

int deletefiles() {
    for (size_t i = 0; i < vector_size(files); i++) {
        char* name= vector_get(files, i);
        char path[strlen(name) + strlen(directory) + 2];
        strcpy(path, directory);
        path[strlen(directory)] = '/';
        strcpy(path + strlen(directory) + 1, name);
        path[strlen(name) + strlen(directory) + 1] = '\0';
        int delete = unlink(path);
        if (delete != 0) {
            LOG("fail to delete: %s", name);
        }
    }
    return 0;
}
int main(int argc, char **argv) {
    // good luck!
    
    if (argc < 2) {
        print_server_usage();
        exit(1);
    }
    char template[7] = "XXXXXX";
    directory = mkdtemp(template);
    print_temp_directory(directory);

    endsession = 0;
    struct sigaction act;
    memset(&act, '\0', sizeof(act));
    act.sa_handler = handler;
    sigaddset(&act.sa_mask, SIGINT);
    if (sigaction(SIGINT, &act, NULL) < 0) {
        perror("sigaction");
        return 1;
    }
    signal(SIGPIPE, SIG_IGN);

    clients = int_to_shallow_dictionary_create();
    files = string_vector_create();
    char* port = argv[1];
    serverSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    int flags = fcntl(serverSocket, F_GETFL, 0);
    fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK);
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // Only want IPv6 (use AF_INET for IPv4)
    hints.ai_socktype = SOCK_STREAM; 
    hints.ai_flags = AI_PASSIVE;
    int s = getaddrinfo(NULL, port, &hints, &result);
    if (s != 0) {
        fprintf(stderr,"%s, \n", gai_strerror(s));
        exit(1);
    }
    int optval = 1;
    int retval = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if(retval == -1) {
        freeaddrinfo(result);
        exit(1);
    }
    retval = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    if(retval == -1) {
        freeaddrinfo(result);
        exit(1);
    }
    if (bind(serverSocket, result->ai_addr, result->ai_addrlen) != 0) {
        freeaddrinfo(result);
        exit(1);
    }
    if (listen(serverSocket, SOMAXCONN) != 0) {
        freeaddrinfo(result);
        exit(1);
    }
    freeaddrinfo(result);
    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        exit(EXIT_FAILURE);
    }
    struct epoll_event ev, events[SOMAXCONN];
    memset(&ev, 0, sizeof(ev));
    ev.events =  EPOLLIN;//for reaading
    ev.data.fd = serverSocket;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, serverSocket, &ev) == -1) {
        exit(EXIT_FAILURE);
    }
    while(1) {
        int nfds = epoll_wait(epollfd, events, SOMAXCONN, -1);
        if (nfds == -1 || endsession == 1) {
            break;
        }
        for (int i = 0; i < nfds; i++) {
            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN))){
              /* An error has occured on this fd, or the socket is not
               * ready for reading (why were we notified then?) */
	            fprintf (stderr, "epoll error. events=%u\n", events[i].events);
	            close (events[i].data.fd);
	            continue;
	        }
            if (events[i].data.fd == serverSocket) {
                /* We have a notification on the listening socket, which
                 means one or more incoming connections. */
            while (1){
                struct sockaddr in_addr;
                socklen_t in_len;
                int conn_sock;

                in_len = sizeof in_addr;
                conn_sock= accept (serverSocket, &in_addr, &in_len);
                if (conn_sock == -1){
                    if ((errno == EAGAIN) || (errno == EWOULDBLOCK)){
                        /* We have processed all incoming
                            connections. */
                        break;
                    } else{
                        perror ("accept");
                        break;
                    }
                }
                  /* Make the incoming socket non-blocking and add it to the
                     list of fds to monitor. */
                int flags = fcntl(conn_sock, F_GETFL, 0);
                fcntl(conn_sock, F_SETFL, flags | O_NONBLOCK);

                ev.data.fd = conn_sock;
                ev.events = EPOLLIN | EPOLLET;
                s = epoll_ctl (epollfd, EPOLL_CTL_ADD, conn_sock, &ev);
                if (s == -1){
                    perror ("epoll_ctl");
                    abort ();
                }
                }
                // int conn_sock = accept(serverSocket, NULL, NULL);
                // // Must set to non-blocking
                // int flags = fcntl(conn_sock, F_GETFL, 0);
                // fcntl(conn_sock, F_SETFL, flags | O_NONBLOCK);
                //  // We will read from this file, and we only want to return once
                // // we have something to read from. We don't want to keep getting
                // // reminded if there is still data left (edge triggered)
                // ev.events = EPOLLIN | EPOLLET;
                // ev.data.fd = conn_sock;
                // epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev);
               
            } else {
                int socket = events[i].data.fd;
                 if (!dictionary_contains(clients, &socket)) {
                    client * state = malloc(sizeof(client));
                    memset(state, 0, sizeof(client));
                    state->request = -1;
                    state->state = 0;
                    state->filename = NULL;
                    dictionary_set(clients, &socket, state);
                } 
                int fin = readformserver(socket);
                if (fin == -1) {
                    client* state = dictionary_get(clients, &socket);    
                    free(state->filename);
                    free(state);
                    // shutdown(socket, SHUT_RDWR);
                    LOG("finish client");
                    close(socket);
                    dictionary_remove(clients, &socket);
                }
            }
        }
    }
    LOG("close server");
    cleanup();
    return 0;
    
}
int readformserver(int socket) {
    client* state = dictionary_get(clients, &socket);
    if (state->state == 0) {
        char* line = NULL;
        size_t linesize;
        int numread = mygetline(&line, &linesize, socket, (ssize_t)(FILENAME_MAX + 1 + 1 + 6));
        if (numread == -1) {
            //invalid
            print_invalid_response();
            write_all_to_socket(socket, "ERROR\n", 6);
            write_all_to_socket(socket, err_bad_request, strlen(err_bad_request));
            return -1;
        }
        if (strcmp(line, "LIST\n") == 0) {
            //finish reading
            state->request = LIST;
            char* list = calloc(1, 1024);
            size_t buffer_size = 1024;
            for(size_t i = 0; i < vector_size(files); i++){
                char* filename = (char*)vector_get(files, i);
                if(strlen(list) + strlen(filename) >= buffer_size){
                    list = (char*)realloc(list, 2 * buffer_size);
                    buffer_size *= 2;
                }
                strcat(list, filename);
                strcat(list, "\n");
            }
            if(strlen(list) != 0){
                *(list + strlen(list) - 1) = 0;
            }
            size_t length = strlen(list);
            write_all_to_socket(socket, "OK\n", 3);
            write_all_to_socket(socket, (char*)&length, sizeof(size_t));
            write_all_to_socket(socket, list, length);
            free(line);
            free(list);
            // write_all_to_socket(socket, "OK\n", 3);
            // size_t len = 0;
            // for (size_t i = 0; i < vector_size(files); i++) {
            //     char* name= vector_get(files, i);
            //     size_t namelen = strlen(name);
            //     len += namelen;
            // }
            // len += vector_size(files) - 1;
            // LOG("%zu", len);
            // write_all_to_socket(socket, (char*)&len, sizeof(size_t));
            // for (size_t i = 0; i < vector_size(files); i++) {
            //     char* name= vector_get(files, i);
            //     size_t namelen = strlen(name);
            //     write_all_to_socket(socket, name, namelen);
            //     if (i != vector_size(files) - 1) {
            //         write_all_to_socket(socket, "\n", 1);
            //     }
            // }
            // free(line);
            return -1;



        } else if (strncmp(line, "PUT ", 4) == 0) {
            // LOG("Here2");
            state->request = PUT;
            char* filename = line + 4;
            size_t filenamelen = strlen(filename);
            if (filename[filenamelen - 1] != '\n') {
                free(line);
                LOG("badfilename");
                return -1;
            } else {
                filename[filenamelen - 1] = '\0';
                state->state = 1;
            }
            ssize_t bytesread = read_all_from_socket(socket, (char*)(&(state->filesize)), sizeof(size_t));
            if (bytesread < 8) {
                LOG("fail to get size");
                free(line);
                return -1;
            }
            // LOG("filesize:%s", state->filesize);
            // state->filesize = (size_t*)state->size;
            LOG("size:%zu", state->filesize);
            state->bytesfin = 0;
            char path[strlen(filename) + strlen(directory) + 2];
            strcpy(path, directory);
            path[strlen(directory)] = '/';
            strcpy(path + strlen(directory) + 1, filename);
            path[strlen(filename) + strlen(directory) + 1] = '\0';
            struct stat file_stat;
            if(stat(path, &file_stat) == -1){
                vector_push_back(files, filename);
            }
            state->filename = strdup(path);
            // LOG("fd: %d", state->file);
            LOG("%s", path);
            free(line);

        } else if (strncmp(line, "GET ", 4) == 0) {
            state->request = GET;
            char* filename = line + 4;
            size_t filenamelen = strlen(filename);
            if (filename[filenamelen - 1] != '\n') {
                print_invalid_response();
                write_all_to_socket(socket, "ERROR\n", 6);
                write_all_to_socket(socket, err_bad_request, strlen(err_bad_request));
                free(line);
                return -1;
            } else {
                filename[filenamelen - 1] = '\0';
                state->state = 1;
            }
            char path[strlen(filename) + strlen(directory) + 2];
            strcpy(path, directory);
            path[strlen(directory)] = '/';
            strcpy(path + strlen(directory) + 1, filename);
            path[strlen(filename) + strlen(directory) + 1] = '\0';
            LOG("%s", path);
            struct stat filedata;
            if (stat(path, &filedata) != 0) {
                write_all_to_socket(socket, "ERROR\n", 6);
                write_all_to_socket(socket, err_no_such_file, strlen(err_no_such_file));
                free(line);
                LOG("NOFILE");
                return -1;
            } else {
                write_all_to_socket(socket, "OK\n", 3);
            }
            state->filesize = filedata.st_size;
            state->bytesfin = 0;
            LOG("%zu", state->filesize);
            write_all_to_socket(socket, (char*)(&(state->filesize)), sizeof(size_t));
            FILE* get_file = fopen(path, "r");
            write_to_file(fileno(get_file), socket);
            fclose(get_file);
            state->state = -1;
            free(line);
            return -1;

        } else if (strncmp(line, "DELETE ", 7) == 0) {
            state->request = DELETE;
            char* filename = line + 7;
            size_t filenamelen = strlen(filename);
            if (filename[filenamelen - 1] != '\n') {
                print_invalid_response();
                write_all_to_socket(socket, "ERROR\n", 6);
                write_all_to_socket(socket, err_bad_request, strlen(err_bad_request));
                free(line);
                return -1;
            } else {
                filename[filenamelen - 1] = '\0';
            }
            LOG("to delete is (%s)", filename);
            int find = 0;
            for (size_t i = 0; i < vector_size(files); i++) {
                char* filei = vector_get(files, i);
                LOG("filei: (%s)", filei);
                if (strcmp(filei, filename) == 0) {
                    LOG("here");
                    find = 1;
                    vector_erase(files, i);
                    char path[strlen(filename) + strlen(directory) + 2];
                    strcpy(path, directory);
                    path[strlen(directory)] = '/';
                    strcpy(path + strlen(directory) + 1, filename);
                    path[strlen(filename) + strlen(directory) + 1] = '\0';
                    int delete = unlink(path);
                    if (delete != 0) {
                        LOG("fail to delete: %s", path);
                    }
                    break;
                }
            }
            if (find) {
                write_all_to_socket(socket, "OK\n", 3);
            } else {
                write_all_to_socket(socket, "ERROR\n", 6);
                write_all_to_socket(socket, err_no_such_file, strlen(err_no_such_file));
            }
            free(line);
            return -1;
        } else {
            //invalid
            write_all_to_socket(socket, "ERROR\n", 6);
            write_all_to_socket(socket, err_bad_request, 13);
            print_invalid_response();
            free(line);
            return -1;
        }
    }
    if (state->request == PUT) {
        int file;
        if (state->state == 1) {
            file = open(state->filename, O_CREAT|O_TRUNC|O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
        } else if (state->state == 2) {
            file = open(state->filename, O_APPEND|O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
        }
        LOG("start get info");
        char* buf = calloc(1024, 1);
        // while(1) {
        //     memset(buf, 0, 1024);
        //     ssize_t read_bytes = read_all_from_socket(socket, buf, 1024);
        //     if (read_bytes == 0) {
        //         LOG("bytes finish %zu", state->bytesfin);
        //         if (state->bytesfin != state->filesize) {
        //             write_all_to_socket(socket, "ERROR\n", 6);
        //             write_all_to_socket(socket, err_bad_file_size, strlen(err_bad_file_size));
        //         } else {
        //             write_all_to_socket(socket, "OK\n", 3);
        //         }
        //         close(file);
        //         free(buf);
        //         return -1;
        //     }
        //     if (read_bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        //         LOG("bytes half %zu", state->bytesfin);
        //         LOG("NOT FINISH");
        //         free(buf);
        //         return 0;
        //     } else if (read_bytes == -1) {
        //         LOG("bytes error %zu", state->bytesfin);
        //         perror("put:");
        //         close(state->file);
        //         free(buf);
        //         return -1;
        //     }
        //     state->bytesfin += read_bytes;
        //     write_all_to_socket(state->file, buf, read_bytes);
        // }
        int fin = 0;
        while (1){
        ssize_t count;
        count = read (socket, buf, 1024);
            if (count == -1){
                if (errno == EINTR) {
                    continue;
                }
                if (errno != EAGAIN){
                    perror ("read");
                    fin = 1;
                }
                break;
            }
            else if (count == 0){
                fin = 1;
                break;
            }
            state->bytesfin += count;
            write_all_to_socket(file, buf, count);
        
        }
        free(buf);
        if (fin) {
            LOG("bytes finish %zu", state->bytesfin);
            if (state->bytesfin != state->filesize) {
                write_all_to_socket(socket, "ERROR\n", 6);
                write_all_to_socket(socket, err_bad_file_size, strlen(err_bad_file_size));
            } else {
                write_all_to_socket(socket, "OK\n", 3);
            }
            state->state = -1;
            close(file);
            return -1;
        }
        close(file);
        state->state = 2;
        return 0;
        
        
    // } else if (state->request == GET) {
    //     char* buf = calloc(1024, 1);
    //     while(1) {
    //         memset(buf, 0, 1024);
    //         ssize_t read_bytes = read_all_from_socket(state->file, buf, 1024);
    //         if (read_bytes == 0) {
    //             close(state->file);
    //             free(buf);
    //             return -1;
    //         }
    //         ssize_t bytese_write = write_all_to_socket(socket, buf, read_bytes);
    //         if (bytese_write == -1) {
    //             //errno == EPIPE
    //             LOG("OTHER ERROR");
    //             close(state->file);
    //             free(buf);
    //             return -1;
    //         }
    //         state->bytesfin += bytese_write;
    //     }
    } else {
        LOG("UNEXPECTED");
        exit(1);
    }
    return 0;
}