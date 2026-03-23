#include "merge.h"

void merge(int * v1, int n1, int * v2, int n2, int * results)
{
  int i = 0;
  int j = 0;
  int k = 0;

  while(i < n1 && j < n2){
    if(v1[i] <= v2[j]){
      results[k] = v1[i];
      i ++;
      k ++;
    }
    else{
      results[k] = v2[j];
      j ++;
      k ++;
    }
  }
  if(i < n1){
    for(int m = i; m < n1; m ++){
      results[k] = v1[m];
      k ++;
    }
  }
  else{
    for(int m = j; m < n2; m++){
      results[k] = v2[m];
      k ++;
    }
  }
}

void merge_branchless(int * v1, int n1, int * v2, int n2, int * results)
{
  int i = 0;
  int j = 0;
  int k = 0;

  while(i < n1 && j < n2){
    int q = (v1[i] <= v2[j]);
    results[k] = q * v1[i] + (1-q) * v2[j];
    k++;
    i = i + q;
    j = j + (1-q);
  }
  while(i < n1) { results[k++] = v1[i++]; }
  while(j < n2) { results[k++] = v2[j++]; }
}
