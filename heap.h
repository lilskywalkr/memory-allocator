//
// Created by ronald on 27.04.2025.
//

#ifndef HEAP_H
#define HEAP_H

#include <unistd.h>
#include <stdint-gcc.h>

enum pointer_type_t
{
    pointer_null,
    pointer_heap_corrupted,
    pointer_control_block,
    pointer_inside_fences,
    pointer_inside_data_block,
    pointer_unallocated,
    pointer_valid
};

struct memory_manager_t
{
    void *memory_start;
    size_t memory_size;
    struct memory_chunk_t *first_memory_chunk;
};

struct memory_chunk_t
{
    struct memory_chunk_t* prev;
    struct memory_chunk_t* next;
    size_t size;
    int free;
    uint32_t chunk_size;
};

int heap_setup(void);
void heap_clean(void);

void* heap_malloc(size_t size);
void* heap_calloc(size_t number, size_t size);
void* heap_realloc(void* memblock, size_t count);
void  heap_free(void* memblock);

size_t   heap_get_largest_used_block_size(void);

enum pointer_type_t get_pointer_type(const void* const pointer);

int heap_validate(void);
void heap_set_chunk_size(struct memory_chunk_t *pointer);
void heap_draw_fences(struct memory_chunk_t *pointer);

#endif //HEAP_H
