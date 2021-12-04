/**
 * vector
 * CS 241 - Spring 2021
 */
#include "sstring.h"
#include "vector.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <string.h>

struct sstring {
    // Anything you want
    vector * vec;
};

sstring *cstr_to_sstring(const char *input) {
    // your code goes here
    sstring * str = (sstring*)malloc(sizeof(sstring));
    str->vec = char_vector_create();
    while(*input != '\0') {
        vector_push_back(str->vec, (void*)input);
        input++;
    }
    return str;
}

char *sstring_to_cstr(sstring *input) {
    // your code goes here
    char array [vector_size(input->vec) + 1];
    char* str = (char*)malloc(sizeof(char) * (vector_size(input->vec) + 1));
    for (size_t i = 0; i < vector_size(input->vec); i++) {
        array[i] = *((char*)(vector_get(input->vec, i)));
    }
    array[vector_size(input->vec)] = '\0';
    strcpy(str, array);
    return str;
}

int sstring_append(sstring *this, sstring *addition) {
    // your code goes here
    vector_reserve(this->vec, vector_size(this->vec)  + vector_size(addition->vec));
    for (size_t i = 0; i < vector_size(addition->vec); i++) {
        vector_push_back(this->vec, vector_get(addition->vec, i));
    }
    return vector_size(this->vec);
}

vector *sstring_split(sstring *this, char delimiter) {
    // your code goes here
    printf("split result:/");
    vector * strvec = string_vector_create();
    size_t start = 0;
    for (size_t i = 0; i < vector_size(this->vec); i++) {
        if (*((char*)vector_get(this->vec, i)) == delimiter) {
            char* str = sstring_slice(this, start, i);
            printf("%s/", str);
            vector_push_back(strvec, (void*)str);
            start = i + 1;
        }
    }
    char* str = sstring_slice(this, start, vector_size(this->vec));
    printf("%s/end", str);
    vector_push_back(strvec, str);
    return strvec;
}

int sstring_substitute(sstring *this, size_t offset, char *target,
                       char *substitution) {
    // your code goes here
    size_t start = vector_size(this->vec);
    bool found = false;
    for (size_t i = offset; i < vector_size(this->vec); i++) {
        if (*((char*)vector_get(this->vec, i)) == target[0]) {
            start = i;
            found = true;
            for (size_t j = 0; j < strlen(target); j++) {
                if (*((char*)vector_get(this->vec, i + j)) != target[j]) {
                    found = false;
                    break;
                }
            }
            if (found == true) {
                break;
            }
        }
    }
    if (!found) {
        return -1;
    } else {
        for (size_t i = 0; i < strlen(target); i++) {
            vector_erase(this->vec, start);
        }
        for (size_t i = 0; i < strlen(substitution); i++) {
            vector_insert(this->vec, start, (void*)(substitution + strlen(substitution) - i - 1));
        }
    }
    return 0;
}

char *sstring_slice(sstring *this, int start, int end) {
    // your code goes here
    assert(start <= end);
    assert((size_t)end <= vector_size(this->vec));
    assert(start >= 0);
    char* str = (char*)malloc(sizeof(char) * (end - start + 1));
    char*ptr = str;
    for (size_t i = (size_t)start; i < (size_t)end; i++) {
        *ptr = *((char*)(vector_get(this->vec, i)));
        ptr++;
    }
    *ptr = '\0';
    return str;
}

void sstring_destroy(sstring *this) {
    // your code goes here
    vector_destroy(this->vec);
    free(this);
}
