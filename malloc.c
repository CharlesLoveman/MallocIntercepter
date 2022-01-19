#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dlfcn.h>

unsigned long bytes_allocated = 0;

/*Temporary buffer to allow dlsym to allocate dynamically.
 *Without this the preloaded malloc is called recursively, leading to a segmentation fault.
 *https://stackoverflow.com/questions/6083337/overriding-malloc-using-the-ld-preload-mechanism
 *I referenced this stack overflow page while trying to solve the seg fault.
 */
#define SIZE 1024
char tempMem[SIZE];
char *tempHead = tempMem;
bool initialising = false;

void *temp_malloc(size_t);
void *malloc(size_t);
void free(void*);

void* (*malloc_func)(size_t) = temp_malloc;
void (*free_func)(void*) = NULL;

//Sets the function pointers to the actual malloc and free functions.
void init() {
    malloc_func = dlsym(RTLD_NEXT, "malloc");
    free_func = dlsym(RTLD_NEXT, "free");
}

//Allocates dynamic memory using malloc_func.
//If we haven't set malloc_func calls init and sets up the initialising flag
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

/*Simple wrapper around free_func.
 *Needed to allow dlsym to free temporary memory.
 *dlsym shouldn't overflow the buffer,
 *so leaving the temporary buffer filled shouldn't cause an issue.
 */
void free(void *ptr) {
    if ((void*) tempMem <= ptr && ptr <= (void*)tempHead) {
        return;
    } else {
        free_func(ptr);
    }
}

/*Allocates temporary dynamic memory from the buffer.
 *Exits if more than SIZE bytes are allocated.
 */
void *temp_malloc(size_t size) {
   void* result = (void*)tempHead;
   tempHead += size;
   if (tempHead - tempMem > SIZE) {
       fprintf(stderr, "More than %d bytes of temporary memory allocated\n", SIZE);
       exit(-1);
   }
   return result;
}
