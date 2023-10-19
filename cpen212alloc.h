#ifndef __CPEN212ALLOC_H__
#define __CPEN212ALLOC_H__

#include <stdlib.h>
#include <stdbool.h>

// description:
// - initialize an allocator
// arguments:
// - heap_start: *heap_start is in the heap area you manage but *(heap_start-1) is not;
//   aligned on an 8-byte boundary
// - heap_end: *(heap_end-1) is in the heap area you manage but *heap_end is not
//   aligned on an 8-byte boundary
// returns:
// - an allocator state pointer that will be passed unchanged to any alloc functions
//   (may be NULL if your allocator is stateless)
void *cpen212_init(void *heap_start, void *heap_end);

// description:
// - free all resources used by an allocator
// - called once per cpen212_init(), after all calls to cpen212_alloc(), cpen212_free(),
//   and cpen212_realloc() that use this allocator
// arguments:
// - alloc_state: the pointer returned by your cpen212_init()
void cpen212_deinit(void *alloc_state);

// description:
// - allocate a block of memory
// arguments:
// - alloc_state: the pointer returned by your cpen212_init()
// - nbytes: a minimum number of bytes to allocate (may be 0)
// returns:
// - pointer p to the allocated block of at least nbytes bytes
//   (p may be NULL if nbytes was 0 or if heap is full)
// output invariants:
// - p must be either in [heap_start,heap_end) as passed to cpen212_init, or NULL
// - if p is not NULL, P+nbytes must be in (heap_start,heap_end] as passed to cpen212_init
// - p must be aligned on an 8-byte boundary
// - [p,p+nbytes) may be overwritten by the caller
void *cpen212_alloc(void *alloc_state, size_t nbytes);

// description:
// - free a previously allocated a block of memory
// arguments:
// - alloc_state: the pointer returned by your cpen212_init()
// - p: a pointer previously returned from cpen212_alloc() and never freed
void cpen212_free(void *alloc_state, void *p);

// description:
// - extend or shrink allocated block size, moving the block address and contents if necessary
// arguments:
// - alloc_state: the pointer returned by your cpen212_init()
// - prev: a pointer previously returned from cpen212_alloc() and never freed,
//         or NULL (in which case realloc behaves like alloc with the same size)
// returns:
// - pointer p to the allocated block of at least nbytes bytes,
//   with a copy of the previous block at prev if prev is not NULL
//   (p may be NULL if nbytes was 0 or if heap is full)
// output invariants:
// - p must be either in [heap_start,heap_end) as passed to cpen212_init, or NULL
// - if p is not NULL, P+nbytes must be in (heap_start,heap_end] as passed to cpen212_init
// - p must be aligned on an 8-byte boundary
// - [p,p+nbytes) may be overwritten by the caller
// - if prev is not NULL, the full contents of the block at prev
//   must be copied to the beginning of the block at p
//   (or nbytes of the previous contents if shrinking)
void *cpen212_realloc(void *alloc_state, void *prev, size_t nbytes);

// description:
// - checks the heap for consistency
// - the invariants this checks are entirely up to you;
//   we only care that it returns true or false
// arguments:
// - alloc_state: the pointer returned by your cpen212_init()
// returns:
// - true iff the heap has passed invariant checks
bool cpen212_check_consistency(void *alloc_state);


#endif // __CPEN212ALLOC_H__
