#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include "cpen212alloc.h"

typedef struct {
    void *start; // head of heap
    void *end;   // end of heap
    void *free;  // next free address on heap - for explicit
} alloc_state_t;

typedef struct {
    uint64_t header; // header of block
} block_t;

size_t get_payload_size(block_t* block){
    return (size_t)((block->header >> 1) - 16);
}

size_t round_up(size_t size){
    return (size_t)8*((size+7+16)/8);
}

size_t get_block_size(block_t* block){
    return block->header >> 1;
}

uint64_t get_alloc(block_t* block) {
    return block->header % 2;
}

void set_header(block_t* block, size_t block_size, uint64_t block_state) {
    block->header = (((uint64_t)block_size) << 1) + block_state;
}

void set_footer(block_t* block, size_t block_size, uint64_t block_state) {
  //  printf("block %p\n", block);
  //  printf("block size %ld\n", block_size);
  //  printf(" foottrt %p\n", (((char*)block) + block_size - 8));
    uint64_t* footer = (uint64_t*)(((char*)block + block_size) - 8);
  //  printf("footer address: %p\n", footer);
    *footer = ((uint64_t)block_size << 1) + block_state; 
}

block_t* payload_to_header(void* p) {
    return (block_t*)(p - 8);
}

block_t* footer_to_header(uint64_t* footer) {
    size_t size = *footer >> 1;
   // printf("size: %ld\n", size);
    return (block_t*)((char*) footer + 8 - size);
}

void* header_to_payload(block_t* block) {
    return (void*)((char*)block + 8);
}

void* get_prev_footer(block_t* block) {
    return (void*)((char*)block - 8);
}

block_t *get_next(block_t* block){
    return (block_t *)((char*)(block) + get_block_size(block));
}

block_t *get_prev(block_t* block){
    void* footer = get_prev_footer(block);
   // printf("footer adr: %p\n", footer);
    return footer_to_header(footer);
}



void print_heap(alloc_state_t* s) {
    block_t *b = (block_t *) s->start;
    printf("-----------------------------------------\n");
    while (get_block_size(b)>0 && (uint64_t)b < (uint64_t)s->end) {
        printf("address of header: %p", b);
        printf(" block size: %ld", get_block_size(b));
        printf(" alloc state: %ld\n", get_alloc(b));
        b = get_next(b);
    }
    printf("-----------------------------------------\n");
}


void *cpen212_init(void *heap_start, void *heap_end) {
    alloc_state_t *s = (alloc_state_t *) malloc(sizeof(alloc_state_t));
    block_t *b = (block_t *) heap_start;
    set_header(b, heap_end - heap_start, 0);
    set_footer(b, heap_end - heap_start, 0);
    s->end = heap_end;
    s->start = heap_start;
    return s;
}

void cpen212_deinit(void *s) {
    free(s);
}

void *cpen212_alloc(void *alloc_state, size_t nbytes) {
    //print_heap(alloc_state);
    alloc_state_t *s = (alloc_state_t *) alloc_state;
    if (s->start == NULL) {
        cpen212_init(s->start, s->end);
    }
    size_t block_size = round_up(nbytes);
    block_t *b = (block_t *) s->start;
    while (get_block_size(b) > 0 && (uint64_t)b + block_size <= (uint64_t)s->end) {
        size_t cur_size = get_block_size(b);
        if (get_alloc(b) == 0 && cur_size > block_size) {
            set_header(b, block_size, 1);
            set_footer(b, block_size, 1);
            block_t* pointer = (block_t *)((char*)(b) + get_block_size(b));
            set_header(pointer, cur_size - block_size, 0);
            set_footer(pointer, cur_size - block_size, 0);
            return header_to_payload(b);
            }
        else if (get_alloc(b) == 0 && cur_size == block_size) {
            set_header(b, block_size, 1);
            set_footer(b, block_size, 1);
            return header_to_payload(b);
        }
        b = get_next(b);
    }
    return NULL;
}

void cpen212_coalesce(void *alloc_state, block_t* block) {
    bool prev_alloc, next_alloc;
    //printf("enter col\n");
    alloc_state_t *s = (alloc_state_t *) alloc_state;
    //printf("heap end, %p\n", s->end);
    block_t* prev = get_prev(block);
    block_t* next = get_next(block);
    //printf("prev: %p\n", prev);
    //printf("next: %p\n", next);
    size_t cur_size = get_block_size(block);
    if (block == s->start) {
        prev = NULL;
    }
   // printf("enter judge\n");
    if ((prev == NULL) && (get_block_size(next) == 0)) {
     //   printf("enter judge1\n");
        prev_alloc = 1;
        next_alloc = 1;
    }
    else if (prev == NULL) {
    //    printf("enter judge2\n");
        prev_alloc = 1;
        next_alloc = get_alloc(next);
    }
    else if (get_block_size(next) == 0) {
      //  printf("enter judge3\n");
        prev_alloc = get_alloc(prev);
        next_alloc = 1;
    }
    else {
       // printf("enter judge4\n");
        prev_alloc = get_alloc(prev);
      //  printf("enter judge41\n");
        next_alloc = get_alloc(next);
     //   printf("enter judge42\n");
    }
    
    
   // printf("prevalloc: %d\n", prev_alloc);
  //  printf("nextalloc: %d\n", next_alloc);


    if (prev_alloc && next_alloc) {
    //    printf("enter if1\n");
        return;
    }
    else if (next_alloc) {
   //     printf("enter if2\n");
        size_t prev_size = get_block_size(prev);
        set_header(prev, prev_size + cur_size, 0);
        set_footer(prev, prev_size + cur_size, 0);
        return;
    }
    else if (prev_alloc) {
        
        set_header(block, get_block_size(next) + cur_size, 0);
        set_footer(block, get_block_size(next) + cur_size, 0);
        return;
    }
    else {
  //      printf("enter if4\n");
        size_t prev_size = get_block_size(prev);
        size_t next_size = get_block_size(next);
        set_header(prev, prev_size + cur_size + next_size, 0);
        //print_heap(alloc_state);
        set_footer(prev,prev_size + cur_size + next_size, 0);
     //    printf("enter if42\n");
        return;
    }
}

void cpen212_free(void *alloc_state, void *p) {
    //print_heap(alloc_state);
    //printf("enter free\n");
    block_t* b = payload_to_header(p);
    size_t block_size = get_block_size(b);
    set_header(b, block_size, 0);
    set_footer(b, block_size, 0);
    //printf("end free\n");
    cpen212_coalesce(alloc_state, b);
    
 }

void *cpen212_realloc(void *s, void *prev, size_t nbytes) {
    alloc_state_t *state = (alloc_state_t *) s;
    void *p;
    block_t* block = payload_to_header(prev);
    if (nbytes == 0) {
        cpen212_free(s, prev);
        return NULL;
    }
    if (prev == NULL) {
        return cpen212_alloc(s, nbytes);
    }
    p = cpen212_alloc(s, nbytes);
    if (p == NULL) {
        return NULL;
    }
    
    size_t cpsize = get_payload_size(block);
    // if (nbytes < cpsize) {
    //     cpsize = nbytes;
    // }
    memmove(p, prev, cpsize); // see WARNING below
    cpen212_free(s, prev);

    return p;
}

// WARNING: we don't know the prev block's size, so memmove just copies nbytes here.
//          this is safe only because in this dumb allocator we know that prev < p,
//          and p has at least nbytes usable. in your implementation,
//          you probably need to use the original allocation size.

bool cpen212_check_consistency(void *alloc_state) {
    alloc_state_t *s = (alloc_state_t *) alloc_state;
    return s->end > s->free;
}
