#ifndef GUI_H_
#define GUI_H_

#include "display.h"
#include "eep.h"
#include <math.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define PIXEL_W_H_RATIO 0.85714

#define FNT_SM 0
#define FNT_MD 1

#define FNT_ADDR 0
#define FNT_LEN 1
#define FNT_WIDTH 2
#define FNT_HEIGHT 3

// character set
#define CHAR_PLUS 62
#define CHAR_MINUS 63
#define CHAR_MULT 64
#define CHAR_DIV 65
#define CHAR_EQUALS 66
#define CHAR_POW 73
#define CHAR_BROPEN 68
#define CHAR_BRCLOSE 69
#define CHAR_POINT 71
#define CHAR_COLON 72
#define CHAR_PERCENT 74
#define CHAR_ARROW_UP 75
#define CHAR_ARROW_DOWN 76
#define CHAR_ARROW_RIGHT 77
#define CHAR_ARROW_LEFT 78

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