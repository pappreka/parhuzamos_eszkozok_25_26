#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "julia_cpu.h"
#include "julia_opencl.h"
#include "image.h"
#include "timer.h"

static void print_usage(const char *prog) {
    fprintf(stderr,
        "Hasznalat:\n"
        "  %s <width> <height> <max_iter> <c_real> <c_imag> <output_base> <csv_file>\n\n"
        "Pelda:\n"
        "  %s 1920 1080 500 -0.7 0.27015 output results.csv\n",
        prog, prog
    );
}

static int compare_buffers(const unsigned char *a, const unsigned char *b, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 8) {
        print_usage(argv[0]);
        return 1;
    }

    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    int max_iter = atoi(argv[3]);
    double c_real = atof(argv[4]);
    double c_imag = atof(argv[5]);
    const char *output_base = argv[6];
    const char *csv_file = argv[7];

    if (width <= 0 || height <= 0 || max_iter <= 0) {
        fprintf(stderr, "Hiba: a szelesseg, magassag es max_iter pozitiv kell legyen.\n");
        return 1;
    }

    size_t image_size = (size_t)width * (size_t)height * 3;

    unsigned char *cpu_image = (unsigned char *)malloc(image_size);
    unsigned char *ocl_image = (unsigned char *)malloc(image_size);

    if (!cpu_image || !ocl_image) {
        fprintf(stderr, "Hiba: nem sikerult memoriat foglalni.\n");
        free(cpu_image);
        free(ocl_image);
        return 1;
    }

    double t0, t1;
    double cpu_time_ms, opencl_time_ms;

    t0 = now_ms();
    generate_julia_cpu(cpu_image, width, height, max_iter, c_real, c_imag);
    t1 = now_ms();
    cpu_time_ms = t1 - t0;

    t0 = now_ms();
    int ocl_ok = generate_julia_opencl(ocl_image, width, height, max_iter, c_real, c_imag, "kernel.cl");
    t1 = now_ms();
    opencl_time_ms = t1 - t0;

    char cpu_name[512];
    char ocl_name[512];

    snprintf(cpu_name, sizeof(cpu_name), "%s_cpu.ppm", output_base);
    snprintf(ocl_name, sizeof(ocl_name), "%s_opencl.ppm", output_base);

    if (write_ppm(cpu_name, cpu_image, width, height) != 0) {
        fprintf(stderr, "Hiba: nem sikerult menteni: %s\n", cpu_name);
    }

    if (ocl_ok) {
        if (write_ppm(ocl_name, ocl_image, width, height) != 0) {
            fprintf(stderr, "Hiba: nem sikerult menteni: %s\n", ocl_name);
        }
    }

    printf("CPU ido: %.3f ms\n", cpu_time_ms);

    if (ocl_ok) {
        printf("OpenCL ido: %.3f ms\n", opencl_time_ms);

        if (opencl_time_ms > 0.0) {
            printf("Gyorsulas: %.3fx\n", cpu_time_ms / opencl_time_ms);
        }

        if (compare_buffers(cpu_image, ocl_image, image_size)) {
            printf("A CPU es OpenCL kimenet megegyezik.\n");
        } else {
            printf("Figyelem: a CPU es OpenCL kimenet nem teljesen egyezik.\n");
        }
    } else {
        printf("OpenCL futtatas sikertelen volt.\n");
    }

    if (append_result_csv(csv_file, width, height, max_iter, c_real, c_imag,
                          cpu_time_ms, ocl_ok ? opencl_time_ms : -1.0) != 0) {
        fprintf(stderr, "Hiba: nem sikerult a CSV fajlba irni.\n");
    }

    free(cpu_image);
    free(ocl_image);

    return 0;
}