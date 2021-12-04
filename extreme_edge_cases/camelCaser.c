/**
 * extreme_edge_cases
 * CS 241 - Spring 2021
 */
#include "camelCaser.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

char **camel_caser(const char *input_str) {
    // TODO: Implement me!
    if (input_str == NULL) {
        return NULL;
    }
    //counting the number of string(punctuation) in camel
    int number = 0;
    for (unsigned i = 0; i < strlen(input_str); i++) {
        if (ispunct(input_str[i]) != 0) {
            number++;
        }
    }
    char ** output_str = (char **)malloc (sizeof (char *) * (number + 1));
    output_str[number] = NULL;
    const char *input_s = input_str;
    for (int i = 0; i < number; i++) {
        //counting number of char in string
        int count = 0;
        // int valid = 1;
        while(!ispunct(input_s[count])) {
            // if (!(ispunct(input_s[count]) || isalpha(input_s[count]) || isspace(input_s[count]))) {
            //     printf("invalid char:%c, count:%d\n", input_s[count], count);
            //     valid = 0;
            // }
            count++;
        }

        char * output_s = (char*) malloc(sizeof(char) * (count + 1));
        // if (valid < 1) {
        //     strncpy(output_s, input_s, count);
        //     input_s += (count + 1);
        //     output_s[count] = '\0';
        //     output_str[i] = output_s;
        //     continue;
        // }
        int upper = 0;
        char* ptr = output_s;
        //copy a valid one
        for (int j = 0; j < count; j++) {
            if (isspace(*input_s)) {
                upper = 1;
                input_s++;
            } else {
                if (!(ispunct(*input_s) || isalpha(*input_s) || isspace(*input_s))) {
                    *ptr = *input_s;
                    ptr ++;
                    input_s ++;
                } else if (upper > 0) {
                    *ptr = toupper(*input_s);
                    ptr ++;
                    input_s ++;
                    upper = 0;
                } else {
                    *ptr = tolower(*input_s);
                    ptr ++;
                    input_s ++;
                }
            }

        }
        input_s ++;
        // printf("input:(%c)", *input_s);
        *ptr = '\0';
        output_s[0] = tolower(output_s[0]);
        output_str[i] = output_s;
    }
    return output_str;
}

void destroy(char **result) {
    // TODO: Implement me!
    char**ptr = result;
    while(*ptr != NULL) {
        free(*ptr);
        ptr++;
    }
    free(result);
    return;
}
