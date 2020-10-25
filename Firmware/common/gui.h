#ifndef GUI_H_
#define GUI_H_

#include "display.h"
#include "eep.h"
#include <math.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

extern const int fonts[2][4];

void gui_draw_byte(char, int, int);
void gui_update_byte(char, int, int);

void gui_draw_char(int, int, int, int, int);
int gui_draw_string(const char *, int , int, int, int);
void gui_draw_image(char[1024]);

void gui_set_pixel(int, int, int);
void gui_draw_line(int, int, int, int);
void gui_clear_line(int, int, int, int);
void gui_draw_rect(int, int, int, int, int);
void gui_draw_circle(int, int, int, int);
void gui_clear_rect(int, int, int, int);
int gui_tab_button(const char *, int);

typedef struct {
    char *optionName;
    int numOptions;
    char *opt1;
    char *opt2;
    char *opt3;
} option;

#endif