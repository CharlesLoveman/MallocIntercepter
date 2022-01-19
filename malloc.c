#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dlfcn.h>

unsigned long bytes_allocated = 0;

#define SIZE 1024
char tempMem[SIZE];
char *tempHead = tempMem;
bool initialising = false;

void *temp_malloc(size_t);
void *malloc(size_t);
void free(void*);

void* (*malloc_func)(size_t) = temp_malloc;
void (*free_func)(void*) = NULL;

void init() {
    malloc_func = dlsym(RTLD_NEXT, "malloc");
    free_func = dlsym(RTLD_NEXT, "free");
}

void *malloc(size_t size) {
    if (malloc_func == temp_malloc && !initialising) {
        initialising = true;
        init();
        initialising = false;
    }
    void *result = malloc_func(size);
    if (result) {
        bytes_allocated += size;
        fprintf(stderr, "Total bytes allocated so far: %llu\n", bytes_allocated);
    }
    return result;
}

void free(void *ptr) {
    if ((void*) tempMem <= ptr && ptr <= (void*)tempHead) {
        return;
    } else {
        free_func(ptr);
    }
}

void *temp_malloc(size_t size) {
   void* result = (void*)tempHead;
   tempHead += size;
   if (tempHead - tempMem > SIZE) {
       fprintf(stderr, "More than %d bytes of temporary memory allocated\n", SIZE);
       exit(-1);
   }
   return result;
}
