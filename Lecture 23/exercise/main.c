#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#include "netpbm.h"
#include "image_transform.h"

#ifndef FILENAMES
#define REFERENCE "kitty.pgm"
#define BLURRED "kitty-transformed-blurred.pgm"
#define EDGE "kitty-transformed-edgeselection.pgm"
#endif

#ifndef SEP
#define SEP "\t"
// #define SEP ","
#endif

int benchmark_images(netpbm *image, int num_threads, int n_iter) {
    netpbm blurred, edge;
    double start, end;
    double blur_time = 0.0;
    double edge_time = 0.0;
    int err;

    omp_set_num_threads(num_threads);

    for (int i = 0; i < n_iter; i++) {

        start = omp_get_wtime();
        err = blur_image(image, BLURRED, &blurred);
        end = omp_get_wtime();

        if (err != 0) {
            fprintf(stderr, "Unable to blur image: %d\n", -err);
            return err;
        }
        blur_time += end - start;

        start = omp_get_wtime();
        err = edge_highlight(image, EDGE, &edge);
        end = omp_get_wtime();

        if (err != 0) {
            fprintf(stderr, "Unable to compute edge_highlight: %d\n", -err);
            close_image(&blurred);
            return err;
        }
        edge_time += end - start;

        close_image(&blurred);
        close_image(&edge);
    }

    printf("%d%s%.6f%s%.6f\n",
           num_threads, SEP, blur_time / n_iter, SEP, edge_time / n_iter);

    return 0;
}

int test_functions(netpbm *image, int num_threads) {
    netpbm blurred, edge;
    int err;

    omp_set_num_threads(num_threads);
    err = blur_image(image, BLURRED, &blurred);
    if (err != 0) {
        fprintf(stderr, "Errore durante l'esecuzione di blur_image: %d\n", -err);
        return err;
    }

    err = edge_highlight(image, EDGE, &edge);
    if (err != 0) {
        fprintf(stderr, "Errore durante l'esecuzione di edge_highlight: %d\n", -err);
        close_image(&blurred);
        return err;
    }

    close_image(&blurred);
    close_image(&edge);
    return 0;
}

int main(void) {
    netpbm image;
    int err;
    err = open_image(REFERENCE, &image);
    if (err != 0) {
        fprintf(stderr, "Unable to open source image: %d\n", -err);
        return 1;
    }

    omp_set_dynamic(0);

    printf("threads%sblur_time%sedge_time\n", SEP, SEP);

    int nproc = omp_get_num_procs();

    // Test del codice:
    err = test_functions(&image, nproc);
    if (err != 0) {
        close_image(&image);
        return 1;
    }

    // Benchmark:
    // for (int i = 1; i <= nproc; i++) {
    //     err = benchmark_images(&image, i, 15);
    //     if (err != 0) {
    //         close_image(&image);
    //         return 1;
    //     }
    // }

    close_image(&image);
    return 0;
}
