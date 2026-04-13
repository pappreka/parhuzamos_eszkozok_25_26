#ifndef IMAGE_H
#define IMAGE_H

#include <stddef.h>

typedef struct {
    int width;
    int height;
    unsigned char *data;
} Image;

int create_image(Image *image, int width, int height);
void destroy_image(Image *image);
size_t get_image_size(const Image *image);
int write_ppm(const char *filename, const Image *image);

#endif