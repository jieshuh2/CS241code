/**
 * parallel_make
 * CS 241 - Spring 2021
 */

#include "format.h"
#include "graph.h"
#include "parmake.h"
#include "parser.h"
#include <unistd.h>
#include <stdio.h>
#include "includes/queue.h"
#include "includes/set.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <assert.h>
// typedef struct dep {
//     size_t numdepfinish;
//     size_t numdepfail;
//     size_t numdep;
// }dep_t;
// typedef struct task {
//     dep_t* depend;
//     char* commands;
// }task_t;
static queue* q;
// static queue* node;
static pthread_cond_t cv;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
static graph* g;
static size_t numgoals;
static size_t numgoalfin;
static vector* goal;
void dfs(char * vertex, set* visited);
int isCyclic(set * visit, set * recurv,char* vertex);
// int runcommand(char* vertex, dep_t * parent);
int run(vector* commands);

void* thread(void* ptr) {
    while(1) {
        setvbuf(stdout, NULL, _IONBF, 0);
        char* vertex = queue_pull(q);
        if (strcmp(vertex, "EOF") == 0) {
            queue_push(q, vertex);
            pthread_exit(NULL);
        }
        pthread_mutex_lock(&mutex);
        rule_t * rule = (rule_t *)graph_get_vertex_value(g, vertex);
        // assert(rule->state == 1);
        if (rule->state != 2) {
            pthread_mutex_unlock(&mutex);
            struct stat  buffer; 
            if (stat (vertex, &buffer) != 0) {
                int fail = run(rule->commands);
                pthread_mutex_lock(&mutex);
                if(fail == 1) {
                    rule->state = 2; //fail
                } else {
                    rule->state = 4; //satisfied
                }
                pthread_mutex_unlock(&mutex);
            } else {
                vector* neighbors = graph_neighbors(g, vertex);
                int modify = 0;
                for (size_t i = 0; i < vector_size(neighbors); i++) {
                    struct stat depbuffer;
                    if (stat (vector_get(neighbors, i), &depbuffer) != 0) {
                        // printf("%s", vertex);
                        modify = 1;
                        break;
                    } else {
                        double timeelapse = difftime(depbuffer.st_mtime, buffer.st_mtime);
                        // printf("here2");
                        // printf("%s", vertex);
                        // printf("%f\n", timeelapse);
                        if (timeelapse > 1) {
                            //later than dependencies
                            // printf("here3");
                            modify = 1;
                            break;
                        }
                    }
                }
                // printf("%s\n", vertex);
                if (modify == 1) {
                    int fail = run(rule->commands);
                    pthread_mutex_lock(&mutex);
                    if(fail == 1) {
                        rule->state = 2; //fail
                    } else {
                        rule->state = 4; //satisfied
                    }
                    pthread_mutex_unlock(&mutex);
                } else {
                    pthread_mutex_lock(&mutex);
                    rule->state = 4;
                    pthread_mutex_unlock(&mutex);
                }
                vector_destroy(neighbors);
            }
        } else {
            pthread_mutex_unlock(&mutex);
        }
        // printf("reach here");
        vector* antineighbor = graph_antineighbors(g, vertex);
        for (size_t i = 0; i < vector_size(antineighbor); i++) {
            char* next = vector_get(antineighbor, i);
            // if (strcmp(next, "") == 0) {
            //     continue;
            // }
            // printf("%s\n", (char*)vector_get(antineighbor, i));
            rule_t * deprule = (rule_t *)graph_get_vertex_value(g, next);
            pthread_mutex_lock(&mutex);
            if (deprule->state == 0) {
                pthread_mutex_unlock(&mutex);
                continue;
            }
            if (deprule->state == 2) {
                pthread_mutex_unlock(&mutex);
                continue;
            }
            assert(deprule->state != 4);
            pthread_mutex_unlock(&mutex);

            vector* dependency = graph_neighbors(g, next);
            int ready = 0;
            int shouldfail = 0;
            pthread_mutex_lock(&mutex);
            for (size_t j = 0; j < vector_size(dependency); j++) {
                rule_t * nextrule = (rule_t *)graph_get_vertex_value(g, vector_get(dependency, j));
                if (nextrule->state == 2) {
                    shouldfail = 1;
                }
                if (nextrule->state == 1) {
                    ready = 1;
                    break;
                }
                assert(nextrule->state != 0);
            }
            pthread_mutex_unlock(&mutex);
            vector_destroy(dependency);
            if (ready == 1) {
                //have an unsatisfied dependency
                continue;
            }
            if (shouldfail == 1) {
                pthread_mutex_lock(&mutex);
                deprule->state = 2;
                pthread_mutex_unlock(&mutex);
            }
            queue_push(q, next);
            // printf("push : %s", next);
            
        }
        // printf("final");
        vector_destroy(antineighbor);
        for (size_t i = 0; i < vector_size(goal); i++) {
            if (strcmp(vertex, vector_get(goal, i)) == 0) {
                pthread_mutex_lock(&mutex);
                // printf("trylock %d\n", try);
                // printf("%zu\n",numgoals);
                numgoalfin += 1;
                vector_erase(goal, i);
                if (numgoalfin == numgoals) {
                    pthread_cond_broadcast(&cv);
                }
                pthread_mutex_unlock(&mutex);
                break;
            }
        }
        // printf("finalhere");
    }
}
int parmake(char *makefile, size_t num_threads, char **targets) {
    // good luck!
    g = parser_parse_makefile(makefile, targets);
    vector *goals = graph_neighbors(g, "");
    vector *toerase = unsigned_long_vector_create();
    for (size_t i = 0; i < vector_size(goals); i++) {
        set* visit = string_set_create();
        set* recurv = string_set_create();
        if(isCyclic(visit, recurv, vector_get(goals, i)) == 1) {
            print_cycle_failure(vector_get(goals, i));
            vector_push_back(toerase, &i);
        }
        set_destroy(visit);
        set_destroy(recurv);
    }
    for (size_t i = 0; i < vector_size(toerase); i++) {
        size_t idx = *((size_t*)vector_get(toerase, i));
        vector_erase(goals, idx);
    }
    q = queue_create(-1);
    // node = queue_create(-1);
    set* visited = string_set_create();
    for (size_t i = 0; i < vector_size(goals); i++) {
        dfs(vector_get(goals, i), visited);
    }
    set_destroy(visited);
    numgoalfin = 0;
    numgoals = vector_size(goals);
    goal = goals;
    pthread_t threads[num_threads];
    for (size_t i = 0; i < num_threads; i++) {
        pthread_create(threads + i, NULL, thread, NULL);
    }
    pthread_mutex_lock(&mutex);
    while(numgoalfin < numgoals) {
        pthread_cond_wait(&cv, &mutex);
    }
    pthread_mutex_unlock(&mutex);
    queue_push(q, "EOF");
    for (size_t i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    // free(end);
    vector_destroy(toerase);
    vector_destroy(goals);
    graph_destroy(g);
    queue_destroy(q);
    // queue_destroy(node);
    return 0;
}
void dfs(char * vertex, set * visited) {
    rule_t * rule = (rule_t *)graph_get_vertex_value(g, vertex);
    pthread_mutex_lock(&mutex);
    rule->state = 1;
    pthread_mutex_unlock(&mutex);
    set_add(visited, vertex);
    vector* neighbors = graph_neighbors(g, vertex);
    for (size_t i = 0; i < vector_size(neighbors); i++) {
        if (!set_contains(visited,vector_get(neighbors, i))) {
            dfs(vector_get(neighbors, i), visited);
        }
    }
    if (vector_size(neighbors) == 0) {
        queue_push(q, vertex);
    }
    vector_destroy(neighbors);
}

int isCyclic(set * visit, set * recurv, char* vertex) {
    // rule_t * rule = (rule_t *)graph_get_vertex_value(g, vertex);
    set_add(visit, vertex);
    set_add(recurv, vertex);
    vector* neighbors = graph_neighbors(g, vertex);
    for (size_t i = 0; i < vector_size(neighbors); i++) {
        if (!set_contains(visit, vector_get(neighbors, i))) {
            int accylic = isCyclic(visit, recurv,vector_get(neighbors, i));
            if (accylic == 1) {
                return 1;
            }
        }
        if (set_contains(recurv, vector_get(neighbors, i))) {
            return 1;
        }
    }
    vector_destroy(neighbors);
    set_remove(recurv, vertex);
    return 0;
}
int run(vector* commands) {
    for (size_t i = 0; i < vector_size(commands); i++) {
        int success = system(vector_get(commands, i));
        if (success != 0) {
            return 1;
        }
    }
    return 0;
}
// int runcommand(char* vertex, dep_t * parent) {//need to be protected
//     pthread_mutex_lock(&mutex);
//     vector* neighbors = graph_neighbors(g, vertex);
//     rule_t * rule = (rule_t *)graph_get_vertex_value(g, vertex);
//     int result = 0;
//     dep_t* child = malloc(sizeof(dep_t)); //free
//     child->numdep = vector_size(neighbors);
//     child->numdepfail = 0;
//     child->numdepfinish = 0;
//     pthread_mutex_unlock(&mutex);

//     for (size_t i = 0; i < vector_size(neighbors); i++) {
//         pthread_mutex_lock(&mutex);
//         rule_t * dependent = (rule_t *)graph_get_vertex_value(g, vector_get(neighbors, i));
//         if (dependent->state == 2) {
//             pthread_mutex_unlock(&mutex);
//             result = 2;
//             break;
//         }
//         if (dependent->state == 0) {
//             pthread_mutex_unlock(&mutex);
//             int depresult = runcommand(vector_get(neighbors,i), child);
//             if (depresult == 3) {
//                 result = 3;
//             }
//         } else {
//             if (dependent->state == 3) {
//                 result = 3;
//             }
//             child->numdepfinish += 1;
//             pthread_mutex_unlock(&mutex);
//         }
//     }
//     pthread_mutex_lock(&mutex);
//     while(child->numdep < child->numdepfinish) {
//         pthread_cond_wait(&cv, &mutex);
//     }
//     if (result == 2 || child->numdepfail > 0) {
//         //fail dependencies
//         free(child);
//         vector_destroy(neighbors);
//         rule->state = 2;
//         return 2;
//     }
//     pthread_mutex_unlock(&mutex);
//     free(child);
//     //finish runing all dependencies
//     struct stat  buffer;  
//     task_t* task = malloc(sizeof(task_t));
//     pthread_mutex_lock(&mutex);
//     task->commands = vertex;
//     task->depend = parent;
//     pthread_mutex_unlock(&mutex);
//     if (stat (vertex, &buffer) != 0) {
//         queue_push(q, task);
//         // int success = run(rule->commands);
//         // if (success > 0) {
//         //     rule->state = 2;
//         //     vector_destroy(neighbors);
//         //     return 2;
//         // } else {
//         //     rule->data = NULL;
//         //     rule->state = 3; //not a rule on disk. Use to alert its ancestor.
//         //     vector_destroy(neighbors);
//         //     return 3;
//         // }
//         pthread_mutex_lock(&mutex);
//         rule->state = 3;
//         pthread_mutex_unlock(&mutex);
//     } else if (result == 3) { //have an dependent that is not a rule on disk
//         // int success = run(rule->commands);
//         // if (success > 0) {
//         //     rule->state = 2;
//         //     vector_destroy(neighbors);
//         //     return 2;
//         // } else {
//         //     rule->state = 3; //have an dependent that is not a rule on disk
//         //     vector_destroy(neighbors);
//         //     return 3;
//         // }
//         pthread_mutex_lock(&mutex);
//         rule->state = 3;
//         pthread_mutex_unlock(&mutex);
//     } else {
//         int modify = 0;
//         for (size_t i = 0; i < vector_size(neighbors); i++)  {
//             struct stat depbuf;
//             if (stat(vector_get(neighbors, i), &depbuf) != 0) { //but normally this file shoud exist or this would be tha cond above
//                 double timeelapse = difftime(depbuf.st_mtime, buffer.st_mtime);
//                 if (timeelapse >= 1) {
//                     modify = 1;
//                     break;
//                 }
//             }
//         }
//         if (modify == 1) {
//             // int success = run(rule->commands);
//             // if (success > 0) {
//             //     rule->state = 2;
//             //     vector_destroy(neighbors);
//             //     return 2;
//             // }
//             queue_push(q, task);
//             pthread_mutex_lock(&mutex);
//             rule->state = 1;
//             pthread_mutex_unlock(&mutex);
//             return 1;
//         } else {
//             pthread_mutex_lock(&mutex);
//             parent->numdepfinish += 1;
//             rule->state = 1;     
//             pthread_mutex_unlock(&mutex);
//         }
//     }
//     vector_destroy(neighbors);
//     return 0;
// }

// void* runthread(void* ptr) {
//     while(1) {
//         task_t* task = queue_pull(q);
//         if (strcmp(task->commands, "end") == 0) {
//             queue_push(q, task);
//             pthread_exit(NULL);
//         }
//         pthread_mutex_lock(&mutex);
//         rule_t * rule = (rule_t *)graph_get_vertex_value(g, task->commands);
//         pthread_mutex_unlock(&mutex);
//         int fail = run(rule->commands);
//         if (fail == 1) {
//             rule->state = 2;
//         }
//         pthread_mutex_lock(&mutex);
//         dep_t* parent = task->depend;
//         parent->numdepfinish += 1;
//         parent->numdepfail += fail;
//         if (parent->numdepfinish == parent->numdep) {
//             pthread_cond_broadcast(&cv);
//         }
//         pthread_mutex_unlock(&mutex);
//         free(task);
//     }
// }
