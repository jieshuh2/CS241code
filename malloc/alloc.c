/**
 * malloc
 * CS 241 - Spring 2021
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <limits.h>
typedef struct entry {
    // void* endd;
    size_t size;
    // size_t size_used;
    int free;//0: inuse, 1: available
    //used for free list
    struct entry * next;
    struct entry* prev;
}entry;
typedef struct end {
    size_t size;
}end;
static entry* head = NULL;//head of the freed memory
static entry* start;//start address of heap
static entry* final;//final address of heap
static size_t TRESHOLD = 1024;
/**
 * Allocate space for array in memory
 *
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size) {
    // implement calloc!
    void* result = malloc(num*size);
    if (!result) return NULL;
    
  // If we're using new memory pages
  // allocated from the system by calling sbrk
  // then they will be zero so zero-ing out is unnecessary,
  // We will be non-robust and memset either way.
    return memset(result, 0, num*size);
}

/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */
void *malloc(size_t size) {
    // implement malloc!
    if (size == 0) return NULL;
    //adjust the size of the block to be 16
    size_t total = (size + sizeof(entry) + sizeof(end) + 15) / 16;
    total *= 16;
    size = total - sizeof(entry) - sizeof(end);
    entry* entr = NULL;
    //check one in free list
    entry* ptr = head;
    while(ptr != NULL) {
        if (ptr->size >= size) {
            entr = ptr;
            break;
        }
        ptr = ptr->next;
    }
    if (entr == NULL) {
        entr = sbrk(sizeof(entry) + size + sizeof(end));
        if (entr == (void*) -1) {
            return NULL;
        }
        if (start == NULL) {
            start = entr;
        }
        final = entr;
        // setend to be NULL;
        entr->size = size;
        entr->free = 0;
        entr->next = NULL;
        entr->prev = NULL;
        end* thisend = (void*) entr + sizeof(entry) + entr->size;
        thisend->size = entr->size;
        return (void*)entr + sizeof(entry);
    }
    if (entr) {
        entr->free = 0;
        entry* previous = entr->prev;
        entry* next = entr->next;
        if (entr->size > size && (entr->size  >= TRESHOLD + size + sizeof(entry) + sizeof(end))) {
            //too much waste we want to divide it here、
            if (sbrk(0) < (void*)entr + sizeof(entry) + sizeof(end) + entr->size) {
                printf("138:end: %p, entr: %p, final: %p", sbrk(0), entr, final);
                exit(11);
            }   
            size_t oldsize = entr->size;
            entr->size = size;
            end* thisend = (void*)entr + size + sizeof(entry);
            // thisend->free = 0;
            thisend->size = size;
            entr->prev = NULL;
            entr->next = NULL;
            entry* newentry = (void*)entr + entr->size + sizeof(end) + sizeof(entry);
            newentry->free = 1;
            newentry->size = oldsize - size - sizeof(entry) - sizeof(end);
            newentry->prev = NULL;
            newentry->next = NULL;
            end* newend = (void*) newentry + newentry->size + sizeof(entry);
            // newend->free = 1;
            newend->size = newentry->size;
            if (final == entr) {
                final = newentry;
            }
            if (entr == head) {
                head = newentry;
            }
            newentry->prev = previous;
            newentry->next = next;
            if (previous != NULL) {
                previous->next = newentry;
            }
            if (next != NULL) {
                next->prev = newentry;
            }
            return (void*)entr + sizeof(entry);
        }
        if (entr == head) {
            head = entr->next;
        }
        if (previous != NULL) {
            previous->next = next;
        }
        if (next != NULL) {
            next->prev = previous;
        }
        entr->prev = NULL;
        entr->next = NULL;
        return (void*)entr + sizeof(entry);
    }
    return NULL;
}

/**
 * Deallocate space in memory
 *
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */
void free(void *ptr) {
    // implement free!
    if (ptr == NULL) {
        return;
    }
    assert(ptr <= (void*)final + sizeof(entry) && ptr >= (void*)start + sizeof(entry));
    entry* entr = (void*)ptr - sizeof(entry);
    assert(entr->free == 0);

    entr ->free = 1;
    entry* previous = NULL;
    entry* next = NULL;
    if (entr > start) {
        end* prevend = (void*)entr - sizeof(end);
        previous = (void*)prevend - prevend->size - sizeof(entry);
    }
    if (entr < final) {
        next = (void*)entr + sizeof(entry) + entr->size + sizeof(end);
    }
    if ((previous != NULL && previous->free) && (next == NULL || !next->free)) {
        //entr is deleted
        previous->size = previous->size + sizeof(end) + sizeof(entry) + entr->size;
        end* endd = (void*) previous + previous->size + sizeof(entry);
        endd->size = previous->size;
        if (final == entr) {
            final = previous;
        }
    } else if ((previous == NULL || !previous->free) && (next != NULL && next->free)) {
        //next is deleted
        if (head == next) {
            head = entr;
        }
        if (final == next) {
            final = entr;
        }
        entry* beforenext = next->prev;
        entry* afternext = next ->next;
        entr->size = entr->size + sizeof(end) + sizeof(entry) + next->size;
        end* nextend = (void*)entr + entr->size + sizeof(entry);
        nextend->size = entr->size;

        entr->prev = beforenext;
        entr->next = afternext;
        if (beforenext != NULL) {
            beforenext->next = entr;
        }
        if (afternext != NULL) {
            afternext->prev = entr;
        }
    } else if (previous != NULL && previous->free && next != NULL && next->free) {
        if (final == next) {
            final = previous;
        }
        // size_t origin = previous->size;
        previous->size = previous->size + 2*sizeof(end) + 2*sizeof(entry) + entr->size + next->size;
        end* nextend = (void*) previous + previous->size + sizeof(entry);
        nextend->size = previous->size;
        previous->next = next->next;
        entry* afternext = next->next;
        if (afternext != NULL) {
            afternext->prev = previous;
        }
        
    } else {
        //no deletion on block
        if (head == NULL) {
            head = entr;
            return;
        }
        entry* p = head;
        while (p -> next != NULL) {
            assert(p->free);
            if (p > entr) {
                break;
            }
            p = p->next;
        }
        if (p < entr) {
            p->next = entr;
            entr->prev = p;
            assert(entr->next == NULL);
            return;
        }
        entry* before = p->prev;
        if (p == head) {
            assert(p->prev == NULL);
            head = entr;
        }
        entr->prev = before;
        entr->next = p;
        if (before != NULL) {
            before->next = entr;
        }
        assert(p != NULL);
        p->prev = entr;
    }
    return;
}

/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size) {
    // implement realloc!
    if (ptr == NULL) {
        return malloc(size);
    }
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    size_t total = (size + sizeof(entry) + sizeof(end) + 15) / 16;
    total *= 16;
    size = total - sizeof(entry) - sizeof(end);
    entry * entr = (void*)ptr - sizeof(entry);
    assert(entr >= start && entr <= final);
    assert(!entr->free);
    void *result = malloc(size);
    if (result == NULL) {
            return NULL;
    } 
    if (ptr) memcpy(result, ptr, size < entr->size ? size : entr->size);
    free(ptr);
    return result;

    // if (entr->size < size) {
    //     void* result =  malloc(size);
    //     if (result == NULL) {
    //         return NULL;
    //     }
    //     memcpy(result, ptr, entr->size);
    //     free(ptr);
    //     return result;
    // }
    // if (entr->size > size && entr->size >= TRESHOLD + size + sizeof(entry) + sizeof(end)) {
    //     //too much waste we want to divide it here
    //     if (entr->size > INT_MAX) {
    //         printf("5size: %zu", entr->size);
    //         printf("p: %p", entr);
    //         printf("sbrk: %p", sbrk(0));
    //         exit(11);
    //     }
    //     size_t oldsize = entr->size;
    //     entr->size = size;
    //     entr->free = 0;
    //     end* thisend = (void*)entr + entr->size + sizeof(entry);
    //     thisend->size = size;
    //     //split a new block. Notice this is equal to add a new block, need to make sure final is pointing to the right block
    //     entry* newentry = (void*)entr + size + sizeof(end) + sizeof(entry);
    //     newentry->free = 1;
    //     assert(oldsize > size + sizeof(entry) + sizeof(end));
    //     newentry->size = oldsize - size - sizeof(entry) - sizeof(end);
    //     newentry->prev = NULL;
    //     newentry->next = NULL;
    //     end* newend = (void*) newentry + newentry->size + sizeof(entry);
    //     newend->size = newentry->size;
    //     if (entr == final) {
    //         final = newentry;
    //     }
    //     //have a enpty block after new entry, merge
    //     if (newentry < final) {
    //         entry* nextblock = (void*)newentry + sizeof(entry) + newentry->size + sizeof(end);
    //         if (nextblock->free) {
    //             if (nextblock == head) {
    //                 head = newentry;
    //             }
    //             if (nextblock == final) {
    //                 final = newentry;
    //             }
    //             entry* beforenext = nextblock->prev;
    //             entry* afternext = nextblock ->next;
    //             newentry->size = newentry->size + sizeof(end) + sizeof(entry) + nextblock->size;
    //             end* nextend = (void*) nextblock + nextblock->size + sizeof(entry);
    //             nextend->size = newentry->size;

    //             newentry->prev = beforenext;
    //             newentry->next = afternext;
    //             if (beforenext != NULL) {
    //                 beforenext->next = newentry;
    //             }
    //             if (afternext != NULL) {
    //                 afternext->prev = newentry;
    //             }
    //             return ptr;
    //         }
    //     }
    //     //no free block after newentry
    //     if (head == NULL) {
    //         head = newentry;
    //         return ptr;
    //     }
    //     entry* p = head;
    //     while (p -> next != NULL) {
    //         if (p > newentry) {
    //             break;
    //         }
    //         p = p->next;
    //     }
    //     if (p < newentry) {
    //         p->next = newentry;
    //         newentry->prev = p;
    //         return ptr;
    //     }
    //     if (p == head) {
    //         head = newentry;
    //     }
    //     entry* before = p->prev;
    //     newentry->prev = before;
    //     newentry->next = p;
    //     if (before != NULL) {
    //         before->next = newentry;
    //     }
    //     if (p != NULL) {
    //         p->prev = newentry;
    //     }
    //     return ptr;
    // }
    // return ptr;
}
// void* print() {
//     entry* ptr = head;
//     while(ptr != NULL) {
//         printf()
//     }
// }