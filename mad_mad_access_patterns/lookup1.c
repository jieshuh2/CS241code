/**
 * mad_mad_access_patterns
 * CS 241 - Spring 2021
 */
#include "tree.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses fseek() and fread() to access the data.

  ./lookup1 <data_file> <word> [<word> ...]
*/
int search(FILE* file, char* word, char* filename, int offset, float* price, uint32_t* count);
int main(int argc, char **argv) {
  if (argc < 3) {
    printArgumentUsage();
    return 1;
  }
  FILE* file = fopen(argv[1], "r");
  if (file == NULL) {
    openFail(argv[1]);
    return 1;
  }
  char check[10];
  size_t bytesread = fread(check, 1, 4, file);
  if (bytesread < 4) {
    formatFail(argv[1]);
    exit(2);
  }
  check[4] = '\0';
  if(strcmp(check, "BTRE") != 0) {
    formatFail(argv[1]);
    exit(2);
  }
  for (int i = 2; i < argc; i++) {
    uint32_t count;
    float price;
    int offset = search(file, argv[i], argv[1], 4, &price, &count);
    if (offset == -1) {
      printNotFound(argv[i]);
    } else {
      printFound(argv[i], count, price);
    }
  }
  fclose(file);
  return 0;
}

int search(FILE* file, char* word, char* filename, int offset, float* price, uint32_t* count) {
  fseek(file, (long)offset, SEEK_SET);
  uint32_t left_child;
  long bytesread = fread((char*)&left_child, 1, sizeof(uint32_t), file);
  if (bytesread < 4) {
    formatFail(filename);
    exit(2);
  }
  uint32_t right_child;
  bytesread = fread((char*)&right_child, 1, sizeof(uint32_t), file);
  if (bytesread < 4) {
    formatFail(filename);
    exit(2);
  }
  bytesread = fread((char*)count, 1, sizeof(uint32_t), file);
  if (bytesread < 4) {
    formatFail(filename);
    exit(2);
  }
  bytesread = fread((char*)price, 1, sizeof(float), file);
  if (bytesread < 4) {
    formatFail(filename);
    exit(2);
  }
  long len = 0;
  while(fgetc(file) != '\0') {
    len ++;
  }
  len++;
  fseek(file, -len, SEEK_CUR);
  char words[len];
  bytesread = fread(words, 1, len, file);
  if (bytesread < len) {
    formatFail(filename);
    exit(2);
  }
  assert(words[len - 1] == '\0');
  if (strcmp(word, words) == 0) {
    return offset;
  } else if (strcmp(word, words) < 0) {
    if (left_child == 0) {
      return -1;
    } else {
      return search(file, word, filename, left_child, price, count);
    }
  } else {
    if (right_child == 0) {
      return -1;
    } else {
      return search(file, word, filename, right_child, price, count);
    }
  }
  return -1;
}

