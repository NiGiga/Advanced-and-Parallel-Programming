#include <stdlib.h>
#include <stdio.h>

#include "unrolled_linked_list.h"

unrolled_linked_list ulst_make(void)
{
  unrolled_linked_list lst = (unrolled_linked_list) malloc(sizeof(struct _unrolled_linked_list));
  lst->head = NULL;
  return lst;
}

void ulst_delete(unrolled_linked_list lst)
{
  if (lst == NULL){
    return;
  }
  unode current = lst->head;
  while (current != NULL){
    unode prev = current;
    current = current->next;
    free(prev);
  }
  free(lst);
}

void ulst_add(unrolled_linked_list lst, int key)
{
  // Case 1: Head node exists? We will try to make some space.
  if (lst->head != NULL) {
    for (int i = 0; i < UNROLLED_SIZE; i++) {
      if (lst->head->valid[i] == false) { // Free space found at index'i'!
        lst->head->keys[i]  = key;
	lst->head->valid[i] = true;
	return;
      }
    }
  }

  // Case 2: If we arrive hear, the list is empty or the nodes are full.
  unode new_node = (unode) malloc(sizeof(struct _unrolled_node));
  for (int i = 0; i < UNROLLED_SIZE; i++) {
    new_node->valid[i] = false;
  }
  new_node->keys[0] = key;
  new_node->valid[0] = true;
  
  new_node->next = lst->head;
  lst->head = new_node;
}

bool ulst_search(unrolled_linked_list lst, int key)
{
  if (lst == NULL){
    return false;
  }
  unode current = lst->head;
  while (current != NULL){
    // 1. Linear scanning inside of the current node
    for (int i = 0; i < UNROLLED_SIZE; i++){
      if (current->valid[i] == true && current->keys[i] == key){
	return true; // Found! We can exit the function!
      }
    }

    // 2. If we didn't found it, we go for the next node
    current = current->next;
  }
  // If we are hear, we didn't found the key
  return false;
}

void ulst_print(unrolled_linked_list lst)
{
  if (lst == NULL) {
    printf("NIL");
    return;
  }
  printf("(");
  unode current = lst->head;
  while (current != NULL) {
    printf("[");
    for (int i = 0; i < UNROLLED_SIZE; i++) {
      if (current->valid[i]) {
	printf("%d", current->keys[i]);
      } else {
	printf(".");
      }
      if (i < UNROLLED_SIZE - 1) {
	printf(" ");
      }
    }
    printf("]");
    if (current->next != NULL) {
      printf(" ");
    }
    current = current->next;
  }
  printf(")");
}
