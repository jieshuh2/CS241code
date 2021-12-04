/**
 * perilous_pointers
 * CS 241 - Spring 2021
 */
#include "part2-functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * (Edit this function to print out the "Illinois" lines in
 * part2-functions.c in order.)
 */
int main() {
    // your code here
    first_step(81);
    int value = 132;
    second_step(&value);
    int * array[1];
    value = 8942;
    array[0] = &value;
    double_step(array);
    char strange[10];
    *(int *)(strange + 5) = 15;
    strange_step(strange);
    empty_step("ab");
    char s2[5];
    s2[3] = 'u';
    two_step (s2, s2);
    three_step(s2, s2+2, s2+4);
    s2[2] = 'a';
    s2[3] = s2[2] + 8;
    s2[1] = s2[2] - 8;
    step_step_step(s2, s2, s2);
    int b = 1;
    char a[4];
    *(int *)a = b;
    it_may_be_odd(a, b);
    char* ten = (char*) malloc(10*sizeof(char));
    strcpy(ten, "a,CS241");
    tok_step(ten);
    free(ten);
    char blue[4];
    blue[0] = 1;
    blue[1] = 0;
    blue[2] = 0;
    blue[3] = 2;
    the_end(blue, blue);
    return 0;
}
