/**
 * nonstop_networking
 * CS 241 - Spring 2021
 */
#include "common.h"

#define GETLINE_MINSIZE 16
ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
    // Your Code Here
    int byteswrite = 0;
    while(byteswrite < (int)count) {
        int r = write(socket, buffer + byteswrite, count - byteswrite);
        if (r == 0) {
            return byteswrite;
        } else if (r > 0) {
            byteswrite += r;
        } else if (r == -1 && (errno == EINTR)) {
            continue;
        } else if (r == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            return byteswrite;
        }  else {
            return -1;
        }
    }
    return byteswrite;
}
ssize_t write_to_file(int src, int dst){
    char* buffer = calloc(1, 1024);
    ssize_t bytes_read = 0;
    while(1){
        ssize_t count = read_all_from_socket(src, buffer, 1024);
        if(count == 0){
            free(buffer);
            return bytes_read;
        }
        else if(count < 0){
            break;
        }
        bytes_read += count;
        
        write_all_to_socket(dst, buffer, count);
        
        memset(buffer, 0, 1024);
    }
    free(buffer);
    return -1;
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
        } else if (r == -1 && (errno == EINTR)) {
            continue;
        } else if (r == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            LOG("read block");
            return bytesread;
        } else {
            LOG("ERROR");
            return -1;
        }
    }
    return bytesread;
}
//modified copy of getline
int mygetline(char **lineptr, size_t *n, int socket, ssize_t max) {
    char ch;
    size_t i = 0;
    char free_on_err = 0;
    char *p;

    errno = 0;
    if (lineptr == NULL || n == NULL) {
        errno = EINVAL;
        return -1;
    }
    if (*lineptr == NULL) {
        *n = GETLINE_MINSIZE;
        *lineptr = (char *)malloc( sizeof(char) * (*n));
        if (*lineptr == NULL) {
            errno = ENOMEM;
            return -1;
        }
        free_on_err = 1;
    }

    for (i=0; ; i++) {
        ssize_t numread = read_all_from_socket(socket, &ch, 1);
        while (i >= (*n) - 2) {
            *n *= 2;
            p = realloc(*lineptr, sizeof(char) * (*n));
            if (p == NULL) {
                if (free_on_err)
                    free(*lineptr);
                errno = ENOMEM;
                return -1;
            }
            *lineptr = p;
        }
        if (max > 0 && i > (size_t)max) {
            //this is used for gurantee that there is a \n
            LOG("FAIL TO READLINE")
            if (free_on_err) {
                free(*lineptr);
                *lineptr = NULL;
                return -1;
            }
        }
        if (numread < 1) {
            if (i == 0) {
                if (free_on_err)
                    free(*lineptr);
                return -1;
            }
            (*lineptr)[i] = '\0';
            *n = i;
            return i;
        }

        if (ch == '\n') {
            (*lineptr)[i] = '\n';
            (*lineptr)[i+1] = '\0';
            *n = i+1;
            return i+1;
        }
        (*lineptr)[i] = ch;
    }
}
// ssize_t get_message_size(int socket) {
//     int32_t size;
//     ssize_t read_bytes =
//         read_all_from_socket(socket, (char *)&size, sizeof(size_t));
//     if (read_bytes == 0 || read_bytes == -1)
//         return read_bytes;

//     return (ssize_t)ntohl(size);
// }

// // You may assume size won't be larger than a 4 byte integer
// ssize_t write_message_size(size_t size, int socket) {
//     // Your code here
//     int32_t netsize = htonl(size);
//     ssize_t write_bytes = write_all_to_socket(socket, (char*)&netsize, sizeof(size_t));
//     // if (write_bytes == 0 || write_bytes == -1) {
//     //     return write_bytes;
//     // }
//     return write_bytes;
// }