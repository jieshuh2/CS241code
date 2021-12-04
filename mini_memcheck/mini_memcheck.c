/**
 * mini_memcheck
 * CS 241 - Spring 2021
 */
#include "mini_memcheck.h"
#include <stdio.h>
#include <string.h>

meta_data* head = NULL;
size_t total_memory_requested = 0;
size_t total_memory_freed = 0;
size_t invalid_addresses = 0;
void *mini_malloc(size_t request_size, const char *filename,
                  void *instruction) {
    // your code here
    setvbuf(stdout, NULL, _IONBF, 0);
    if (request_size == 0) {
        return NULL;
    }
    void* ptr = malloc(sizeof(meta_data) + request_size);
    if (ptr == NULL) {
        return NULL;
    }
    total_memory_requested += request_size;
    void*memory = ptr + sizeof(meta_data);
    
    meta_data* data = (meta_data*)ptr;
    data->filename = filename;
    data->instruction = instruction;
    data->request_size = request_size;
    if (head == NULL) {
        head = data;
        head->next = NULL;
    } else {
        data->next = head;
        head = data;
    }
    return memory;
}

void *mini_calloc(size_t num_elements, size_t element_size,
                  const char *filename, void *instruction) {
    // your code here
    if (num_elements == 0 || element_size == 0) {
        return NULL;
    }
    size_t size = num_elements* element_size;
    void* ptr = malloc(sizeof(meta_data) + size);
    if (ptr == NULL) {
        return NULL;
    }
    total_memory_requested += size;
    void*memory = ptr + sizeof(meta_data);
    char* memptr = (char*)memory;
    for (size_t i = 0; i < size; i++) {
        *(char*)memptr = 0;
        memptr++;
    }
    meta_data* data = (meta_data*)ptr;
    data->filename = filename;
    data->instruction = instruction;
    data->request_size = size;
    if (head == NULL) {
        head = data;
        head->next = NULL;
    } else {
        data->next = head;
        head = data;
    }
    return memory;
}

void *mini_realloc(void *payload, size_t request_size, const char *filename,
                   void *instruction) {
    // your code here
    if (payload == NULL && request_size == 0) {
        return NULL;
    }
    if (payload == NULL) {
        return mini_malloc(request_size, filename, instruction);
    } 
    if (request_size == 0) {
        mini_free(payload);
        return NULL;
    }

    meta_data* ptr = head;
    meta_data* preptr = NULL;
    int find = 0;
    while(ptr != NULL) {
        if ((char*)ptr + sizeof(meta_data) == payload) {
            find = 1;
            break;
        }
        if (preptr == NULL) {
            preptr = head;
            ptr = ptr->next;
            continue;
        }
        ptr = ptr->next;
        preptr = preptr->next;
    }
    if (find == 0) {
        invalid_addresses += 1;
        return NULL;
    }
    if (ptr->request_size == request_size) {
        return payload;
    } 
    if (ptr->request_size < request_size) {
        total_memory_requested += request_size - ptr->request_size;
    } else {
        total_memory_freed += ptr->request_size - request_size;
    }
    if (ptr == head) {
        head = head->next;
        free((void*)ptr);
    } else {
        preptr->next = ptr->next;
        free((void*)ptr);
    }
    //malloc
    void* newptr = malloc(sizeof(meta_data) + request_size);
    if (ptr == NULL) {
        return NULL;
    }
    void*memory = newptr + sizeof(meta_data);
    
    meta_data* data = (meta_data*)newptr;
    data->filename = filename;
    data->instruction = instruction;
    data->request_size = request_size;
    if (head == NULL) {
        head = data;
        head->next = NULL;
    } else {
        data->next = head;
        head = data;
    }
    return memory;
}

void mini_free(void *payload) {
    // your code here
    if (payload == NULL) {
        return;
    }
    meta_data* ptr = head;
    meta_data* preptr = NULL;
    setvbuf(stdout, NULL, _IONBF, 0);
    int find = 0;
    while(ptr != NULL) {
        if ((char*)ptr + sizeof(meta_data) == payload) {
            find = 1;
            break;
        }
        if (preptr == NULL) {
            preptr = head;
            ptr = ptr->next;
            continue;
        }
        ptr = ptr->next;
        preptr = preptr->next;
    }
    if (find == 0) {
        invalid_addresses += 1;
        return;
    }
    if (ptr == head) {
        head = head->next;
        total_memory_freed += ptr->request_size;
        free((void*)ptr);
    } else {
        preptr->next = ptr->next;
        total_memory_freed += ptr->request_size;
        free((void*)ptr);
    }
}
