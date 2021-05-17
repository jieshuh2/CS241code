/**
 * nonstop_networking
 * CS 241 - Spring 2021
 */
#pragma once
#include <stddef.h>
#include <sys/types.h>

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>

#define LOG(...)                      \
    do {                              \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n");        \
    } while (0);

typedef enum { GET, PUT, DELETE, LIST, V_UNKNOWN } verb;
ssize_t write_all_to_socket(int socket, const char *buffer, size_t count);
ssize_t read_all_from_socket(int socket, char *buffer, size_t count);
int mygetline(char **lineptr, size_t *n, int socket, ssize_t max);
ssize_t write_to_file(int src, int dst);