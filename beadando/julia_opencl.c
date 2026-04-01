#include "julia_opencl.h"

#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK_CL(err, msg) \
    do { \
        if ((err) != CL_SUCCESS) { \
            fprintf(stderr, "%s hiba: %d\n", (msg), (err)); \
            goto cleanup; \
        } \
    } while (0)

static char *load_kernel_source(const char *filename, size_t *length) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Nem sikerult megnyitni a kernel fajlt: %s\n", filename);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    if (size <= 0) {
        fclose(fp);
        fprintf(stderr, "Ures vagy ervenytelen kernel fajl.\n");
        return NULL;
    }

    char *source = (char *)malloc((size_t)size + 1);
    if (!source) {
        fclose(fp);
        fprintf(stderr, "Nem sikerult memoriat foglalni a kernel forrashoz.\n");
        return NULL;
    }

    size_t read_size = fread(source, 1, (size_t)size, fp);
    fclose(fp);

    if (read_size != (size_t)size) {
        free(source);
        fprintf(stderr, "Nem sikerult teljesen beolvasni a kernel fajlt.\n");
        return NULL;
    }

    source[size] = '\0';
    *length = (size_t)size;
    return source;
}

static void print_build_log(cl_program program, cl_device_id device) {
    size_t log_size = 0;
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

    if (log_size > 1) {
        char *log = (char *)malloc(log_size);
        if (log) {
            clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
            fprintf(stderr, "OpenCL build log:\n%s\n", log);
            free(log);
        }
    }
}

int generate_julia_opencl(unsigned char *image,
                          int width,
                          int height,
                          int max_iter,
                          double c_real,
                          double c_imag,
                          const char *kernel_file) {
    cl_int err = CL_SUCCESS;
    int result = 0;

    cl_platform_id platform = NULL;
    cl_device_id device = NULL;
    cl_context context = NULL;
    cl_command_queue queue = NULL;
    cl_program program = NULL;
    cl_kernel kernel = NULL;
    cl_mem image_buffer = NULL;

    char *source = NULL;
    size_t source_len = 0;

    size_t image_size = (size_t)width * (size_t)height * 3;

    source = load_kernel_source(kernel_file, &source_len);
    if (!source) {
        return 0;
    }

    err = clGetPlatformIDs(1, &platform, NULL);
    CHECK_CL(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "GPU nem elerheto, probalkozas CPU eszkozzel.\n");
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
        CHECK_CL(err, "clGetDeviceIDs");
    }

    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    CHECK_CL(err, "clCreateContext");

#if defined(CL_VERSION_2_0)
    queue = clCreateCommandQueueWithProperties(context, device, 0, &err);
#else
    queue = clCreateCommandQueue(context, device, 0, &err);
#endif
    CHECK_CL(err, "clCreateCommandQueue");

    program = clCreateProgramWithSource(context, 1, (const char **)&source, &source_len, &err);
    CHECK_CL(err, "clCreateProgramWithSource");

    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "clBuildProgram hiba: %d\n", err);
        print_build_log(program, device);
        goto cleanup;
    }

    kernel = clCreateKernel(program, "julia_kernel", &err);
    CHECK_CL(err, "clCreateKernel");

    image_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, image_size, NULL, &err);
    CHECK_CL(err, "clCreateBuffer");

    err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &image_buffer);
    err |= clSetKernelArg(kernel, 1, sizeof(int), &width);
    err |= clSetKernelArg(kernel, 2, sizeof(int), &height);
    err |= clSetKernelArg(kernel, 3, sizeof(int), &max_iter);
    err |= clSetKernelArg(kernel, 4, sizeof(float), &(float){(float)c_real});
    err |= clSetKernelArg(kernel, 5, sizeof(float), &(float){(float)c_imag});
    CHECK_CL(err, "clSetKernelArg");

    size_t global_size[2] = { (size_t)width, (size_t)height };

    err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_size, NULL, 0, NULL, NULL);
    CHECK_CL(err, "clEnqueueNDRangeKernel");

    err = clFinish(queue);
    CHECK_CL(err, "clFinish");

    err = clEnqueueReadBuffer(queue, image_buffer, CL_TRUE, 0, image_size, image, 0, NULL, NULL);
    CHECK_CL(err, "clEnqueueReadBuffer");

    result = 1;

cleanup:
    if (image_buffer) clReleaseMemObject(image_buffer);
    if (kernel) clReleaseKernel(kernel);
    if (program) clReleaseProgram(program);
    if (queue) clReleaseCommandQueue(queue);
    if (context) clReleaseContext(context);
    free(source);

    return result;
}