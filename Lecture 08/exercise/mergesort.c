#include <stdlib.h>
#include <stdio.h>
#include "merge.h"

void merge_sort(int * v, int len, void (*m) (int *, int, int *, int, int *)){

  int* tmp = (int*)malloc(sizeof(int)*len);
  
  for(int curr_size = 1; curr_size < len; curr_size = curr_size*2){

    for(int left_start = 0; left_start < len; left_start = left_start + 2 * curr_size){

      int mid = left_start + curr_size;
      if (mid > len) mid = len;

      int right_end = left_start + 2*curr_size;
      if (right_end > len) right_end = len;

      int n1 = mid - left_start;
      int n2 = right_end - mid;

      m(v + left_start, n1, v + mid, n2, tmp + left_start);
    }

    for( int i = 0; i < len; i++){
      v[i] = tmp[i];
    }
  }
  free(tmp);
}
