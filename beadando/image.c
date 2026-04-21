#include "image.h"

#include <stdio.h>
#include <stdlib.h>

//Kép létrehozása dinamikus memóriával.
//Beállítja a szélességet, magasságot és lefoglalja a pixel adatokat.
int create_image(Image *image, int width, int height) {
    // Hibás bemenet ellenőrzése
    if (!image || width <= 0 || height <= 0) {
        return -1;
    }

    // Méretek beállítása
    image->width = width;
    image->height = height;

    // Memóriafoglalás RGB képhez (3 byte/pixel)
    image->data = (unsigned char *)malloc((size_t)width * (size_t)height * 3);

    // Ha nem sikerült a memóriafoglalás
    if (!image->data) {
        image->width = 0;
        image->height = 0;
        return -1;
    }

    return 0;
}

//Kép felszabadítása.
//Felszabadítja a memóriát és nullázza a struktúrát.
void destroy_image(Image *image) {
    // NULL pointer ellenőrzése
    if (!image) {
        return;
    }

    // Memória felszabadítása
    free(image->data);

    // Pointer és méretek nullázása
    image->data = NULL;
    image->width = 0;
    image->height = 0;
}

//Kép méretének lekérdezése byte-ban.
size_t get_image_size(const Image *image) {
    // Hibás vagy üres kép ellenőrzése
    if (!image || !image->data || image->width <= 0 || image->height <= 0) {
        return 0;
    }

    // Méret kiszámítása: width * height * 3 (RGB)
    return (size_t)image->width * (size_t)image->height * 3;
}

//Kép kiírása PPM (P6) formátumban fájlba.
int write_ppm(const char *filename, const Image *image) {
    // Bemeneti paraméterek ellenőrzése
    if (!filename || !image || !image->data || image->width <= 0 || image->height <= 0) {
        return -1;
    }

    // Fájl megnyitása bináris írásra
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        return -1;
    }

    // PPM fejléc írása
    // P6 = bináris RGB formátum
    // 255 = maximális színmélység
    fprintf(fp, "P6\n%d %d\n255\n", image->width, image->height);

    // Pixel adatok kiírása
    size_t count = get_image_size(image);
    if (fwrite(image->data, 1, count, fp) != count) {
        fclose(fp);
        return -1;
    }

    // Fájl lezárása
    fclose(fp);
    return 0;
}