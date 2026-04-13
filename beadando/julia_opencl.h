#ifndef JULIA_OPENCL_H
#define JULIA_OPENCL_H

#include <stddef.h>
#include <CL/cl.h>

#include "image.h"

char *load_kernel_source(const char *filename, size_t *length);
void print_build_log(cl_program program, cl_device_id device);
void print_device_info(cl_device_id device);

int generate_julia_opencl(Image *image,
                          int max_iter,
                          double c_real,
                          double c_imag,
                          const char *kernel_file,
                          double *transfer_time_ms);

#endif