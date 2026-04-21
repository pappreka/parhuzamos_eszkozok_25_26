//Ha nem OpenCL környezetben fordítjuk (pl. CPU fallback vagy teszt), akkor definiáljuk a szükséges kulcsszavakat, hogy sima C-ben is leforduljon.
#ifndef __OPENCL_VERSION__
#define __kernel
#define __global
#define get_global_id(x) 0
#define uchar unsigned char
#endif

//Julia-halmaz kernel (GPU-n fut).
//Minden work-item egy pixelért felel.
__kernel void julia_kernel(__global uchar *image,
                           int width,
                           int height,
                           int max_iter,
                           float c_real,
                           float c_imag) {

    //A work-item (szál) globális koordinátái
    int x = get_global_id(0);
    int y = get_global_id(1);

    //Határellenőrzés
    if (x >= width || y >= height) {
        return;
    }

    //Komplex sík tartomány
    float xmin = -1.5f;
    float xmax = 1.5f;
    float ymin = -1.0f;
    float ymax = 1.0f;

    //Pixel leképezése komplex síkra
    float zx = xmin + (xmax - xmin) * (float)x / (float)(width - 1);
    float zy = ymin + (ymax - ymin) * (float)y / (float)(height - 1);

    int iter = 0;

    //Julia iteráció: z(n+1) = z(n)^2 + c
    while ((zx * zx + zy * zy <= 4.0f) && (iter < max_iter)) {
        float xtemp = zx * zx - zy * zy + c_real;
        zy = 2.0f * zx * zy + c_imag;
        zx = xtemp;
        iter++;
    }

    //Színképzés
    uchar r, g, b;

    if (iter >= max_iter) {
        //Halmazon belüli pont fekete
        r = 0;
        g = 0;
        b = 0;
    } else {
        //Iteráció alapján színezés
        int value = (255 * iter) / max_iter;

        r = (uchar)value;
        g = (uchar)((value * 5) / 8);
        b = (uchar)(255 - value);
    }

    //Pixel index (RGB → 3 byte/pixel)
    int idx = (y * width + x) * 3;

    //Szín kiírása a globális memóriába
    image[idx + 0] = r;
    image[idx + 1] = g;
    image[idx + 2] = b;
}