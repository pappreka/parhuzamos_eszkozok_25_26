#include "julia_cpu.h"

#include <stddef.h>

//Iterációszám alapján színt generáló függvény.
void color_map(int iter, int max_iter,
               unsigned char *r,
               unsigned char *g,
               unsigned char *b) {
    // Ha a pont nem szökik ki (a halmazon belül van), fekete szín
    if (iter >= max_iter) {
        *r = 0;
        *g = 0;
        *b = 0;
        return;
    }

    // Skálázott érték az iterációk alapján (0–255 tartomány)
    int value = (255 * iter) / max_iter;

    // Egyszerű színezési séma (gradiens)
    *r = (unsigned char)value;              // vörös komponens
    *g = (unsigned char)((value * 5) / 8);  // zöld komponens (csökkentett)
    *b = (unsigned char)(255 - value);      // kék komponens (invertált)
}

//Julia-halmaz generálása CPU-n (soros implementáció).
void generate_julia_cpu(Image *image,
                        int max_iter,
                        double c_real,
                        double c_imag) {

    // Bemeneti adatok ellenőrzése
    if (!image || !image->data || image->width <= 0 || image->height <= 0 || max_iter <= 0) {
        return;
    }

    // Komplex sík vizsgált tartománya
    const double xmin = -1.5;
    const double xmax = 1.5;
    const double ymin = -1.0;
    const double ymax = 1.0;

    // Végigmegyünk a kép minden pixelén
    for (int y = 0; y < image->height; ++y) {
        for (int x = 0; x < image->width; ++x) {

            // Pixel leképezése a komplex síkra
            double zx = xmin + (xmax - xmin) * x / (double)(image->width - 1);
            double zy = ymin + (ymax - ymin) * y / (double)(image->height - 1);

            int iter = 0;

            // Iteráció: z(n+1) = z(n)^2 + c
            while ((zx * zx + zy * zy <= 4.0) && (iter < max_iter)) {
                //valós rész
                double xtemp = zx * zx - zy * zy + c_real;
                //képzetes rész
                zy = 2.0 * zx * zy + c_imag;
                zx = xtemp;
                iter++;
            }

            // Szín kiszámítása az iterációk alapján
            unsigned char r, g, b;
            color_map(iter, max_iter, &r, &g, &b);

            // Pixel index kiszámítása (RGB -> 3 byte/pixel)
            size_t idx = ((size_t)y * (size_t)image->width + (size_t)x) * 3;

            // RGB értékek beírása a képadatba
            image->data[idx + 0] = r;
            image->data[idx + 1] = g;
            image->data[idx + 2] = b;
        }
    }
}