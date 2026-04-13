#ifndef TIMER_H
#define TIMER_H

double now_ms(void);

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