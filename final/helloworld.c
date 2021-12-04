// author: jieshuh2
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc != 4) {
        printf("Hello World");
        exit(0);
    }
    char* cmd = argv[0];
    char* mystuff = argv[1];
    char* bin1 = argv[2];
    char* bin2 = argv[3];
    if (strcmp(cmd, "./encrypt") == 0) {
        //encrypt
        struct stat sb;
        int fd = open(mystuff, O_RDWR);
        //check file exist
        int s = fstat(fd, &sb);
        if (s != 0) {
            printf("no file");
            exit(1);
        }
        //mmap inpuf file
        off_t offset = 0;
        size_t length = sb.st_size;
        off_t page_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
        char * addr = mmap(NULL, length + offset - page_offset, PROT_READ | PROT_WRITE, MAP_SHARED, fd, page_offset);
        //open random and bins file
        int rand = open("/dev/random", O_RDONLY);   
        FILE* file1 = fopen(bin1, "w");
        FILE* file2 = fopen(bin2, "w");
        for (size_t i = 0; i < length; i++) {
            char rw;
            read(rand, &rw, sizeof(char));
            char mychar = addr[i];
            fputc(rw, file1);
            fputc(rw^mychar, file2);
            //set bytes to zero
            addr[i] = 0x00;
        }
        //close files
        munmap(addr, length + offset - page_offset);
        close(fd);
        fclose(file1);
        fclose(file2);
        close(rand);

    } else if (strcmp(cmd, "./decrypt") == 0) {
        //decrypt
        //mmap bin1
        struct stat sb1;
        int fd1 = open(bin1, O_RDONLY);
        int s = fstat(fd1, &sb1);
        if (s != 0) {
            printf("no file");
            exit(1);
        }
        off_t offset = 0;
        size_t length = sb1.st_size;
        off_t page_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
        char * addr1 = mmap(NULL, length + offset - page_offset, PROT_READ, MAP_PRIVATE, fd1, page_offset);
        //mmap bin2
        struct stat sb2;
        int fd2 = open(bin2, O_RDONLY);
        s = fstat(fd2, &sb2);
        if (s != 0) {
            printf("no file");
            exit(1);
        }
        //check size 1 == size 2
        if (sb2.st_size != sb1.st_size) {
            printf("size don't match");
            exit(1);
        }
        length = sb2.st_size;
        page_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
        char * addr2 = mmap(NULL, length + offset - page_offset, PROT_READ, MAP_PRIVATE, fd2, page_offset);
        //open(outputfile)
        FILE* output = fopen(mystuff, "w");
        for (size_t i = 0; i < length; i++) {
            char c = addr1[i] ^ addr2[i];
            // putc
            fputc(c, output);
        }
        munmap(addr1, length + offset - page_offset);
        munmap(addr2, length + offset - page_offset);
        close(fd1);
        close(fd2);
        fclose(output);
        //delete files
        unlink(bin1);
        unlink(bin2);
    } else {
        printf("Hello World");
        exit(0);
    }
    return 0;
}