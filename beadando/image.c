#include "image.h"

#include <stdio.h>

int write_ppm(const char *filename, const unsigned char *image, int width, int height) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        return -1;
    }

    fprintf(fp, "P6\n%d %d\n255\n", width, height);

    size_t count = (size_t)width * (size_t)height * 3;
    if (fwrite(image, 1, count, fp) != count) {
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}