#ifndef IMAGE_TRANSFORM_H
#define IMAGE_TRANSFORM_H

#define RADIUS_BLUR 50
#define RADIUS_BORDER_HIGHLIGHT 250

int blur_image(netpbm_ptr src, char *path, netpbm_ptr dst);

unsigned long long *build_prefix_sum(const unsigned char *src_data, int w, int h);

int edge_highlight(netpbm_ptr src, char *path, netpbm_ptr dst);

int close_image(netpbm_ptr img);

#endif