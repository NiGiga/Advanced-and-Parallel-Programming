#ifndef _MERGE_H
#define _MERGE_H

void merge(int * v1, int n1, int * v2, int n2, int * results);

void merge_branchless(int * v1, int n1, int * v2, int n2, int * results);

void merge_sort(int * v, int len, void (*m) (int *, int, int *, int, int *));

#endif
