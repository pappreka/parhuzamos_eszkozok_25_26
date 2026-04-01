#ifndef JULIA_CPU_H
#define JULIA_CPU_H

void generate_julia_cpu(unsigned char *image,
                        int width,
                        int height,
                        int max_iter,
                        double c_real,
                        double c_imag);

#endif