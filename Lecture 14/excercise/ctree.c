#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "ctree.h"

ctree make_cnode(int key, float val)
{
  /*
    Description: Function that makes a node in a k-nary search tree and initialize his childs to NULL
    Input:
        key: integer rappresenting the key of the element
	val: float rappresenting the value of the element
    Returns:
        t: the node that we create
   */

     
  ctree t = (ctree) malloc(sizeof(struct _ctree_node));
  // First element in first position
  t->key[0] = key;
  t->val[0] = val;
  // Insert exactly 1 element
  t->first_free = 1;

  // New node is leaf, all childrens are NULL
  for (int i = 0; i <= N; i++){
    t ->children[i] = NULL;
  }
  return t;
}

ctree cinsert(ctree t, int key, float val)
{
  /*
    Description: Function that insert  a node in a k-nary search tree
    Input:
        t: ctree node array of key, val
        key: integer rappresenting the key of the element
	val: float rappresenting the value of the element
    Returns:
        t: the node that we create
   */
  
  // Case 1: ctree is empty
  if (t == NULL){
    return make_cnode(key, val);
  }
  
  // Case 2: if the key exists, replace the corrisponding value
  // iterate on the current node, if we find key we replace the value and return
  for (int i = 0; i < t->first_free; i++){
    if (t->key[i] == key){
      t->val[i] = val;
      return t;
    }
  }

  // Case 3: The key is not in the curent node, we need to add a new key
  // If first_free<N every new key is insertable in the node with no need of new nodes
  if (t->first_free < N){
    // iterate on the node to find where to insert elem. without breaking asc. order
    int i;
    for (i = t->first_free - 1; i >= 0; i--){
      if (key < t->key[i]){
	t->key[i+1] = t->key[i];
	t->val[i+1] = t->val[i];
      }else{ // we've found the right place, exit loop
	break;
      }
    }

    // update the key and the value
    t->key[i+1] = key;
    t->val[i+1] = val;
    // we had insert a new node
    t->first_free++;
    
    return t;
  }

  // Case 4: the node is full, we need to insert the node in a sub-tree
  // we can have N+1 children, we need to find the fisrt key > of the key to insert
  int i;
  // every node has max N+1 children, we need to find the correct children by index
  for (i = 0; i < N; i++){
    if (key < t->key[i]){
      break;
    }
  }
  // insret key and value in the child
  t->children[i] = cinsert(t->children[i], key, val);

  return t;
}


bool csearch(ctree t, int key, float * val)
{
    /*
    Description: Function that search a key and eventually replace the pointer to val with the val of the node
    if the keys are equal
    Input:
        t: ctree node array of key, val
        key: integer rappresenting the key of the element
	val: pointer to float rappresenting the value of the element
    Returns:
        boolean: True if we find a match and we replace val, false otherwise
   */
  
  // Case 0: If tree is empty return false
  if (t == NULL){
    return false;
  }

  // Case 1: Iterate on the current node
  for (int i = 0; i < t->first_free; i++){
    // If the key is the current node key, modify *val and return true
    if (t->key[i] == key){
      *val = t->val[i];
      return true;
    }
  }

  // Case 2: If we are here the key is not in the current node,
  // we need to search in the children
  int i;
  for (i = 0; i < t->first_free; i++){
    if (key < t->key[i]){
      break;
    }
  }
  // insret key and value in the child
  return csearch(t->children[i], key, val);
  
}

void print_ctree(ctree t)
{
    /*
    Description: Function that prints a k-nary search tree
    Input:
        t: ctree node array of key, val
    
   */
  
  if (t == NULL){
    printf(".");
    return;
  }
  printf("(");

  int i;
  for (i = 0; i < t->first_free; i++){
    print_ctree(t->children[i]);
    printf(" %d ", t->key[i]);
  }
  print_ctree(t->children[i]);
  printf(")");
  return;
}
