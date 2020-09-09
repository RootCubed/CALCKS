#include "mandel.h"
#include "gui.h"
#include "display.h"

int mandel_hasDrawn = 0;

typedef struct {
    double real;
    double imag;
} complex;

#define MAX_ITERATIONS 20

void mandel_draw() {
    if (!mandel_hasDrawn) {
        disp_clear();
        complex z, c, newZ;
        for (int y = 0; y < 64; y++) {
            for (int x = 0; x < 128; x++) {
                z.real = 0;
                z.imag = 0;
                c.real = (x - 64) / 64.0 - 0.5;
                c.imag = (y - 32) / 32.0;
                int iter = 0;
                do {
                    newZ.real = z.real * z.real - z.imag * z.imag + c.real;
                    newZ.imag = z.real * z.imag + z.imag * z.real + c.imag;
                    z.real = newZ.real;
                    z.imag = newZ.imag;
                    if (z.real * z.real + z.imag * z.imag > 4) {
                        break;
                    }
                    iter++;
                } while (iter < MAX_ITERATIONS);
                if (iter == MAX_ITERATIONS) gui_set_pixel(x, y, 1);
            }
        }
        mandel_hasDrawn = 1;
    }
}

void mandel_reset_state() {
    mandel_hasDrawn = 0;
}