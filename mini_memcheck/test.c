/**
 * mini_memcheck
 * CS 241 - Spring 2021
 */
#include <stdio.h>
#include <stdlib.h>

int main() {
    void* p1 = malloc(10);
    void *p2 = malloc(40);
    void* p4 = realloc(p2, 60);
    free(p4);
    free(p1);
    return 0;
}