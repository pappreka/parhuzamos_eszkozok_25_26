#include "julia_cpu.h"
#include <stdlib.h>

static void color_map(int iter, int max_iter,
                      unsigned char *r,
                      unsigned char *g,
                      unsigned char *b) {
    if (iter >= max_iter) {
        *r = 0;
        *g = 0;
        *b = 0;
        return;
    }

    int value = (255 * iter) / max_iter;

    *r = (unsigned char)value;
    *g = (unsigned char)((value * 5) / 8);
    *b = (unsigned char)(255 - value);
}

void generate_julia_cpu(unsigned char *image,
                        int width,
                        int height,
                        int max_iter,
                        double c_real,
                        double c_imag) {
    const double xmin = -1.5;
    const double xmax = 1.5;
    const double ymin = -1.0;
    const double ymax = 1.0;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double zx = xmin + (xmax - xmin) * x / (double)(width - 1);
            double zy = ymin + (ymax - ymin) * y / (double)(height - 1);

            int iter = 0;
            while ((zx * zx + zy * zy <= 4.0) && (iter < max_iter)) {
                double xtemp = zx * zx - zy * zy + c_real;
                zy = 2.0 * zx * zy + c_imag;
                zx = xtemp;
                iter++;
            }

            unsigned char r, g, b;
            color_map(iter, max_iter, &r, &g, &b);

            size_t idx = ((size_t)y * (size_t)width + (size_t)x) * 3;
            image[idx + 0] = r;
            image[idx + 1] = g;
            image[idx + 2] = b;
        }
    }
}