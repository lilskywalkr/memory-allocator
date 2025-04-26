# Custom Memory Allocation in C

## Project Overview

This project implements a custom memory allocator in C, simulating dynamic memory management through functions like `malloc`, `calloc`, `realloc`, and `free`. It utilizes a custom `sbrk` function to manage memory and implements a simple heap structure to track allocated and free memory blocks.

The goal of this project is to demonstrate how low-level memory allocation works and to develop a deeper understanding of memory management in C. It is a great way to learn about memory chunks, block allocation, and memory corruption handling.

---

## Features

- **Memory Setup**: Initializes the custom heap memory with a defined chunk size.
- **Malloc**: Custom memory allocation that returns a pointer to the requested size.
- **Calloc**: Memory allocation that also zeroes the allocated memory.
- **Realloc**: Resizes an allocated block of memory.
- **Free**: Frees a previously allocated memory block.
- **Memory Validation**: Ensures that the memory blocks are correctly allocated, free, and sized.
- **Custom `sbrk` Implementation**: A custom implementation of `sbrk` and `brk` system calls for managing heap memory.

---

## File Structure

- **main.c**: Contains the main program that demonstrates memory allocation and deallocation using the custom allocator functions.
- **heap.h**: Header file for the heap memory management functions and data structures.
- **custom_unistd.h**: Defines a custom version of `sbrk` to avoid using the standard library function, ensuring a custom heap management system.
- **heap.c**: Contains the implementation of heap memory management functions like `malloc`, `calloc`, `free`, etc.

---

## Functions

### `heap_malloc(size_t size)`
Allocates a memory block of the requested size. Returns a pointer to the allocated memory, or `NULL` if the allocation fails.

### `heap_calloc(size_t number, size_t size)`
Allocates memory for an array of `number` elements, each of the specified `size`, and initializes the memory to zero.

### `heap_realloc(void* ptr, size_t size)`
Resizes the allocated block `ptr` to the new size. If necessary, a new block is allocated, and the content from the old block is copied over.

### `heap_free(void* ptr)`
Frees the previously allocated memory block. It also attempts to merge adjacent free blocks to reduce fragmentation.

### `heap_validate(void)`
Validates the heap structure by checking for errors such as invalid chunk sizes or corrupted fences.

### `heap_clean(void)`
Cleans up the heap, resetting its state and freeing all allocated memory.

### `heap_get_largest_used_block_size(void)`
Returns the size of the largest allocated memory block.

---

## Example Usage

```c
#include <stdio.h>
#include "heap.h"

#define MY_MEMORY_CHUNK_SIZE sizeof(struct memory_chunk_t)

int main() {

    printf("The memory chunk size: %d\n", (int)MY_MEMORY_CHUNK_SIZE);

    // Allocate memory for a string
    char *string = heap_calloc(4, sizeof(char));
    string[0] = 'A';
    string[1] = 'B';
    string[2] = 'C';
    string[3] = '\0';

    // Print the string
    printf("%s\n", string);

    // Free the memory
    heap_free(string);

    return 0;
}   
```

