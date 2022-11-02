#include <stdlib.h>
#include <stdio.h>
#include "history.h"
#include "tokenizer.h"
#include "implementation.c"

List* init_history(){
  List* list = (List*)malloc(sizeof(List));

  return list;

}
void add_history(List *list, char *str){
Item *node;
 node = (Item*)malloc(sizeof(Item));
 
char *copy = copy_str(str,256);
node -> str = copy;
int id = 0;
if(list -> root == NULL){
node -> id = id;
list -> root = node;
}
 else{
   Item *location = list -> root;
   int id = 1;
   while(location != NULL){

     if(location -> next == NULL){
       node -> id = id;
       location -> next = node;
       break;
     }
     location = location -> next;
     id++;
   }
 }
}


char *get_history(List *list, int id){
  Item *location = list -> root;
  while(location != NULL){
    if(location -> id == id) return location -> str;
    location = location -> next;
  }
  return NULL;
}

void print_history(List *list){
  Item *location = list -> root;
  printf("-----List History-----\n");
  while(location != NULL){
    printf("[%d]--%s\n",location -> id,location -> str);
      location = location -> next;
  }
}

void free_history(List *list){
  Item *location = list -> root;
  Item *location2 = location -> next;
  while(location2 != NULL){
    __free_impl(location -> str);
    __free_impl(location);
    location = location2;
    location2 = location2 -> next;
  }
  __free_impl(location -> str);
  __free_impl(location);
  __free_impl(list);
}
