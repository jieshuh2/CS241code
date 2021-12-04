/**
 * charming_chatroom
 * CS 241 - Spring 2021
 */
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "utils.h"
#include <unistd.h>
static const size_t MESSAGE_SIZE_DIGITS = 4;

char *create_message(char *name, char *message) {
    int name_len = strlen(name);
    int msg_len = strlen(message);
    char *msg = calloc(1, msg_len + name_len + 4);
    sprintf(msg, "%s: %s", name, message);

    return msg;
}

ssize_t get_message_size(int socket) {
    int32_t size;
    ssize_t read_bytes =
        read_all_from_socket(socket, (char *)&size, MESSAGE_SIZE_DIGITS);
    if (read_bytes == 0 || read_bytes == -1)
        return read_bytes;

    return (ssize_t)ntohl(size);
}

// You may assume size won't be larger than a 4 byte integer
ssize_t write_message_size(size_t size, int socket) {
    // Your code here
    int32_t netsize = htonl(size);
    ssize_t write_bytes = write_all_to_socket(socket, (char*)&netsize, MESSAGE_SIZE_DIGITS);
    // if (write_bytes == 0 || write_bytes == -1) {
    //     return write_bytes;
    // }
    return write_bytes;
}

ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
    // Your Code Here
    int bytesread = 0;
    while(bytesread < (int)count) {
        int r = read(socket, buffer + bytesread, count - bytesread);
        if (r == 0) {
            return bytesread;
        } else if (r > 0) {
            bytesread += r;
        } else if (r == -1 && errno == EINTR) {
            continue;
        } else {
            return -1;
        }
    }
    return bytesread;
}

ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
    // Your Code Here
    int byteswrite = 0;
    while(byteswrite < (int)count) {
        int r = write(socket, buffer + byteswrite, count - byteswrite);
        if (r == 0) {
            return byteswrite;
        } else if (r > 0) {
            byteswrite += r;
        } else if (r == -1 && errno == EINTR) {
            continue;
        } else {
            return -1;
        }
    }
    return byteswrite;
}
