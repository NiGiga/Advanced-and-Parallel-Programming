#ifndef NETPBM_H
#define NETPBM_H

#include <stdio.h>
typedef struct _netpbm_image {
    int width;
    int height;
    int offset;
    int size;
    FILE *fd;
    char *data;
} netpbm;

typedef netpbm *netpbm_ptr;

int open_image(char *path, netpbm_ptr img);

char *pixel_at(netpbm_ptr img, int x, int y);

int create_output_image(char *path, netpbm_ptr src, netpbm_ptr dst, int pixel_mask_size);

#endif