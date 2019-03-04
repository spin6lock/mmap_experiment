#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <err.h>
#include <sys/mman.h>
#include <sys/stat.h>   /* For mode constants */
#include <fcntl.h>      /* For O_* constants */
#include <unistd.h>     /* ftruncate */
#include <sys/types.h>  /* ftruncate */

void *nullptr = NULL;
#define PAGESIZE 4*1024
#define PAGE_CNT 2
#define SKIP 2
#define PATTERN_ONE 1
#define PATTERN_TWO 2
#define SAMPLE_LEN 30

char filename[255] = "/tmp/test_map_file.XXXXXX";
int create_memory_file() {
    int fd = mkstemp(filename);
    if (fd < 0) {
        fprintf(stderr, "mkstemp failed:%s\n", strerror(errno));
        err(1, "%s", filename);
        exit(1);
    }
    int ret = unlink(filename);
    if (ret != 0) {
        fprintf(stderr, "unlink failed:%s\n", strerror(errno));
        exit(1);
    }
    ret = ftruncate(fd, PAGE_CNT * PAGESIZE);
    if (ret != 0) {
        fprintf(stderr, "ftruncate failed:%s\n", strerror(errno));
        exit(1);
    }
    return fd;
}

char *create_one_on_one_mapping(int fd) {
    char* mymem = mmap(nullptr, PAGE_CNT * PAGESIZE, PROT_READ|PROT_WRITE, 
            MAP_SHARED, fd, 0);
    if (mymem == MAP_FAILED) {
        fprintf(stderr, "mmap failed:%s\n", strerror(errno));
        exit(1);
    }
    printf("mapping begins at:%p\n", mymem);
    return mymem;
}

void write_pattern(char *begin, char pattern, unsigned int every_n_char, 
        unsigned int count_limit) {
    int count = 0;
    for (int i = 0; count < count_limit; i += every_n_char) {
        *(begin+i) = pattern;
        count++;
    }
}

void read_one_page(char *begin, unsigned int count) {
    for (int i = 0; i < count; i++) {
        printf("%x", begin[i]);
    }
    printf("\nread:%d bytes\n", count);
}

void merge_two_page(void *remove, int fd, int keep_offset_in_file) {
    void *ret = mmap(remove, PAGESIZE, PROT_READ|PROT_WRITE, 
            MAP_SHARED|MAP_FIXED, fd, keep_offset_in_file);
    if (ret == MAP_FAILED) {
        fprintf(stderr, "mmap failed:%s\n", strerror(errno));
        exit(1);
    }
}

void *create_one_page(int fd, int offset) {
    void *ret = mmap(nullptr, PAGESIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, offset);
    if (ret == MAP_FAILED) {
        fprintf(stderr, "line:%d, mmap failed:%s\n", __LINE__, strerror(errno));
        exit(1);
    }
}

void read_file(int fd) {
    char buff[255] = {'\0'};
    int read_bytes = read(fd, buff, 254);
    printf("read file=>read:%d, content:%s\n", read_bytes, buff);
}

void copy_two_to_one(char *two, char *one) {
    for (int i = 0; i < PAGESIZE; i++) {
        if (two[i] > 0) {
            one[i] = two[i];
        }
    }
}

int main() {
    int fd = create_memory_file();
    char *page_one = create_one_page(fd, 0);
    //fill in 10101010...
    write_pattern(page_one, PATTERN_ONE, SKIP, PAGESIZE / SKIP);
    printf("page one pattern:\n");
    read_one_page(page_one, SAMPLE_LEN);
    char *page_two = create_one_page(fd, PAGESIZE);
    //fill in 0202020202...
    write_pattern(page_two + 1, PATTERN_TWO, SKIP, PAGESIZE / SKIP);
    printf("page two pattern:\n");
    read_one_page(page_two, SAMPLE_LEN);
    copy_two_to_one(page_two, page_one);
    merge_two_page(page_two, fd, 0);
    printf("page two after merge:\n");
    read_one_page(page_two, SAMPLE_LEN);
    return 0;
}
