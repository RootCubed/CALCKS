#ifndef GUI_H_
#define GUI_H_

void gui_draw_char(int, int, int, int, int);
int gui_draw_string(const char *, int , int, int, int);
void gui_draw_image();

int gui_tab_button(const char *, int);

typedef struct {
    char *optionName;
    int numOptions;
    char *opt1;
    char *opt2;
    char *opt3;
} option;

#endif