#ifndef TIMER_H
#define TIMER_H

//Aktuális idő lekérdezése milliszekundumban.
double now_ms(void);

//Mérési eredmények hozzáfűzése CSV fájlhoz.
int append_result_csv(const char *filename,
                      int width,
                      int height,
                      int max_iter,
                      double c_real,
                      double c_imag,
                      double cpu_time_ms,
                      double opencl_time_ms,
                      double transfer_time_ms);

#endif