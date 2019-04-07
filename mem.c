#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "mem.h"

#include <stdio.h>

// header of a free chunk
// size: 16 bytes (64-bit machine), 8 bytes (32-bit machine)
typedef struct free_node {
    int size;
    struct free_node *next;
} free_node;

// header of an allocated chunk
// size: 16 bytes (64-bit machine), 8 bytes (32-bit machine)
typedef struct malloc_node {
    int size;
    long long magic;
} malloc_node;

// GLOBAL VARIABLES
free_node *first_free = NULL;

// CONSTANTS
#define MAGIC_NUM 6148914691236517205

// coalesces a free chunk with the next one if possible and allowed
void _mem_coalesce(free_node *ptr, int coalesce) {
    free_node *next_neighbor = ptr->next;
    if ( coalesce && next_neighbor != NULL && (void *) ptr + ptr->size + sizeof(free_node) == (void *) next_neighbor ) {
        ptr->size += sizeof(free_node) + next_neighbor->size;
        ptr->next = next_neighbor->next;
    }
}

void *Mem_Init(int sizeOfRegion) {

    // round up sizeOfRegion to be divisible by page size
    int pg_size = getpagesize();
    sizeOfRegion += pg_size - (sizeOfRegion % pg_size);

    // get sizeOfRegion bytes of memory initialized to 0
    int fd = open("/dev/zero", O_RDWR);
    void *ptr = mmap(NULL, sizeOfRegion, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    close(fd);

    // allocation failed
    if (ptr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // initialize the first free node
    first_free = (free_node *) ptr;
    first_free->size = sizeOfRegion - sizeof(free_node);
    first_free->next = NULL;

    return ptr;
}

void *Mem_Alloc(int size) {

    // error-checks
    if ( size <= 0 )
        return NULL;

    // find free node which can accomodate the request
    free_node *prev = NULL;
    free_node *node = first_free;
    for ( ; node != NULL && node->size + sizeof(free_node) < size + sizeof(malloc_node); prev = node, node = node->next );

    // none found
    if ( node == NULL )
        return NULL;

    malloc_node *allocation = (malloc_node *) node;
    int alloc_size = node->size;
    free_node *next_free = node->next;

    // split free chunk into allocated chunk and smaller free chunk
    if ( node->size >= sizeof(malloc_node) + size ) {

        // convert space
        free_node *new_free = (void *) node + size + sizeof(malloc_node);
        new_free->size = node->size - sizeof(malloc_node) - size;
        new_free->next = node->next;
        next_free = new_free;
        alloc_size = size;
    }
    
    // connect the previous free chunk to the next (or new) one
    if ( prev != NULL )
        prev->next = next_free;
    else
        first_free = next_free;
    
    // prepare header for the new allocation
    allocation->size = alloc_size;
    allocation->magic = MAGIC_NUM;

    return (void *) (allocation + 1);
}

int Mem_Free(void *ptr, int coalesce) {

    // nothing to free
    if ( ptr == NULL )
        return 0;    
    
    // check head validity
    malloc_node *header = ptr - sizeof(malloc_node);
    int alloc_size = header->size;
    if ( header->magic != MAGIC_NUM )
        return -1;

    // there is no free memory chunk before ptr
    if ( first_free == NULL || (void *) first_free > (void *) header ) {
        
        // create free chunk header
        free_node *new_free = (free_node *) header;
        new_free->size = alloc_size + sizeof(malloc_node) - sizeof(free_node);
        new_free->next = first_free;
        first_free = new_free;

        // coalesce with next free chunk if allowed and possible
        _mem_coalesce(first_free, coalesce);
        
    } else {
        
        // find a free memory chunk before ptr
        free_node *prev = first_free;
        for ( ; prev != NULL && prev->next != NULL && (void *) prev->next < (void *) header; prev = prev->next );
        
        // create free chunk header
        free_node *new_free = (free_node *) header;
        new_free->size = alloc_size + sizeof(malloc_node) - sizeof(free_node);
        new_free->next = prev->next;
        prev->next = new_free;

        // coalesce  with next free chunk if allowed and possible
        _mem_coalesce(new_free, coalesce);

        // coalesce with previous free chunk if allowed and possible
        _mem_coalesce(prev, coalesce);

    }

    return 0;
}

void Mem_Dump() {
    // print stats on all free memory chunks
    free_node *node = first_free;
    printf("Free nodes:\n");
    int i = 0;
    while ( node != NULL ) {
        printf("%d. Header addr: %p, size: %d, next: %p, data addr: %p\n", ++i, (void *) node, node->size, (void *) node->next, (void *) (node + 1));
        node = node->next;
    }
    printf("\n");
}
