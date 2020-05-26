#ifndef GUI_H_
#define GUI_H_

void gui_drawChar(int, int, int, int, int);
int gui_drawString(const char *, int , int, int, int);
void gui_drawImage();

int gui_tabButton(const char *, int);

typedef struct {
    char *optionName;
    int numOptions;
    char *opt1;
    char *opt2;
    char *opt3;
} option;

#endif