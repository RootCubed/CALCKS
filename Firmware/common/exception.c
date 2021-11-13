#include "exception.h"
#include "apps.h"
#include "gui.h"

#include <stdio.h>
#include <string.h>

jmp_buf exception_state;
char *exception_msg;

void exception_call(char *msg) {
    exception_msg = msg;
    exception_draw(msg);
    longjmp(exception_state, 1);
}

void exception_draw(char *msg) {
    gui_clear_screen();
    gui_draw_string("Exception handler", 10, 0, FNT_SM, 1);
    gui_draw_string(msg, 0, 10, FNT_SM, 0);

    gui_draw_string("Trace:", 0, 30, FNT_SM, 0);
    char buf[128];
    memset(buf, 0, sizeof(buf));
    
    #ifndef console
    for (int i = 0; i < sizeof(exception_state->_jb); i++) {
        char tmp[8];
        sprintf(tmp, "%02x", exception_state->_jb[i]);
        strcat(buf, tmp);
        if (i % 8 == 0 && i > 0) strcat(buf, "\n");
    }
    #else
    strcpy(buf, "Not available\nin online version");
    #endif

    gui_draw_string(buf, 0, 40, FNT_SM, 0);
}