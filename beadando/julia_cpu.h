#ifndef JULIA_CPU_H
#define JULIA_CPU_H

#include "image.h"

void color_map(int iter, int max_iter,
               unsigned char *r,
               unsigned char *g,
               unsigned char *b);

void generate_julia_cpu(Image *image,
                        int max_iter,
                        double c_real,
                        double c_imag);

#endif