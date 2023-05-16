#ifndef __BUDDY_MALLOC_H
#define __BUDDY_MALLOC_H
#include <stdlib.h>

// Some important constants.
#define HEAP_SIZE 4096
#define MAGIC 0xDEADBEEF

// This struct is used as the header of an allocated block.
typedef struct ___header_t {
  size_t size;  // the number of bytes of allocated memory
  unsigned int
      magic;  // the magic number used to identify a valid allocated block
} header__t;

// This struct is used for the binary tree used which stores buddy nodes.
typedef struct __binary_tree {
  struct __binary_tree *parent;           //the parent node to itself; will be used during coalesce
  size_t size;                            // the number of bytes available in this free block
  size_t usedSpace;                       //size in binary tree node which was actually used, the rest (size - usedSpace) is internal fragmentation
  bool empty;                             //tells whether any region in the block has been allocated
  void *offset;                           //gives the corresponding offset of the start of block in heap. For eg: the offset of head node of the binary tree will be the address of the begining of the heap
  struct __binary_tree *rightChild;       // a pointer to the next free list node
  struct __binary_tree *leftChild;        // a pointer to the next free list node
} binary_t;



// This is the primary interface.
void *buddy_malloc(size_t);
void buddy_my_free(void *);

// These functions are helper functions or used for debugging and printing.
void buddy_createBinaryTreeNode(binary_t ** node);
void buddy_reset_heap();
void freeNodeAndAvailableMemoryHelper (binary_t * node);
size_t buddy_available_memory();
int buddy_number_of_free_nodes();
void buddy_print_free_list();
void buddy_find_free(size_t size, void **found, binary_t **node);
void buddy_split(binary_t **node, binary_t **leftChild,
           binary_t **rightChild);
void buddy_coalesce(binary_t *foundNode);
void buddy_findTreeNode (size_t size, binary_t * node, void * offset, binary_t ** foundNode);

#endif
