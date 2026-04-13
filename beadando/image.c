#include "image.h"

#include <stdio.h>
#include <stdlib.h>

int create_image(Image *image, int width, int height) {
    if (!image || width <= 0 || height <= 0) {
        return -1;
    }

    image->width = width;
    image->height = height;
    image->data = (unsigned char *)malloc((size_t)width * (size_t)height * 3);

    if (!image->data) {
        image->width = 0;
        image->height = 0;
        return -1;
    }

    return 0;
}

void destroy_image(Image *image) {
    if (!image) {
        return;
    }

    free(image->data);
    image->data = NULL;
    image->width = 0;
    image->height = 0;
}

size_t get_image_size(const Image *image) {
    if (!image || !image->data || image->width <= 0 || image->height <= 0) {
        return 0;
    }

    return (size_t)image->width * (size_t)image->height * 3;
}

int write_ppm(const char *filename, const Image *image) {
    if (!filename || !image || !image->data || image->width <= 0 || image->height <= 0) {
        return -1;
    }

    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        return -1;
    }

    fprintf(fp, "P6\n%d %d\n255\n", image->width, image->height);

    size_t count = get_image_size(image);
    if (fwrite(image->data, 1, count, fp) != count) {
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}