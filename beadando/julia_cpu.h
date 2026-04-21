#ifndef JULIA_CPU_H
#define JULIA_CPU_H

#include "image.h"

//Iterációszám alapján RGB színt állít elő.
void color_map(int iter, int max_iter,
               unsigned char *r,
               unsigned char *g,
               unsigned char *b);

//Julia-halmaz generálása CPU-n, soros módon.
void generate_julia_cpu(Image *image,
                        int max_iter,
                        double c_real,
                        double c_imag);

#endif