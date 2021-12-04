/**
 * mad_mad_access_patterns
 * CS 241 - Spring 2021
 */
#include "tree.h"
#include "utils.h"
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses mmap to access the data.

  ./lookup2 <data_file> <word> [<word> ...]
*/

int search(char* addr, char* word, char* filename, int offset, float* price, uint32_t* count);
int main(int argc, char **argv) {
  if (argc < 3) {
    printArgumentUsage();
    return 1;
  }
  int fd = open(argv[1], O_RDONLY); //File is 2 Pages
  if (fd == -1) {
    openFail(argv[1]);
    return 1;
  }
  struct stat sb;
  fstat(fd, &sb);
  off_t offset = 0;
  size_t length = sb.st_size;
  off_t page_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
  char * addr = mmap(NULL, length + offset - page_offset, PROT_READ,
     MAP_PRIVATE, fd, page_offset);
  char check[5];
  strncpy(check, addr, 4);
  check[4] = '\0';
  if(strcmp(check, "BTRE") != 0) {
    formatFail(argv[1]);
    exit(2);
  }
  for (int i = 2; i < argc; i++) {
    uint32_t count;
    float price;
    int offset = search(addr, argv[i], argv[1], 4, &price, &count);
    if (offset == -1) {
      printNotFound(argv[i]);
    } else {
      printFound(argv[i], count, price);
    }
  }
  close(fd);
  munmap(addr, length + offset - page_offset);
  return 0;
}

int search(char* addr, char* word, char* filename, int offset, float* price, uint32_t* count) {
  uint32_t left_child;
  strncpy((char*)&left_child, addr + offset, sizeof(uint32_t));

  uint32_t right_child;
  strncpy((char*)&right_child, addr + offset + sizeof(uint32_t), sizeof(uint32_t));

  strncpy((char*)count, addr + offset + sizeof(uint32_t) * 2, sizeof(uint32_t));
  strncpy((char*)price, addr + offset + sizeof(uint32_t) * 3, sizeof(float));

  long len;
  for (len = 0;;len++) {
    if (addr[offset + sizeof(uint32_t) * 3 + sizeof(float) + len] == '\0') {
      break;
    }
  }
  char words[len + 1];
  strncpy(words, addr + offset + sizeof(uint32_t) * 3 + sizeof(float), len + 1);
  assert(words[len] == '\0');
  if (strcmp(word, words) == 0) {
    return offset;
  } else if (strcmp(word, words) < 0) {
    if (left_child == 0) {
      return -1;
    } else {
      return search(addr, word, filename, left_child, price, count);
    }
  } else {
    if (right_child == 0) {
      return -1;
    } else {
      return search(addr, word, filename, right_child, price, count);
    }
  }
  return -1;
}
