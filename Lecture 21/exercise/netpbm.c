#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

#include "netpbm.h"

int open_image(char * path, netpbm_ptr img)
{
  img->fd = fopen(path, "r+");
  if (img->fd == NULL) {
    return -1;
  }
  char magic[3];
  int max_val;
  if (fscanf(img->fd, "%2s %d %d %d", magic, &img->width, &img->height, &max_val) != 4) {
    return -1; // Se non ha letto 4 elementi, il formato del file non è valido
}

  if (strcmp(magic, "P5") != 0){
    return -1;
  }
  img->size = img->width * img->height;
  img->offset = ftell(img->fd);

  img->data = (char *) mmap(NULL, img->offset + img->size, PROT_READ | PROT_WRITE, MAP_SHARED, fileno(img->fd), 0);

  return 0;
}

int empty_image(char * path, netpbm_ptr img, int width, int height)
{
  FILE * fd = fopen(path, "w+");
  if (fd == NULL) {
    return -1;
  }
  int written = fprintf(fd, "P5\n%d %d\n255\n", width, height);
  if (ftruncate(fileno(fd), written + width * height) != 0) {
    return -1; // Errore nel ridimensionamento del file
  }
  fclose(fd);
  return open_image(path, img);
}

char * pixel_at(netpbm_ptr img, int x, int y)
{
  int pos = y*img->width + x;
  
  return img->data + img->offset + pos;
}

int close_image(netpbm_ptr img)
{
  munmap(img->data, img->offset + img->size);
  fclose(img->fd);
  return 0;
}
