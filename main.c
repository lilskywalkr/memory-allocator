#include <stdio.h>
#include "heap.h"

#define MY_MEMORY_CHUNK_SIZE sizeof(struct memory_chunk_t)

int main() {

    printf("The memory chunk size: %d\n", (int)MY_MEMORY_CHUNK_SIZE);

    char *string = heap_calloc(4, sizeof(char));
    string[0] = 'a';
    string[1] = 'b';
    string[2] = 'c';
    string[3] = '\0';

    printf("%s\n", string);

    heap_free(string);

    return 0;
}