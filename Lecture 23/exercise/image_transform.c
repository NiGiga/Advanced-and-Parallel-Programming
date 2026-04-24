#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <omp.h>

#include "netpbm.h"
#include "image_transform.h"

int blur_image(netpbm_ptr src, char *path, netpbm_ptr dst) {
    if (src == NULL || dst == NULL || path == NULL) {
        return -1;
    }
    create_output_image(path, src, dst, RADIUS_BLUR);

    // iterate on all input image pixels
    #pragma omp parallel for 
    for (int x = RADIUS_BLUR; x < src->width - RADIUS_BLUR; x++){
      for (int y = RADIUS_BLUR; y < src->height - RADIUS_BLUR; y++){
	// iterate on all neighbors
	int sum = 0;
	for (int i = x - RADIUS_BLUR; i <=  x + RADIUS_BLUR; i++){
	  for (int j = y - RADIUS_BLUR; j <= y + RADIUS_BLUR; j++){
	    sum += *(unsigned char*) pixel_at(src, i, j);
	  }
	}
	*(unsigned char*) pixel_at(dst, x - RADIUS_BLUR, y - RADIUS_BLUR) = sum / ((2 * RADIUS_BLUR + 1) * (2 * RADIUS_BLUR + 1));
      }
    }
    
				
    return 0;
}

unsigned long long *build_prefix_sum(const unsigned char *src_data, int w, int h) {

    unsigned long long *prefix = calloc((size_t)(h + 1) * (w + 1), sizeof(unsigned long long));

    if (prefix == NULL) {
        return NULL;
    }

    for (int y = 0; y < h; y++){
      for (int x = 0; x < w; x++){
	// current pixel of current image
	unsigned long long curr_pixel = src_data[y * w + x];

	// computing the monodimensional indexies for table prefix
	int curr_idx = (y + 1) * (w + 1) + (x + 1);
	int up_idx = y * (w + 1) + (x + 1);
	int lf_idx = (y + 1) * (w + 1) + x;
	int up_lf_idx = y * (w + 1) + x;

	prefix[curr_idx] = curr_pixel + prefix[up_idx] + prefix[lf_idx] - prefix[up_lf_idx];
      }
    }

    return prefix;
}


int edge_highlight(netpbm_ptr src, char *path, netpbm_ptr dst) {
    if (src == NULL || dst == NULL || path == NULL) {
        return -1;
    }
    create_output_image(path, src, dst, RADIUS_BORDER_HIGHLIGHT);
    unsigned long long *prefix = build_prefix_sum((unsigned char*)src->data, src->width, src->height);

    #pragma omp parallel for
    for (int y = RADIUS_BORDER_HIGHLIGHT; y < src->height - RADIUS_BORDER_HIGHLIGHT; y++){
      for (int x = RADIUS_BORDER_HIGHLIGHT; x < src->width - RADIUS_BORDER_HIGHLIGHT; x++){

        int R = RADIUS_BORDER_HIGHLIGHT;
	int w_p = src->width + 1;

	// down right
	int idx_D = (y + R + 1) * w_p + (x + R + 1);

	// down left
	int idx_C = (y + R + 1) * w_p + (x - R);

	// up right
	int idx_B = (y - R) * w_p + (x + R + 1);

	// up left
	int idx_A = (y - R) * w_p + (x - R);
	
	unsigned long long sum = prefix[idx_D] - prefix[idx_C] - prefix[idx_B] + prefix[idx_A];

	unsigned long long mean = sum / ((2 * R + 1) * (2 * R + 1));

	if (mean >= *(unsigned char*)pixel_at(src, x, y)){
	  // Sottraiamo il raggio per traslare le coordinate nella nuova immagine
	  *(unsigned char*)pixel_at(dst, x - RADIUS_BORDER_HIGHLIGHT, y - RADIUS_BORDER_HIGHLIGHT) = 255;
	} else {
	  *(unsigned char*)pixel_at(dst, x - RADIUS_BORDER_HIGHLIGHT, y - RADIUS_BORDER_HIGHLIGHT) = 0;
      }
    }
    }
    free(prefix);

    return 0;
}
