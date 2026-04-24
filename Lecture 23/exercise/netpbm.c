#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <omp.h>

#include "netpbm.h"


int open_image(char * path, netpbm_ptr img) {
  img->fd = fopen(path, "r+");
  if (img->fd == NULL) {
    return -1;
  }
  struct stat sbuf;
  stat(path, &sbuf);
  img->size = sbuf.st_size;
  if (fscanf(img->fd, "P5\n%d %d\n255\n", &img->width, &img->height) != 2) {
    fclose(img->fd);
    return -2;
  }
  img->offset = ftell(img->fd);
  img->data = mmap((void *)0, img->size, PROT_READ | PROT_WRITE, MAP_SHARED, fileno(img->fd), 0);
  if (img->data == MAP_FAILED) {
    fclose(img->fd);
    return -3;
  }
  return 0;
}

char *pixel_at(netpbm_ptr img, int x, int y) {
    if (img == NULL) {
        return NULL;
    }
   if (x < 0 || x >= img->width) {
        return NULL;
    }
    if (y < 0 || y >= img->height) {
        return NULL;
    }
    return &img->data[y * img->width + x + img->offset];
}

int close_image(netpbm_ptr img) {
    if (img == NULL) {
        return -1;
    }
    munmap(img->data, img->size);
    fclose(img->fd);
    return 0;
}

int create_output_image(char *path, netpbm_ptr src, netpbm_ptr dst, int pixel_mask_size) {
    if (path == NULL || src == NULL || dst == NULL) {
        return -1;
    }

    // dimensioni nuova immagine & check
    int dst_width  = src->width  - 2 * pixel_mask_size;
    int dst_height = src->height - 2 * pixel_mask_size;
    if (dst_width <= 0 || dst_height <= 0) {
        return -2;
    }

    FILE *fd = fopen(path, "w+");
    if (fd == NULL){
        return -3;
    }
    int written = fprintf(fd, "P5\n%d %d\n255\n", dst_width, dst_height);
    if (written < 0) {
        fclose(fd);
        return -4;
    }
    if (ftruncate(fileno(fd), written + dst_width * dst_height) != 0) {
        fclose(fd);
        return -5;
    }
    fclose(fd);

    return open_image(path, dst);
}