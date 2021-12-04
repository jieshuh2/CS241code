/**
 * vector
 * CS 241 - Spring 2021
 */
#include "vector.h"
#include <stdio.h>
int main(int argc, char *argv[]) {
    // Write your test cases here
    vector * vec = char_vector_create();
    char v0 = 'a';
    vector_push_back(vec, &v0);
    vector_clear(vec);
    vector_push_back(vec, &v0);
    for (size_t i = 0; i < vector_size(vec); i++) {
        char string = *((char*)vector_get(vec, i));
        printf("%c", string);
    }
    printf("\n");
    vector_reserve(vec, 10);
    vector_resize(vec, 20);
    for (size_t i = 0; i < vector_size(vec); i++) {
        char string = *((char*)vector_get(vec, i));
        printf("%c", string);
    }
    printf("\n");
    vector_resize(vec, 5);
    char v3 = 'b';
    vector_insert(vec, 2, &v3);
    printf("%zu\n", vector_size(vec));
    vector_erase(vec, 2);
    printf("%zu\n", vector_size(vec));
    for (size_t i = 0; i < vector_size(vec); i++) {
        char string = *((char*)vector_get(vec, i));
        printf("%c", string);
    }
    printf("\n");
    vector_destroy(vec);
    return 0;
}
