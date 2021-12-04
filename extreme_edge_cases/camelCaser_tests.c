/**
 * extreme_edge_cases
 * CS 241 - Spring 2021
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "camelCaser.h"
#include "camelCaser_tests.h"

int test_camelCaser(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
    // TODO: Implement me!
    char *input0 = NULL;
    char** output0 = camelCaser(input0);
    if (output0 != NULL) {
        return 0;
    }
    char *input1 = "";
    char** output1 = camelCaser(input1);
    if (output1[0] != NULL) {
        return 0;
    }
    char *input2 = "hello Hi \nhello";
    char** output2 = camelCaser(input2);
    if (output2[0] != NULL) {
        return 0;
    }

    char *input = "   HaHH, Wooh tO\x6f; Hello. \n.welCoMe\t to Cs241 \n448ccgg59?!241. ,hello world   ";
    char** output = camelCaser(input);

    if (output[0] == NULL || strcmp(output[0], "hahh") != 0) {
        return 0;
    }
    if (output[1] == NULL || strcmp(output[1], "woohToo") != 0) {
        return 0;
    }
    if (output[2] == NULL ||strcmp(output[2], "hello") != 0) {
        return 0;
    }
    if (output[3] == NULL ||strcmp(output[3], "") != 0) {
        return 0;
    }
    if (output[4] == NULL ||strcmp(output[4], "welcomeToCs241448Ccgg59") != 0) {
        return 0;
    }
    if (output[5] == NULL ||strcmp(output[5], "") != 0) {
        return 0;
    }
    if (output[6] == NULL ||strcmp(output[6], "241")!= 0) {
        return 0;
    }
    if (output[7] == NULL ||strcmp(output[7], "")!= 0) {
        return 0;
    }
    if (output[8] != NULL) {
        return 0;
    }
    destroy(output);
    return 1;
}
