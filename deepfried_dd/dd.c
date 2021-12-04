/**
 * deepfried_dd
 * CS 241 - Spring 2021
 */
#include "format.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
extern char *optarg;
extern int optind, opterr, optopt;
static size_t bytescopy;
static size_t fullblockin;
static size_t partblockin;
static size_t partblockout;
static size_t fullblockout;
static clock_t begin;
void handler() {
    clock_t end = clock();
    print_status_report(fullblockin, partblockin, fullblockout, partblockout, bytescopy, (double)(end - begin) / CLOCKS_PER_SEC);
}
int main(int argc, char **argv) {
    begin = clock();
    int opt;
    FILE* inputfile = stdin;
    FILE* outputfile = stdout;
    long blocksize = 512;
    long blocknum = -1;
    long inputskip = 0;
    long outputskip = 0;
    char *eptr;
    signal(SIGUSR1, handler);
    while ((opt = getopt(argc, argv, "i:o:b:c:p:k:")) != -1) {
        switch (opt) {
        case 'i':
            inputfile = fopen(optarg, "r");
            if (inputfile == NULL) {
                print_invalid_input(optarg);
                exit(1);
            }
            break;
        case 'o':
            outputfile = fopen(optarg, "a+");
            if (outputfile == NULL) {
                print_invalid_output(optarg);
                exit(1);
            }
            break;
        case 'b':
            blocksize = strtol(optarg, &eptr, 10);
            break;
        case 'c':
            blocknum = strtol(optarg, &eptr, 10);
            break;
        case 'p':
            inputskip = strtol(optarg, &eptr, 10);
            break;
        case 'k':
            outputskip = strtol(optarg, &eptr, 10);
            break;
        default: 
            exit(1);
        }
    }
    char block[blocksize];
    // block = blockk;
    fullblockin = 0;
    partblockin = 0;
    bytescopy = 0;
    fullblockout = 0;
    partblockout = 0;
    if (inputfile != stdin) {
        fseek(inputfile, inputskip*blocksize,SEEK_SET);
    }
    if (outputfile != stdout) {
        fseek(outputfile, outputskip*blocksize, SEEK_SET);
    }
    ssize_t totalbytes = blocknum*blocksize;
    if (blocknum < 0) {
        totalbytes = -1;
    }
    while(1) {
        if (totalbytes > 0 && bytescopy >= (size_t)totalbytes) {
            break;
        }
        if(feof(inputfile)) { 
            break;
        }
        memset(block, 0, blocksize);
        size_t numread = fread(block, sizeof(char), blocksize, inputfile);
        if (numread > 0) {
            if (numread < (size_t)blocksize) {
                partblockin += 1;
            } else {
                fullblockin += 1;
            }
        }
        size_t numwrite = fwrite(block, sizeof(char), numread, outputfile);
        if (numwrite > 0) {
            if (numwrite < (size_t)blocksize) {
                partblockout += 1;
            } else {
                fullblockout += 1;
            }
        }
        bytescopy += numwrite;
    }
    clock_t end = clock();
    print_status_report(fullblockin, partblockin, fullblockout, partblockout, bytescopy, (double)(end - begin) / CLOCKS_PER_SEC);
}