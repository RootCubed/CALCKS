#include "solver.h"

solver solver_s;
char resBuf[32];

int hasDrawn = 0;
int blinkCounter = 0;

void solver_init() {
    solver_s.m = mathinput_initBox(FNT_MD, 20, 18, 10);
    solver_s.q = mathinput_initBox(FNT_MD, 20, 18, 35);
    solver_s.currentSelected = 0;
}

void solver_free() {
    mathinput_freeBox(solver_s.m);
    mathinput_freeBox(solver_s.q);
}

int solver_updateScreen() {
    if (!hasDrawn) {
        disp_clear();
        gui_draw_string("mx+q=0", 0, 0, FNT_SM, 0);
        gui_draw_string("m=", 0, 10, FNT_MD, 1);
        gui_draw_string("q=", 0, 35, FNT_MD, 0);
        gui_draw_string("Calculate", 0, 50, FNT_MD, 0);
        hasDrawn = 1;
    }
    return 1;
}

void solver_buttonPress(int buttonID) {
    if (buttonID == back) {
        hasDrawn = 0;
        solver_s.currentSelected = 0;
    }
    if (solver_s.currentSelected == 0) {
        mathinput_buttonPress(solver_s.m, buttonID);
    } else {
        mathinput_buttonPress(solver_s.q, buttonID);
    }

    if (buttonID == enter) {
        solver_s.currentSelected++;
        switch (solver_s.currentSelected) {
            case 1:
                gui_draw_string("m=", 0, 10, FNT_MD, 0);
                gui_draw_string("q=", 0, 35, FNT_MD, 1);
                break;
            case 2:
                gui_draw_string("q=", 0, 35, FNT_MD, 0);
                gui_draw_string("Calculate", 0, 50, FNT_MD, 1);
                break;
            case 3:
                disp_clear();
                double m = mathinput_calcContent(solver_s.m);
                double q = mathinput_calcContent(solver_s.q);
                double x = -q / m;
                sprintf(resBuf, "x=%g", x);
                gui_draw_string(resBuf, 0, 20, FNT_MD, 0);
                break;
        }
    }
}