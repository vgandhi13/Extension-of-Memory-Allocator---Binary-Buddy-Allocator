# Memory Allocator Project

Kindly watch the recording of the presentation before looking at the README.md file to better understand the implementation through visual representations and explanation.

Presentation Video Link:\
Presentation Slides Link: https://docs.google.com/presentation/d/1JwD6ZSrCvl3qAM70vlp1tFPzyfzNBees4rMeHBCoImU/edit?usp=sharing

## Project 5 Memory Allocator Extension

The first thing I did was extend the Project 5 memory allocator by making the heap expandable and modifying `coalesce()` to merge ALL adjacent free blocks regardless of the list structure. The following components were updated to account for this:

- `heap` function
- `coalesce` function
- `my_malloc` function
- `find_free` function

All the other functions remain untouched. Additionally, I added a function named `findLastNode` which finds the last node of the free list. Instead of using the `mmap` system call to allocate pages from the OS, I used the `sbrk` system call. The reason for this choice is that `sbrk` provides the process with pages contiguous in memory address on each `sbrk` call. For a detailed explanation and visual representation, please refer to the slides linked below. The code for this extension can be found in `my_malloc.h` and `my_malloc.cpp`.

## Binary Buddy Algorithm

The second thing I worked on was the Binary Buddy Algorithm. It is a method of memory allocation where available space is repeatedly split into halves, known as buddies. It keeps splitting buddies until it chooses the smallest possible block that can contain the size of memory requested. It allows for easy memory allocation and deallocation, but it requires the unit of space to be a power of two and may result in significant internal fragmentation. The code for this algorithm can be found in `buddy_malloc.h` and `buddy_malloc.cpp`.

In my implementation, I replaced the `node_t` struct in the memory allocator project with a `binary_t` struct, which is used to create a binary tree. Unlike `node_t`, the `binary_t` nodes of the tree have their own designated memory. Initially, the heap available to a program/user is completely empty from top to bottom with no header. The binary tree starts with only one node, the head, which has a `void*` type member called "offset" that points to the address of the start of this heap. The `mmap` system call is made each time a node is created in the binary tree.

I chose this implementation because, as mentioned earlier, each block of memory that is split requires the unit of space to be a power of two, and using `binary_t` headers in the heap for free nodes would not guarantee this.

The implementation consists of two primary functions: one to provide the address of the memory allocated to the user/program and another to free the allocated memory. The interface also includes other functions used for debugging purposes or as helper functions. Most of the functions are recursive and utilize the Depth First Search algorithm to find free nodes in the binary tree.

### More about the Binary Tree created using `binary_t` struct:

- The free nodes in the binary tree will always be leaf nodes, but a leaf node won't necessarily be free if it has been allocated.
- All nodes have an `offset` member which points to an address in the heap available to the user/program.
- When a binary tree node is split, two new nodes are created, and the left and right pointers of the current node point to them.
  - The left child's `offset` points to the same address as the current node's `offset`.
  - The offset of the right child is the current node's `offset` + half of the actual size allocated to this node. For example, if the size of the current node is 1024 and its `offset` member points to 0x0, when this node is split, its left child will have `offset` point to 0x0, and the right child will have an `offset` point to 0x512.

## Running the Code

To run the code, follow these steps:

1. Clone the repository.
2. Build the code by running `make`.
3. Execute the command `./allocator_app`.
4. The results for both `my_malloc.cpp` and `buddy_malloc.cpp` will be displayed.

If you have any questions regarding this project, feel free to email me at vgandhi@umass.edu.
