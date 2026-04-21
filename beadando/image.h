#ifndef IMAGE_H
#define IMAGE_H

#include <stddef.h>

//Egyszerű RGB kép struktúra.
typedef struct {
    int width;              // kép szélessége
    int height;             // kép magassága
    unsigned char *data;    // pixel adatok (R, G, B sorrendben)
} Image;

//Kép létrehozása.
//Lefoglalja a szükséges memóriát és inicializálja a struktúrát.
int create_image(Image *image, int width, int height);

//Kép felszabadítása.
//Felszabadítja a data memóriát és nullázza a mezőket.
void destroy_image(Image *image);

//Kép méretének lekérdezése byte-ban.
size_t get_image_size(const Image *image);

//Kép mentése PPM (P6) formátumban.
int write_ppm(const char *filename, const Image *image);

#endif