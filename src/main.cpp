#include <my_malloc.h>
#include <buddy_malloc.h>
#include <stdio.h>
#include <iostream>
using namespace std;

int main() {
  void *allocated[10]; 



//LECTURE EXAMPLE for expandable heap with both way coallescing
  
cout << "\033[1;32m";
cout << "-------------------------------------------------------------------------------" << endl;
cout << "--------------------------------Updated Coalescing-----------------------------" << endl;
cout << "-------------------------------------------------------------------------------" << endl;
cout << "\033[0m";
  cout <<"Available memory before: 4080" <<endl;
  for (int i = 0; i < 3; i++) {
    cout<<i+1<<". "<<"100 bytes of memory requested"<<endl;
    allocated[i] = my_malloc(100);
    print_free_list();
    cout<<""<<endl;
  }

  printf("Available memory after: %zd.\n", available_memory());
  cout<<""<<endl; 
  printf("Available memory before Freeing the malloced memory: %zd.\n", available_memory());
  cout<<""<<endl; 
  
  my_free(allocated[0]);
  cout<<"First 100 bytes block freed"<<endl;
  print_free_list();
  cout<<""<<endl;
  my_free(allocated[1]);
  cout<<"Second 100 bytes block freed"<<endl;
  print_free_list();
  cout<<""<<endl;
  my_free(allocated[2]);
  cout<<"Third 100 bytes block freed"<<endl;
  print_free_list();
  cout<<""<<endl;

  printf("Available memory after Freeing the malloced memory: %zd.\n", available_memory());

  cout << "\033[1;31m";
  cout<<"-------------------------------------------------------------------------------"<<endl;
  cout<<"---------------------------------Expandable Heap-------------------------------"<<endl;
  cout<<"-------------------------------------------------------------------------------"<<endl;
  cout << "\033[0m"; 

  cout <<"Available memory before: 4080" <<endl;
  for (int i = 0; i < 10; i++) {
    cout<<i+1<<". "<<"1000 bytes of memory requested"<<endl;
    allocated[i] = my_malloc(1000);
    print_free_list();
    cout<<""<<endl;
  }
  print_free_list();
  printf("Available memory after: %zd.\n", available_memory());
  cout<<""<<endl;

  printf("Available memory before freeing the malloced memory: %zd.\n", available_memory());
  for (int i = 0; i < 10; i++) {
    my_free(allocated[i]);
  }
  cout<<""<<endl;
   cout<<"Freeing memory.."<<endl;
  print_free_list();
  cout<<""<<endl;
  printf("Available memory after: %zd.\n", available_memory());

  //BINARY BUDDY ALLOCATOR
  cout << "\033[1;34m"; // Set text color to light blue
  cout<<"-------------------------------------------------------------------------------"<<endl;
  cout<<"------------------------------Binary Buddy Allocator---------------------------"<<endl;
  cout<<"-------------------------------------------------------------------------------"<<endl;
  cout << "\033[0m"; // Reset text color to default
  cout<< "" <<endl;
  cout<<"---------------------------------Allocating Memory--------------------------------"<<endl;
  cout<<""<<endl;
  cout <<"490 bytes of memory allocated iteratively 6 times. Look at the heap at each iteration."<<endl;
  void *allocated2[10];
  cout<<" "<<endl;
  buddy_print_free_list();
  cout <<"Available memory before: "<< buddy_available_memory() <<endl;
  cout<<" "<<endl;
  for (int i = 0; i < 6; i++) {
    cout<<i+1<<"."<<endl;
    allocated2[i] = buddy_malloc(490);
    buddy_print_free_list();
    cout<<"Available Memory: "<<buddy_available_memory()<<endl;
    cout<<"Number of free Nodes in the binary tree: "<<buddy_number_of_free_nodes()<<endl;
    cout<<" "<<endl;
  }
  
  // printf("Available memory after: %zd.\n", available_memory());
  cout<<"---------------------------------Freeing Memory--------------------------------"<<endl;
  // printf("Available memory before: %zd.\n", available_memory());
  

  for (int i = 0; i < 6; i++) {
    cout<<i+1<<"."<<endl;
    cout<<"Before freeing: "<<endl;
    buddy_print_free_list();
    cout<<"After freeing: "<<endl;
    buddy_my_free(allocated2[i]);
    buddy_print_free_list();
    cout<<"Available Memory: "<<buddy_available_memory()<<endl;
    cout<<"Number of free Nodes in the binary tree: "<<buddy_number_of_free_nodes()<<endl;
    cout<<" "<<endl;
  }
  
  
  // my_free(allocated[1]);
  // my_free(allocated[2]);

  // print_free_list();
  // printf("Available memory after: %zd.\n", available_memory());

  return 0;
}
