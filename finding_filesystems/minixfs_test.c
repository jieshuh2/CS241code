/**
 * finding_filesystems
 * CS 241 - Spring 2021
 */
#include "minixfs.h"
#include "minixfs_utils.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    // Write tests here!
    // char * buffer = calloc(1, 13);
    file_system *fs = open_fs("test.fs");
    off_t off = 0;
    char buf[13];
    ssize_t bytes_read = minixfs_read(fs, "/goodies/hello.txt", buf, 180, &off);
    // char *expected[13];
    // off = 0;
    // bytes_read = minixfs_write(fs, "/goodies/m.txt", buf, 13, &off);
    printf("bytes_read, %zd", bytes_read);
    printf("%s", buf);
    
    //open /goodies/hello.txt with open() or fopen() and read the contents way you normally would
    // assert(!strcmp(buf, expected));
    close_fs(&fs);
}
