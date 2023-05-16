#include <assert.h>
#include <buddy_malloc.h>
#include <stdio.h>
#include <sys/mman.h>
#include <iostream>
using namespace std;

bool emptyFound = false;
binary_t * treeHead = NULL; // A pointer to the head of the binary tree.


// Allocates memory to the binary_t type node of the binary tree
//
// PARAMETERS:
// node - the binary tree node that needs to be created
//
//
void buddy_createBinaryTreeNode(binary_t ** node) {
   *node =(binary_t*) mmap(NULL, sizeof(binary_t), PROT_READ | PROT_WRITE,MAP_ANON | MAP_PRIVATE, -1, 0);
}


// The buddy_heap function returns the head pointer to the binary tree. If the heap
// has not been allocated yet (treeHead is NULL) it will use mmap to allocate
// a page of memory from the OS which will act as the heap for our code
binary_t *buddy_heap() {
  if (treeHead == NULL) {
    // This allocates the heap and initializes the head node.
    buddy_createBinaryTreeNode (&treeHead); //this will allocate memory to treeHead
    void * offset = mmap(NULL, HEAP_SIZE, PROT_READ | PROT_WRITE,
                          MAP_ANON | MAP_PRIVATE, -1, 0);  
    treeHead->size = HEAP_SIZE - sizeof(header__t);  
    treeHead->rightChild = NULL;     //no children in the begining
    treeHead->leftChild = NULL;
    treeHead->parent = NULL;      //the head has no parent
    treeHead->empty = true;       
    treeHead->offset = offset;      //the head's offset points to the start of the heap
  }

  return treeHead;
}

// Reallocates the heap.
void buddy_reset_heap() {
  if (treeHead != NULL) {
    munmap(treeHead, HEAP_SIZE);
    treeHead = NULL;
    buddy_heap();
  }
}


// Helper function for buddy_number_of_free_nodes and buddy_available_memory
//
// Recursively updates the global variables that store available memory and
// number of free nodes in the binary tree
// 
// PARAMETERS: 
//
// node: head of the binary tree
//
size_t available_memory = -1;
int num_of_free_nodes = -1;
void freeNodeAndAvailableMemoryHelper (binary_t * node) {
  bool childrenDontExist = (node->leftChild == NULL) && (node->rightChild ==NULL);
    if (childrenDontExist && (node->empty ==true)) {  //base case: checks if the node is an empty leaf nodex 
      available_memory+=node->size;
      num_of_free_nodes++;
    }
    else {
      if (!childrenDontExist) { //children exist
        //recursively call the function for it's children nodes
        freeNodeAndAvailableMemoryHelper(node->leftChild);    
        freeNodeAndAvailableMemoryHelper(node->rightChild);
      }
  } 
}

// Calculates the amount of free memory available in the heap.
size_t buddy_available_memory() {
  binary_t *hd = buddy_heap();
  available_memory = 0;
  freeNodeAndAvailableMemoryHelper(hd);

  return available_memory;
}

// Returns the number of nodes with available memory in the binary tree.
int buddy_number_of_free_nodes() {
  binary_t *hd = buddy_heap();
  num_of_free_nodes = 0;
  freeNodeAndAvailableMemoryHelper(hd);

  return num_of_free_nodes;
}

//This function is used for styling purposes
void heapStyling() {
  cout << "\033[1;34m"; // Set text color to light blue
  cout<<"----";
  cout << "\033[0m"; // Reset text color to default
  cout<<""<<endl;
}

// This is a recursive helper function used by buddy_print_free_list to print
// the leaf nodes of the binary tree that are empty
void helper(binary_t * node) {
    bool childrenDontExist = (node->leftChild == NULL) && (node->rightChild ==NULL);
    if (childrenDontExist && (node->empty ==true)) {    //base case: checks if the node is an empty leaf nodex 
      printf("%zd\n", node->size);
      heapStyling();
    }
    else {
      if (!childrenDontExist) { //children exist
        helper(node->leftChild);
        helper(node->rightChild);
      }
    } 
  }

// Prints the free list. Useful for debugging purposes.
void buddy_print_free_list() {
  binary_t *hd = buddy_heap();
  heapStyling();
  helper(hd);
  cout<<""<<endl;
}

// Splits a binary tree node to two other binary tree nodes.
//accommodate an allocation request.
//
// The job of this function is to take a given binary tree node found from
// `find_free` and split it according to the number of bytes to allocate. 
// The reason behind splitting this node is because it's size is >= atleast  
// of the size of memory the user/program is requesting for. In doing so, it create two new
// binary_t nodes and make it's leftChild and rightChild pointers to point to them.
// The size field of these two new nodes will be half - sizeof(header_t) of the 
// actual size of the parent node (which is: parent's size + sizeof(header_t))
//
// PARAMETERS:
// node - the current node of the binary tree
// leftChild - the left Child of the current node in the binary tree
// rightChild - the right Child of the current node in the binary tree
//
// Updates:
// leftChild - the left Child is allocated memory and is pointed to by 
// the current node of the binary tree
// rightChild - the right Child is allocated memory and is pointed to by 
// the current node of the binary tree
//
void buddy_split(binary_t **node, binary_t **leftChild,
           binary_t **rightChild) {
  
  buddy_createBinaryTreeNode(leftChild);    //memory allocated for leftChild from mmap
  buddy_createBinaryTreeNode(rightChild);   //memory allocated for rightChild from mmap
  (*node)->rightChild = *rightChild;
  (*node)->leftChild = *leftChild;
  (*node)->empty = false;                   //memory region from where this node's offset field points is not empty anymore. Since it is being splitted, either its children or their children will be used

  size_t actualHalfOfCurr = ((((*node)->size) + sizeof(header__t))/2);    //eg 1024  => (1008 + 16 )/2 = 512
  size_t halfAfterHeader = actualHalfOfCurr - sizeof(header__t);          // 512 - 16 = 496
  void * rightOffset = ((void *)((char *)((*node)->offset) + actualHalfOfCurr));


  // splitting the current memory block into two equal halves of memory region. First block which is the leftChild
  //will have the starting offset as the node
  //the rightChild will have offset starting from half of the memory region's address
  (*rightChild)->parent = *node;    //the children have a pointer to the parent
  (*rightChild)->empty = true;
  (*rightChild)->rightChild = NULL;
  (*rightChild)->leftChild = NULL;
  (*rightChild)->size = halfAfterHeader;
  (*rightChild)->offset = rightOffset;      

  (*leftChild)->parent = *node;      //the children have a pointer to the parent
  (*leftChild)->empty = true;
  (*leftChild)->rightChild = NULL;
  (*leftChild)->leftChild = NULL;
  (*leftChild)->size = halfAfterHeader;
  (*leftChild)->offset = (*node)->offset;     //same offset as parent
} 

// Finds a node on the binary tree that has enough available memory to
// allocate to a calling program. This function uses the recursive "binary buddy allocation"
// algorithm to locate a free node.
//
// PARAMETERS:
// size - the number of bytes requested to allocate
// found - the offset/address in the heap which could be returned to the calling program as a response to their request 
// node - Initially the head of the binary tree
//
// Updates:
// found - offset set to the offset found in the first empty node in the binary tree of appropriate size
// node - with each recursive call the variable node will be updated with the value of either its left or right child
//
void buddy_find_free(size_t size, void **found, binary_t **node) {
    // TODO

    //base case
    if (emptyFound == true || (size > (*node)->size) || (((*node)->empty == false) && ((*node)->leftChild == NULL) && ((*node)->rightChild == NULL))) {
      return;
    }

    binary_t *leftChild = NULL;
    binary_t *rightChild = NULL;

    if (((*node)->rightChild == NULL) && ((*node)->leftChild == NULL)) {  //if leaf node of a binary tree reached
        
        size_t actualHalfOfCurr = ((((*node)->size) + sizeof(header__t))/2);
        size_t halfAfterHeader = actualHalfOfCurr - sizeof(header__t);
        if (size > (halfAfterHeader)) {   // if the node cannot be split any further, when the size requested is greater than half of current node's size
          *found = (*node)->offset;     
          emptyFound = true;  
          (*node)->usedSpace = size;
          (*node)->empty = false;   //only in this case wehre we will allocate memory to this offset, we will make the node's empty to be false
          return;
        }
        else {
          buddy_split(node, &leftChild, &rightChild);     //split the node when the size requested is less than =  half of the size of current node's size
        }
    }
    else {
        //either of them have to be used up 
        leftChild = (*node)->leftChild;
        rightChild = (*node)->rightChild;
    }

    buddy_find_free(size, found, &rightChild);
    buddy_find_free(size, found, &leftChild);
    return;

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
void *buddy_malloc(size_t size) {
  // TODO
  void * found = NULL;      // will store the offset updated by the function buddy_find_free
  binary_t * treeHeadCpy = buddy_heap();
  emptyFound = false;
  buddy_find_free(size, &found, &treeHeadCpy);    //updates found to point to an offset in heap which will be the begining of header_t
  if (found == NULL) {
    return NULL;
  }

  //adding header to the first 16 bytes of the blocks
  header__t * allocated = ((header__t *) found);
  allocated->size = size;
  allocated->magic = MAGIC;

  return (void *)(((char *)(allocated)) + sizeof(header__t));
}

// Merges the empty buddies in the binary tree to reduce external fragmentation.
//
// This recursive function will coallesce buddy nodes bottom up from the current node
// to ancestors until either child of the parent is not empty or the parent is NULL
//
// PARAMETERS:
// foundNode - the starting leaf node on the binary tree to coalesce
//
void buddy_coalesce(binary_t *foundNode) {
  // TODO
  binary_t * parent = foundNode->parent;
  if (parent == NULL) {           //if the current node is the head of binary tree
    foundNode->empty = true;
    return;
  }
  else {
    foundNode->empty = true;
    if ((parent->leftChild->empty == true) && (parent->rightChild->empty == true)) { // if both the children of a parent are empty then coalesce them
      cout<<"The two nodes with size "<<parent->leftChild->size<<" were coalesced to size "<<parent->size<<endl;
      munmap(parent->leftChild, sizeof(binary_t));    //free memory for this node
      munmap(parent->rightChild, sizeof(binary_t));   //free memory for this node
      parent->leftChild = NULL;
      parent->rightChild = NULL;
      buddy_coalesce(parent);       //recursively keep on calling until it is not possible to coalesce
    }
  }
 
}

// Finds the binary tree node whose offset field points to the same address as the parameter offset. The is
// a recursive function which uses Depth first Search Approach to find the node.
//
// PARAMETERS:
// size - size of the memory being freed by the user/program
// node - the head of the binary tree
// offset - the address of the start of the memory block that needs to be freed
// foundNode - Initally NULL, Is updated when the matching offset binary tree node is found. The node found
// will always be a leaf node in the binary tree
//
bool nodeFound = false;
void buddy_findTreeNode (size_t size, binary_t * node, void * offset, binary_t ** foundNode) {
  if (nodeFound ==true || node == NULL) {
    return;
  }
  if ((node->offset == offset) && (node->empty == false) && (node->usedSpace == size)) {
    *foundNode = node;
    nodeFound = true;   //the node in the tree whose offset points to this address is found, no need to iterate any more
  }

  buddy_findTreeNode (size, node->rightChild, offset, foundNode);
  buddy_findTreeNode (size, node->leftChild, offset, foundNode);
}

// Frees a given region of memory back to the heap. Updates the respective binary tree node accordingly.
//
// PARAMETERS:
// allocated - a pointer to a region of memory previously allocated by buddy_malloc
//
void buddy_my_free(void *allocated) {
  header__t * startOfBlock = (header__t*)(((char*)(allocated)) - sizeof(header__t));
  size_t size = startOfBlock->size;   //gives the size of memory originally requested by user/program
  assert(startOfBlock->magic == MAGIC);
  void * offset = (void*)startOfBlock; 

  //Now we will have to find which node in the binary tree has empty set as false, and has the same offset as this offset. We will basically do a DFS
  binary_t * treeHeadCpy = buddy_heap();
  binary_t * foundNode = NULL;
  nodeFound = false;
  buddy_findTreeNode(size, treeHeadCpy,offset, &foundNode);
  foundNode->usedSpace = 0; //setting the used space back to 0

  buddy_coalesce(foundNode);
}