#include <assert.h>
#include <my_malloc.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <iostream>
using namespace std;

// A pointer to the head of the free list.
node_t *head = NULL;
node_t *new_head = NULL;
bool notFound = false;


// gives the last node in the free list
//updates the last_node pointer with it
void findLastNode(node_t ** last_node) {
    node_t * headCpy = head;
    while (headCpy != NULL) {
        *last_node = headCpy;
        headCpy = headCpy->next;
      }
}

// The heap function returns the head pointer to the free list. If the heap
// has not been allocated yet (head is NULL) it will use mmap to allocate
// a page of memory from the OS and initialize the first free node.
node_t *heap() {
  if (head == NULL || notFound == true) { //only entered when the free list is empty or when on iterating through
                                          //the free list in find free nothing was found
    //sbrk function in C is used to dynamically adjust the program's data segment by incrementing or 
    //decrementing the program's break value. It allows you to allocate or deallocate memory directly from the 
    //operating system.
    new_head = (node_t *)sbrk(HEAP_SIZE);
    if (head == NULL) { //if head is empty when the current free list is empty
      head = new_head;
    }
    else {//free list exists
      //last node in in free list 
      node_t * last_node = NULL;
      findLastNode(&last_node);
      //if last node's next is not NULL, .next is new_head else increase the size of the last node 
      void * endOfFreeList = (void *)(((char *)(last_node)) + (last_node)->size + sizeof(node_t)); //give the address of the end of free list
      void * beginningOfExpansion = (void *)(new_head);   //gives the address of the start of newly expanded heap
      if(endOfFreeList == beginningOfExpansion) { // if it is true, this would mean the last free list node is directly touching the newly expanded block
          //in this case we just expand the current last node
          //in this case we would expand the last node of the current free list
          size_t heap_size = 4096;
          (last_node)->size = (last_node)->size + heap_size;  //updating the size of the last free node to accomodate the size of newly expanded heap
          cout<<"Not Enough Memory"<<endl;
          cout << "Heap expanded by 4096 bytes" << endl;
          notFound = false;
          return head;
      }
      else {
          //in this case after the block of last node which was free, there is some block which was allocated to the user by my_malloc
         (last_node)->next = new_head;  //changing next of the current Last Node from NULL to the begining of this new head
      }
    }
    new_head->size = HEAP_SIZE - sizeof(node_t);
    new_head->next = NULL;
    notFound = false; //setting it back to false
  }

  return head;
}


// Returns a pointer to the head of the free list.
node_t *free_list() { return head; }

// Calculates the amount of free memory available in the heap.
size_t available_memory() {
  size_t n = 0;
  node_t *p = head;
  while (p != NULL) {
    n += p->size;
    p = p->next;
  }
  return n;
}

// Returns the number of nodes on the free list.
int number_of_free_nodes() {
  int count = 0;
  node_t *p = head;
  while (p != NULL) {
    count++;
    p = p->next;
  }
  return count;
}

// Prints the free list. Useful for debugging purposes.
void print_free_list() {
  node_t *p = head;
  while (p != NULL) {
    printf("Free(%zd)", p->size);
    p = p->next;
    if (p != NULL) {
      printf("->");
    }
  }
  printf("\n");
}

// Finds a node on the free list that has enough available memory to
// allocate to a calling program. This function uses the "first-fit"
// algorithm to locate a free node.
//
// PARAMETERS:
// size - the number of bytes requested to allocate
//
// RETURNS:
// found - the node found on the free list with enough memory to allocate
// previous - the previous node to the found node
//
void find_free(size_t size, node_t **found, node_t **previous) {
  node_t * headCpy = heap();
  while (headCpy != NULL) {
    if ((headCpy->size + sizeof(node_t)) >= (size + sizeof(header_t))) {
      *found = headCpy;
      return;
    }
    else {
      *previous = headCpy;
      headCpy = headCpy->next;
    }
  }
  //this point only reached when the free list does not have a big enough node
  notFound = true;
}

// Splits a found free node to accommodate an allocation request.
//
// The job of this function is to take a given free_node found from
// `find_free` and split it according to the number of bytes to allocate.
// In doing so, it will adjust the size and next pointer of the `free_block`
// as well as the `previous` node to properly adjust the free list.
//
// PARAMETERS:
// size - the number of bytes requested to allocate
// previous - the previous node to the free block
// free_block - the node on the free list to allocate from
//
// RETURNS:
// allocated - an allocated block to be returned to the calling program
//
void split(size_t size, node_t **previous, node_t **free_block,
           header_t **allocated) {
  assert(*free_block != NULL);
  // TODO
  node_t * currNext = (*free_block)->next;
  size_t actual_size = size + sizeof(header_t);
  size_t remainingSize = ((*free_block)->size) - actual_size;
  node_t *free_block_copy = *free_block;

  if (remainingSize > 0) { //if the remaining size is atleast 1, only then there is a point of having  it in free list
                            // else leave the remaining space as internal fragmentation - no need to even try to create a free list node for it, even if there is exact space for a header of node list
    *free_block = (node_t *)(((char *)*free_block) + actual_size);
    (*free_block)->size = remainingSize;
    (*free_block)->next = currNext;
  }
  
  if (*previous == NULL) {
      if (currNext == NULL && (remainingSize == 0)) {  //what about internal fragmentation  //when the block is completely used up and is the last block
        head = NULL;  //when the entire free list is used up what do we set head to?
      }
      else if ((remainingSize == 0) && currNext != NULL)
      {
        head = currNext;
      }
      else { //remaining != 0 && currnext !=null or even when remainng != 0 && currnext ==null
        head = *free_block;
      }
  }
  else {
      if (remainingSize != 0) {
        (*previous)->next = *free_block;
      }
      else {
        (*previous)->next = currNext;
      }
  }

  *allocated = (header_t *)(free_block_copy);  //assigns the first sizeof(header_t) bytes to the struct
  //header is now pointing to the complete start of free block, where we had node_t earlier
  (*allocated)->size = size;
  (*allocated)->magic = MAGIC;
}

// Returns a pointer to a region of memory having at least the request `size`
// bytes.
//
// PARAMETERS:
// size - the number of bytes requested to allocate
//
// RETURNS:
// A void pointer to the region of allocated memory
//
void *my_malloc(size_t size) {
  // TODO
  node_t * previous = NULL;
  node_t * found = NULL;

  while (found == NULL) {   //while a big enough free_block not found keep on expanding the heap, on the second iteration of this, the global variable notFound will be true
    previous = NULL;
    find_free(size, &found, &previous);
  }

  header_t * allocated = NULL;
  split(size, &previous, &found, &allocated);

  return (void *)(((char *)(allocated)) + sizeof(header_t));
}

// Merges adjacent nodes on the free list to reduce external fragmentation.
//
// This function will handle coaelsce of all adjacent free nodes, regardless of their order in the free list
//  This is an O(N^2) soln
//
// PARAMETERS:
// free_list_start - the starting node on the free list to coalesce
//
void coalesce(node_t *free_list_start) { //free list start is the head of the free list
  // TODO
  while(free_list_start != NULL) { //for each node in the free list
    node_t * headCpy = head;
    bool changeMade = false;
    node_t * prev = NULL;
    while (headCpy!= NULL) {  //for each node in the free list
      if ((((char*)(free_list_start))+ free_list_start->size + sizeof(node_t)) == ((char*) (headCpy))) { //if the next block direct at the end of curr free block ie empty and has the same address as the block headCpy
        free_list_start->size = free_list_start->size + headCpy->size + sizeof(node_t); //update the size of the freelist block with the size of the adjacent free block
        if (prev!=NULL) {
          prev->next = headCpy->next;     //since we coellasce headCpy, and it does not have a next pointer anymore, we assign its next to previous's next
        }
        else {
          head = headCpy->next;//since headCpy (which is the head of free list as prev is null) is now coeallesed with free_list_start curr node
          prev= NULL;          //keep prev as same
        }
        headCpy = headCpy->next;
        changeMade = true;
        continue;
      }
      prev = headCpy;
      headCpy = headCpy->next;
    }
    if (changeMade == true) {
      continue; //do not update free_list_start to free_list_start->next, 
      //Reason: The last block added to current free_list_start node could potentially have an adjacent subsequent node in headCpy free list
    }
    else {
      free_list_start = free_list_start->next;
    }
  }
}

// Frees a given region of memory back to the free list.
//
// PARAMETERS:
// allocated - a pointer to a region of memory previously allocated by my_malloc
//
void my_free(void *allocated) {
  // TODO
  header_t * nowAllocated = (header_t*)(((char*)(allocated)) - sizeof(header_t));
  assert(nowAllocated->magic == MAGIC);
  size_t size = nowAllocated->size;

  node_t * freeAllocated = (node_t*) nowAllocated;
  freeAllocated->size = size;
  freeAllocated->next = head;
  head = freeAllocated;
  coalesce(freeAllocated);
}
