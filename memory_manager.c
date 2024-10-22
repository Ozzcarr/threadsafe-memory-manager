#include "memory_manager.h"

void *memory;
MemoryBlock *memory_head;
size_t memory_size;
pthread_mutex_t lock;

/**
 * @brief Initializes the memory manager with the specified size.
 *
 * @param size The size of the memory pool in bytes.
 */
void mem_init(size_t size) {
    memory = malloc(size);
    memory_head = NULL;
    memory_size = size;
    pthread_mutex_init(&lock, NULL);
}

/**
 * @brief Allocates a block of memory with the specified size.
 *
 * @param size The size of the allocated block in bytes.
 * @return A pointer to the start of the allocated memory, or NULL if the
 * allocation fails.
 */
void *mem_alloc_no_lock(size_t size) {
    if (!memory || size > memory_size) return NULL;
    if (size == 0) return memory;

    MemoryBlock *new_block = malloc(sizeof(MemoryBlock));
    if (!new_block) return NULL;

    // Insert first
    if (!memory_head || memory_head->start - memory >= size) {
        new_block->start = memory;
        new_block->end = memory + size;
        new_block->next = memory_head;
        memory_head = new_block;
        return memory;
    }

    // General insert
    MemoryBlock *current = memory_head;
    while (current) {
        size_t free_size = current->next ? current->next->start - current->end
                                         : memory + memory_size - current->end;
        if (free_size >= size) {
            new_block->start = current->end;
            new_block->end = current->end + size;
            new_block->next = current->next;
            current->next = new_block;
            return new_block->start;
        }
        current = current->next;
    }

    free(new_block);
    return NULL;
}

/**
 * @brief Allocates a block of memory with the specified size.
 *
 * @param size The size of the allocated block in bytes.
 * @return A pointer to the start of the allocated memory, or NULL if the
 * allocation fails.
 */
void *mem_alloc(size_t size) {
    pthread_mutex_lock(&lock);
    void *allocated = mem_alloc_no_lock(size);
    pthread_mutex_unlock(&lock);
    return allocated;
}

void mem_free_no_lock(void *block) {
    if (!block) return;

    // Get memory block to free
    MemoryBlock *previous = NULL;
    MemoryBlock *current = memory_head;
    while (current && current->start != block) {
        previous = current;
        current = current->next;
    }

    // Return if memory block was not found
    if (!current) return;

    if (previous)
        previous->next = current->next;
    else
        memory_head = current->next;

    free(current);
}

/**
 * @brief Frees the specified block of memory.
 *
 * @param block A pointer to the start of the memory block.
 */
void mem_free(void *block) {
    pthread_mutex_lock(&lock);
    mem_free_no_lock(block);
    pthread_mutex_unlock(&lock);
}

/**
 * @brief Changes the size of the memory block, possibly moving it.
 *
 * @param block A pointer to the start of the memory block.
 * @param size The new size of the memory block.
 * @return A pointer to the start of the resized memory block, or NULL if the
 * resize fails.
 */
void *mem_resize(void *block, size_t size) {
    if (size == 0) {
        if (block) mem_free(block);
        return NULL;
    }

    if (!block) return mem_alloc(size);

    pthread_mutex_lock(&lock);

    // Get memory block to free
    MemoryBlock *current = memory_head;
    while (current && current->start != block) current = current->next;
    if (!current) {
        pthread_mutex_unlock(&lock);
        return NULL;
    }

    size_t current_size = current->end - current->start;

    // Free and try to allocate with new size
    mem_free_no_lock(block);
    void *new_block = mem_alloc_no_lock(size);
    if (!new_block) {
        mem_alloc_no_lock(current_size);
        pthread_mutex_unlock(&lock);
        return NULL;
    }

    // If resize succeeded, move the memory
    size_t new_size = (size <= current_size) ? size : current_size;
    if (new_block != block) memcpy(new_block, block, new_size);
    pthread_mutex_unlock(&lock);
    return new_block;
}

/**
 * @brief Deinitializes the memory manager previously initialized with
 * `mem_init`.
 */
void mem_deinit() {
    pthread_mutex_lock(&lock);
    free(memory);

    while (memory_head) {
        MemoryBlock *temp = memory_head;
        memory_head = memory_head->next;
        free(temp);
    }

    memory_size = 0;
    pthread_mutex_unlock(&lock);
    pthread_mutex_destroy(&lock);
}
