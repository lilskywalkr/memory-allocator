//
// Created by ronald on 27.04.2025.
//

#include "heap.h"
#include "custom_unistd.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct memory_manager_t my_memory;

#define MY_MEMORY_CHUNK_SIZE sizeof(struct memory_chunk_t)
#define FENCES 2

void heap_set_chunk_size(struct memory_chunk_t *pointer) {
    uint32_t chunk_size = 0;

    for(size_t i = 0; i < MY_MEMORY_CHUNK_SIZE - sizeof(uint32_t); i++) {
        chunk_size += (uint32_t)(*((uint8_t *)pointer+i));
    }

    pointer->chunk_size = chunk_size;
}

int heap_setup(void) {
    void *current_brk = custom_sbrk(0);

    if (*(int *)(current_brk) == -1) {
        return -1;
    }

    my_memory.memory_start = current_brk;
    my_memory.memory_size = 0;
    my_memory.first_memory_chunk = NULL;

    return 0;
}

void heap_draw_fences(struct memory_chunk_t *pointer) {
    for (size_t i = 0; i < FENCES; i++) {
        *((uint8_t *) pointer + i + sizeof(struct memory_chunk_t)) = '#';
        *((uint8_t *) pointer + i + pointer -> size + MY_MEMORY_CHUNK_SIZE + FENCES) = '#';
    }
}

void* heap_malloc(size_t size) {
    // Check for invalid size
    if (size <= 0) {
        return NULL;
    }

    // Iterate through the my_memory chunks to find a suitable free block
    struct memory_chunk_t *current_chunk = my_memory.first_memory_chunk;

    /*If the memory list is empty*/
    if (current_chunk == NULL) {
        void *tmp = custom_sbrk(MY_MEMORY_CHUNK_SIZE + size + (FENCES * 2));
        if (tmp == (void *)-1) {
            return NULL;
        }

        my_memory.first_memory_chunk = tmp;
        my_memory.memory_size = MY_MEMORY_CHUNK_SIZE + size + (FENCES * 2);
        my_memory.memory_start = tmp;

        my_memory.first_memory_chunk->next = NULL;
        my_memory.first_memory_chunk->prev = NULL;

        my_memory.first_memory_chunk->size = size;
        my_memory.first_memory_chunk->free = 0;

        heap_draw_fences(my_memory.first_memory_chunk);
        heap_set_chunk_size(my_memory.first_memory_chunk);

        return (struct memory_chunk_t *)((uint8_t *) (my_memory.first_memory_chunk) + FENCES + MY_MEMORY_CHUNK_SIZE);
    }

    /*Getting the last but one chunk*/
    while (current_chunk -> next != NULL) {
        /*If there's a free chunk with sufficient space*/
        if (current_chunk -> size >= size && current_chunk -> free == 1) {
            current_chunk -> free = 0;

            if (current_chunk -> size - size > MY_MEMORY_CHUNK_SIZE + FENCES * 2) {
                struct memory_chunk_t *new_free_chunk = (struct memory_chunk_t *) ((uint8_t *) current_chunk + size + MY_MEMORY_CHUNK_SIZE + FENCES * 2);
                new_free_chunk ->size = current_chunk -> size - size - MY_MEMORY_CHUNK_SIZE - FENCES * 2;
                new_free_chunk -> free = 1;

                new_free_chunk -> prev = current_chunk;

                if (current_chunk -> next != NULL) {
                    current_chunk -> next -> prev = new_free_chunk;
                }

                new_free_chunk -> next = current_chunk-> next;
                current_chunk -> next = new_free_chunk;

                heap_set_chunk_size(new_free_chunk);
                if (new_free_chunk -> next != NULL) {
                    heap_set_chunk_size(new_free_chunk -> next);
                }
            }

            current_chunk -> size = size;

            heap_set_chunk_size(current_chunk);
            heap_draw_fences(current_chunk);
            heap_set_chunk_size(current_chunk -> next);

            return (struct memory_chunk_t *)((uint8_t *) (current_chunk) + FENCES + MY_MEMORY_CHUNK_SIZE);
        }

        current_chunk = current_chunk -> next;
    }

    while (current_chunk -> next != NULL) {
        current_chunk = current_chunk -> next;
    }

    void *tmp = custom_sbrk(MY_MEMORY_CHUNK_SIZE + size + (FENCES * 2));
    if (tmp == (void *)-1) {
        return NULL;
    }

    current_chunk -> next = tmp;
    current_chunk -> next -> size = size;
    current_chunk -> next -> free = 0;
    current_chunk -> next -> prev = current_chunk;
    current_chunk -> next -> next = NULL;

    heap_set_chunk_size(current_chunk);
    heap_draw_fences(current_chunk -> next);
    heap_set_chunk_size(current_chunk -> next);

    my_memory.memory_size += MY_MEMORY_CHUNK_SIZE + size + (FENCES * 2);

    return (struct memory_chunk_t *)((uint8_t *) (current_chunk -> next) + FENCES + MY_MEMORY_CHUNK_SIZE);
}

void* heap_calloc(size_t number, size_t size) {
    if (number <= 0 || size <= 0) {
        return  NULL;
    }

    /*Allocating number * size bytes*/
    void *ptr = heap_malloc(number * size);
    if (ptr == NULL) {
        /*If the allocation failed*/
        return NULL;
    }

    /*Cleaning the pointer*/
    memset(ptr, 0, size*number);

    return ptr;
}

void* heap_realloc(void* memblock, size_t count) {
    if ((memblock == NULL && count <= 0) || heap_validate()) {
        return NULL;
    }

    /*If memblock is null - allocate*/
    if (memblock == NULL) {
        void *ptr = heap_malloc(count);
        return ptr;
    }

    /*If memblock is not null and count is 0*/
    if (count == 0) {
        heap_free(memblock);
        return NULL;
    }

    /*If the pointer is unallocated*/
    if(get_pointer_type(memblock) == pointer_unallocated) {
        return memblock;
    }

    /*If the pointer is not valid*/
    if(get_pointer_type(memblock) != pointer_valid) {
        return NULL;
    }

    struct memory_chunk_t* memory_chunk = (struct memory_chunk_t*)((uint8_t*)memblock - MY_MEMORY_CHUNK_SIZE - FENCES);

    /*If the count is equal to memblock size*/
    if (memory_chunk -> size == count) {
        return memblock;
    }

    /*Decreasing the allocated memory*/
    if (memory_chunk -> size > count) {
        if (memory_chunk -> size - count > MY_MEMORY_CHUNK_SIZE + FENCES * 2) {
            struct memory_chunk_t *new_free_chunk = (struct memory_chunk_t *) ((uint8_t *) memory_chunk + count + MY_MEMORY_CHUNK_SIZE + FENCES * 2);
            new_free_chunk -> size = memory_chunk -> size - count - MY_MEMORY_CHUNK_SIZE - FENCES * 2;
            new_free_chunk -> free = 1;

            new_free_chunk -> prev = memory_chunk;

            if (memory_chunk -> next != NULL) {
                memory_chunk -> next -> prev = new_free_chunk;
            }

            new_free_chunk -> next = memory_chunk-> next;
            memory_chunk -> next = new_free_chunk;

            if (new_free_chunk -> next != NULL && new_free_chunk -> next ->free == 1) {
                // Merge the two adjacent free chunks
                new_free_chunk->size += new_free_chunk->next->size + MY_MEMORY_CHUNK_SIZE + FENCES * 2;
                new_free_chunk->next = new_free_chunk->next->next;

                if (new_free_chunk->next != NULL) {
                    new_free_chunk->next->prev = new_free_chunk;
                }

                heap_set_chunk_size(new_free_chunk);
            }
            if (new_free_chunk->next != NULL) {
                heap_set_chunk_size(new_free_chunk->next);
            }

            heap_set_chunk_size(new_free_chunk);
        }

        memory_chunk -> size = count;

        heap_set_chunk_size(memory_chunk);
        heap_draw_fences(memory_chunk);
        if (memory_chunk -> next != NULL) {
            heap_set_chunk_size(memory_chunk -> next);
        }
        return (struct memory_chunk_t *)((uint8_t *) (memory_chunk) + FENCES + MY_MEMORY_CHUNK_SIZE);
    }

    /*Increasing allocated memory*/
    if (memory_chunk -> size < count) {
        /*If the next chunks exists and is free*/
        if (memory_chunk -> next != NULL && memory_chunk -> next -> free == 1) {
            /*If two chunks with the next free chunk are sufficient for the new memory*/
            if (memory_chunk -> size + memory_chunk -> next -> size + MY_MEMORY_CHUNK_SIZE >= count) {
                /*If two merged chunks are too big for the new memory*/
                if (memory_chunk -> size + memory_chunk -> next -> size - count + MY_MEMORY_CHUNK_SIZE > MY_MEMORY_CHUNK_SIZE) {
                    memory_chunk -> next -> size = memory_chunk -> next -> size + memory_chunk -> size - count;
                    heap_set_chunk_size(memory_chunk -> next);

                } else {
                    if (memory_chunk -> next -> next != NULL) {
                        memory_chunk -> next -> next -> prev = memory_chunk;
                    }

                    memory_chunk -> next = memory_chunk -> next -> next;
                }

                // Merge the two adjacent free chunks
                memory_chunk->size = count;
                memory_chunk->next = memory_chunk->next->next;

                heap_set_chunk_size(memory_chunk);
                heap_draw_fences(memory_chunk);

                if (memory_chunk->next != NULL) {
                    heap_set_chunk_size(memory_chunk->next);
                    heap_draw_fences(memory_chunk->next);
                }

                return (struct memory_chunk_t *)((uint8_t *) (memory_chunk) + FENCES + MY_MEMORY_CHUNK_SIZE);
            }
        }

        if (memory_chunk -> next == NULL) {
            void *ptr = custom_sbrk(count - memory_chunk -> size);
            if (ptr == (void *)-1) {
                return NULL;
            }

            my_memory.memory_size += count - memory_chunk -> size;
            memory_chunk -> size += count - memory_chunk -> size;

            heap_draw_fences(memory_chunk);
            heap_set_chunk_size(memory_chunk);

            return (struct memory_chunk_t *)((uint8_t *) memory_chunk + FENCES + MY_MEMORY_CHUNK_SIZE);
        }

        void *ptr = heap_malloc(count);
        if (ptr == NULL) {
            return NULL;
        }

        struct memory_chunk_t *new_chunk = (struct memory_chunk_t*)((uint8_t*)ptr - MY_MEMORY_CHUNK_SIZE - FENCES);
        memcpy((uint8_t *)new_chunk + MY_MEMORY_CHUNK_SIZE + FENCES, (uint8_t *)memory_chunk + MY_MEMORY_CHUNK_SIZE + FENCES, memory_chunk -> size);
        heap_free((uint8_t *)memory_chunk + MY_MEMORY_CHUNK_SIZE + FENCES);

        return (struct memory_chunk_t *)((uint8_t *) new_chunk + FENCES + MY_MEMORY_CHUNK_SIZE);;
    }


    return memblock;
}

void heap_free(void* memblock) {
    if (memblock == NULL) {
        return;
    }

    /* Searching the memblock in the list */
    for (struct memory_chunk_t* current_chunk = my_memory.first_memory_chunk; current_chunk != NULL; current_chunk = current_chunk->next) {
        struct memory_chunk_t* memory_chunk = (struct memory_chunk_t*)((uint8_t*)memblock - MY_MEMORY_CHUNK_SIZE - FENCES);
        /* If the chunk is memblock */
        if (memory_chunk == current_chunk) {
            memory_chunk->free = 1;
            heap_set_chunk_size(current_chunk);
            break; // No need to continue searching once we find the block
        }
    }

    for (int i = 0; i < 7; ++i) {
        /* Looking for free chunks that are next to each other and merging them */
        for (struct memory_chunk_t* current_chunk = my_memory.first_memory_chunk; current_chunk != NULL && current_chunk->next != NULL; current_chunk = current_chunk->next) {
            if (current_chunk->free && current_chunk->next->free) {
                // Merge the two adjacent free chunks
                current_chunk->size += current_chunk->next->size + MY_MEMORY_CHUNK_SIZE + FENCES * 2;
                current_chunk->next = current_chunk->next->next;

                if (current_chunk->next != NULL) {
                    current_chunk->next->prev = current_chunk;
                }

                heap_set_chunk_size(current_chunk);

                if (current_chunk->next != NULL) {
                    heap_set_chunk_size(current_chunk->next);
                }
            }
        }
    }
}


enum pointer_type_t get_pointer_type(const void* const pointer) {
    if (pointer == NULL) {
        return pointer_null;
    }

    for (struct memory_chunk_t *current_chunk = my_memory.first_memory_chunk; current_chunk != NULL; current_chunk = current_chunk->next) {
        if(((uint8_t*)current_chunk <= (uint8_t*)pointer) && ((uint8_t*)current_chunk + MY_MEMORY_CHUNK_SIZE + current_chunk->size + FENCES * 2 > (uint8_t*)pointer)){
            /*If the pointer points to unallocated memory space*/
            if (current_chunk->free == 1) {
                return pointer_unallocated;
            }

            /*If the pointer points to control block (my_memory)*/
            if ((uint8_t *)pointer < (uint8_t *)current_chunk + MY_MEMORY_CHUNK_SIZE && (uint8_t *)current_chunk <= (uint8_t *)pointer) {
                return pointer_control_block;
            }

            /*If the pointer points to fences in my_memory*/
            if ((uint8_t *)pointer < (uint8_t *)current_chunk + MY_MEMORY_CHUNK_SIZE + current_chunk->size + FENCES * 2 &&
                (uint8_t *)pointer >= (uint8_t *)current_chunk + MY_MEMORY_CHUNK_SIZE + current_chunk->size + FENCES ) {
                return pointer_inside_fences;
            }

            if ((uint8_t *)pointer < (uint8_t *)current_chunk + MY_MEMORY_CHUNK_SIZE + FENCES &&
                (uint8_t *)pointer >= (uint8_t *)current_chunk + MY_MEMORY_CHUNK_SIZE) {
                return pointer_inside_fences;
            }

            /*If the pointer points to outside of fences*/
            if ((uint8_t *)pointer < (uint8_t *)current_chunk + MY_MEMORY_CHUNK_SIZE + FENCES + current_chunk->size &&
                (uint8_t *)pointer > (uint8_t *)current_chunk + MY_MEMORY_CHUNK_SIZE + FENCES) {
                return pointer_inside_data_block;
            }
        }
    }

    return pointer_valid;
}

void heap_clean(void) {
    intptr_t heap_cleaning = (intptr_t)(-1 * my_memory.memory_size);
    custom_sbrk(heap_cleaning);

    my_memory.memory_start = NULL;
    my_memory.first_memory_chunk = NULL;
}

int heap_validate(void) {
    /*If the stack is uninitiated*/
    if (my_memory.memory_start == NULL) {
        return 2;
    }

    /*Validating whether the control structures are damaged*/
    uint32_t chunk_size = 0;
    struct memory_chunk_t *chunk = my_memory.first_memory_chunk;

    while (chunk != NULL) {
        for(size_t i = 0; i < MY_MEMORY_CHUNK_SIZE - sizeof(uint32_t); i++) {
            chunk_size += (uint32_t)(*((uint8_t *)chunk+i));
        }

        if(chunk_size != chunk -> chunk_size) {
            return 3;
        }

        chunk_size = 0;
        chunk = chunk -> next;
    }

    /*Validating whether the fences are damaged*/
    chunk = my_memory.first_memory_chunk;
    while (chunk != NULL) {
        for(int i = 0; i < FENCES; i++) {
            if((chunk -> free != 1) && (*((uint8_t *)chunk + MY_MEMORY_CHUNK_SIZE + i) != '#' || *((uint8_t *)chunk+ MY_MEMORY_CHUNK_SIZE + i + FENCES + chunk -> size) != '#')) {
                return 1;
            }
        }

        chunk = chunk -> next;
    }

    return 0;
}

size_t heap_get_largest_used_block_size(void) {
    if (heap_validate() != 0) {
        return 0;
    }

    size_t size = 0;
    for (struct memory_chunk_t *chunk = my_memory.first_memory_chunk; chunk != NULL; chunk = chunk -> next) {
        if (chunk -> free == 0 && chunk -> size > size) {
            size = chunk -> size;
        }
    }

    return size;
}
