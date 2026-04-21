#include "julia_opencl.h"

#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "timer.h"

//OpenCL hibakezelő makró.
#define CHECK_CL(err, msg) \
    do { \
        if ((err) != CL_SUCCESS) { \
            fprintf(stderr, "%s error: %d\n", (msg), (err)); \
            goto cleanup; \
        } \
    } while (0)

//Az OpenCL kernel forrásfájljának beolvasása memóriába.
//filename - a kernel forrásfájl neve
//length   - ide kerül a beolvasott forrás mérete
char *load_kernel_source(const char *filename, size_t *length) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Error: failed to open kernel file: %s\n", filename);
        return NULL;
    }

    //Fájlméret meghatározása
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    if (size <= 0) {
        fclose(fp);
        fprintf(stderr, "Error: kernel file is empty or invalid.\n");
        return NULL;
    }

    //Memóriafoglalás a forráskódnak (+1 a lezáró nullának)
    char *source = (char *)malloc((size_t)size + 1);
    if (!source) {
        fclose(fp);
        fprintf(stderr, "Error: failed to allocate memory for kernel source.\n");
        return NULL;
    }

    //Fájl tartalmának beolvasása
    size_t read_size = fread(source, 1, (size_t)size, fp);
    fclose(fp);

    if (read_size != (size_t)size) {
        free(source);
        fprintf(stderr, "Error: failed to read kernel file completely.\n");
        return NULL;
    }

    //C string lezárása
    source[size] = '\0';
    *length = (size_t)size;
    return source;
}

//OpenCL fordítási napló kiírása.
void print_build_log(cl_program program, cl_device_id device) {
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

//Az OpenCL eszköz főbb hardveres tulajdonságainak kiírása.
//Ez segít annak ellenőrzésében, hogy milyen GPU/CPU eszközt használ a program.
void print_device_info(cl_device_id device) {
    char device_name[256] = {0};
    char vendor_name[256] = {0};
    char device_version[256] = {0};
    char driver_version[256] = {0};

    cl_uint compute_units = 0;
    cl_uint clock_frequency = 0;
    cl_ulong global_mem_size = 0;
    cl_ulong local_mem_size = 0;
    size_t max_work_group_size = 0;
    size_t max_work_item_sizes[3] = {0, 0, 0};

    clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_name), device_name, NULL);
    clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(vendor_name), vendor_name, NULL);
    clGetDeviceInfo(device, CL_DEVICE_VERSION, sizeof(device_version), device_version, NULL);
    clGetDeviceInfo(device, CL_DRIVER_VERSION, sizeof(driver_version), driver_version, NULL);
    clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(compute_units), &compute_units, NULL);
    clGetDeviceInfo(device, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(clock_frequency), &clock_frequency, NULL);
    clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(global_mem_size), &global_mem_size, NULL);
    clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(local_mem_size), &local_mem_size, NULL);
    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(max_work_group_size), &max_work_group_size, NULL);
    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(max_work_item_sizes), max_work_item_sizes, NULL);

    printf("OpenCL device information:\n");
    printf("  Device name           : %s\n", device_name);
    printf("  Vendor                : %s\n", vendor_name);
    printf("  Device version        : %s\n", device_version);
    printf("  Driver version        : %s\n", driver_version);
    printf("  Compute units         : %u\n", compute_units);
    printf("  Max clock frequency   : %u MHz\n", clock_frequency);
    printf("  Global memory         : %.2f MB\n", (double)global_mem_size / (1024.0 * 1024.0));
    printf("  Local memory          : %.2f KB\n", (double)local_mem_size / 1024.0);
    printf("  Max work-group size   : %zu\n", max_work_group_size);
    printf("  Max work-item sizes   : %zu x %zu x %zu\n",
           max_work_item_sizes[0],
           max_work_item_sizes[1],
           max_work_item_sizes[2]);
}

//Julia-halmaz generálása OpenCL segítségével.
int generate_julia_opencl(Image *image,
                          int max_iter,
                          double c_real,
                          double c_imag,
                          const char *kernel_file,
                          double *transfer_time_ms) {
    //Bemeneti paraméterek ellenőrzése
    if (!image || !image->data || image->width <= 0 || image->height <= 0 || max_iter <= 0) {
        return 0;
    }

    //Alapértelmezett érték hiba esetére
    if (transfer_time_ms) {
        *transfer_time_ms = -1.0;
    }

    cl_int err = CL_SUCCESS;
    int result = 0;

    //OpenCL objektumok
    cl_platform_id platform = NULL;
    cl_device_id device = NULL;
    cl_context context = NULL;
    cl_command_queue queue = NULL;
    cl_program program = NULL;
    cl_kernel kernel = NULL;
    cl_mem image_buffer = NULL;

    //Kernel forrás
    char *source = NULL;
    size_t source_len = 0;

    //Képadat mérete byte-ban
    size_t image_size = get_image_size(image);

    //Kernel forrásfájl beolvasása
    source = load_kernel_source(kernel_file, &source_len);
    if (!source) {
        return 0;
    }

    //Platform lekérdezése
    err = clGetPlatformIDs(1, &platform, NULL);
    CHECK_CL(err, "clGetPlatformIDs");

    //Elsődlegesen GPU használata
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Warning: GPU is not available, trying CPU device.\n");
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
        CHECK_CL(err, "clGetDeviceIDs");
    }

    //Kiválasztott eszköz adatainak kiírása
    print_device_info(device);

    //OpenCL kontextus létrehozása
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    CHECK_CL(err, "clCreateContext");

    //Parancssor létrehozása
    #if defined(CL_VERSION_2_0)
        queue = clCreateCommandQueueWithProperties(context, device, 0, &err);
    #else
        queue = clCreateCommandQueue(context, device, 0, &err);
    #endif
    CHECK_CL(err, "clCreateCommandQueue");

    //Program létrehozása a kernel forrásból
    program = clCreateProgramWithSource(context, 1, (const char **)&source, &source_len, &err);
    CHECK_CL(err, "clCreateProgramWithSource");

    //Program fordítása
    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "clBuildProgram error: %d\n", err);
        print_build_log(program, device);
        goto cleanup;
    }

    //Kernel objektum létrehozása
    kernel = clCreateKernel(program, "julia_kernel", &err);
    CHECK_CL(err, "clCreateKernel");

    //Puffer létrehozása a képadatok számára
    image_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, image_size, NULL, &err);
    CHECK_CL(err, "clCreateBuffer");

    int width = image->width;
    int height = image->height;
    float c_real_f = (float)c_real;
    float c_imag_f = (float)c_imag;

    //Kernel argumentumainak beállítása
    err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &image_buffer);
    err |= clSetKernelArg(kernel, 1, sizeof(int), &width);
    err |= clSetKernelArg(kernel, 2, sizeof(int), &height);
    err |= clSetKernelArg(kernel, 3, sizeof(int), &max_iter);
    err |= clSetKernelArg(kernel, 4, sizeof(float), &c_real_f);
    err |= clSetKernelArg(kernel, 5, sizeof(float), &c_imag_f);
    CHECK_CL(err, "clSetKernelArg");

    //Globális munkaméret: egy work-item minden pixelhez
    size_t global_size[2] = { (size_t)width, (size_t)height };

    //Kernel futtatása
    err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_size, NULL, 0, NULL, NULL);
    CHECK_CL(err, "clEnqueueNDRangeKernel");

    //Megvárjuk a kernel futás végét
    err = clFinish(queue);
    CHECK_CL(err, "clFinish");

    //Az eredmény visszaolvasásának idejét külön mérjük
    double transfer_start_ms = now_ms();
    err = clEnqueueReadBuffer(queue, image_buffer, CL_TRUE, 0, image_size, image->data, 0, NULL, NULL);
    double transfer_end_ms = now_ms();
    CHECK_CL(err, "clEnqueueReadBuffer");

    //Adatmásolási idő eltárolása
    if (transfer_time_ms) {
        *transfer_time_ms = transfer_end_ms - transfer_start_ms;
    }

    result = 1;

    cleanup:
        //OpenCL erőforrások felszabadítása
        if (image_buffer) {
            clReleaseMemObject(image_buffer);
        }
        if (kernel) {
            clReleaseKernel(kernel);
        }
        if (program) {
            clReleaseProgram(program);
        }
        if (queue) {
            clReleaseCommandQueue(queue);
        }
        if (context) {
            clReleaseContext(context);
        }

        //Kernel forrás felszabadítása
        free(source);

    return result;
}