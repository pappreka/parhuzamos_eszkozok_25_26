#ifndef JULIA_OPENCL_H
#define JULIA_OPENCL_H

int generate_julia_opencl(unsigned char *image,
                          int width,
                          int height,
                          int max_iter,
                          double c_real,
                          double c_imag,
                          const char *kernel_file);

#endif