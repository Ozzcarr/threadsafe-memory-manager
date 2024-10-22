#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

typedef struct MemoryBlock {
    void *start;
    void *end;
    struct MemoryBlock *next;
} MemoryBlock;

void mem_init(size_t size);
void *mem_alloc(size_t size);
void mem_free(void *block);
void *mem_resize(void *block, size_t size);
void mem_deinit();

#endif