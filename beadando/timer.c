#include "timer.h"

#include <stdio.h>
#include <sys/time.h>

double now_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec * 1000.0 + (double)tv.tv_usec / 1000.0;
}

int append_result_csv(const char *filename,
                      int width,
                      int height,
                      int max_iter,
                      double c_real,
                      double c_imag,
                      double cpu_time_ms,
                      double opencl_time_ms) {
    int write_header = 0;

    FILE *test = fopen(filename, "r");
    if (!test) {
        write_header = 1;
    } else {
        fclose(test);
    }

    FILE *fp = fopen(filename, "a");
    if (!fp) {
        return -1;
    }

    if (write_header) {
        fprintf(fp, "width,height,max_iter,c_real,c_imag,cpu_ms,opencl_ms,speedup\n");
    }

    double speedup = -1.0;
    if (opencl_time_ms > 0.0) {
        speedup = cpu_time_ms / opencl_time_ms;
    }

    fprintf(fp, "%d,%d,%d,%.10f,%.10f,%.6f,%.6f,%.6f\n",
            width, height, max_iter, c_real, c_imag,
            cpu_time_ms, opencl_time_ms, speedup);

    fclose(fp);
    return 0;
}