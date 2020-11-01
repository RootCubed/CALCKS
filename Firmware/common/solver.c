#include "solver.h"

solver solver_s;
char resBuf[32];

int solver_needsRedraw = 1;
int blinkCounter = 0;

void solver_init() {
    solver_s.m = mathinput_initBox(FNT_MD, 20, 18, 10);
    solver_s.q = mathinput_initBox(FNT_MD, 20, 18, 28);
    solver_s.currentSelected = 0;
    mathinput_setCursor(solver_s.m, CURSOR_ON);
}

void solver_free() {
    mathinput_freeBox(solver_s.m);
    mathinput_freeBox(solver_s.q);
}

void solver_updateScreen() {
    if (solver_needsRedraw) {
        disp_clear();
        if (solver_s.currentSelected == 3) {
            // results
            mathinput_buttonPress(solver_s.m, enter);
            mathinput_buttonPress(solver_s.q, enter);
            double m = mathinput_calcContent(solver_s.m);
            double q = mathinput_calcContent(solver_s.q);
            double x = -q / m;
            sprintf(resBuf, "x=%g", x);
            gui_draw_string(resBuf, 0, 26, FNT_MD, 0);
        } else {
            gui_draw_string("mx+q=0", 0, 0, FNT_SM, 0);
            gui_draw_string("m=", 0, 10, FNT_MD, (solver_s.currentSelected == 0));
            gui_draw_string("q=", 0, 28, FNT_MD, (solver_s.currentSelected == 1));
            gui_draw_string("Calculate", 0, 45, FNT_MD, (solver_s.currentSelected == 2));

            mathinput_setCursor(solver_s.m, CURSOR_HIDDEN);
            mathinput_setCursor(solver_s.q, CURSOR_HIDDEN);
            if (solver_s.currentSelected == 0) {
                mathinput_setCursor(solver_s.m, CURSOR_ON);
            }
            if (solver_s.currentSelected == 1) {
                mathinput_setCursor(solver_s.q, CURSOR_ON);
            }

            mathinput_redraw(solver_s.m);
            mathinput_redraw(solver_s.q);
        }
        solver_needsRedraw = 0;
    }
    mathinput_cursorFrame(solver_s.m);
    mathinput_cursorFrame(solver_s.q);
}

void solver_buttonPress(int buttonID) {
    if (buttonID == back) {
        solver_needsRedraw = 1;
        solver_s.currentSelected = 0;
        mathinput_buttonPress(solver_s.m, enter);
        mathinput_buttonPress(solver_s.q, enter);
    }
    if (solver_s.currentSelected == 0) {
        mathinput_buttonPress(solver_s.m, buttonID);
    } else {
        mathinput_buttonPress(solver_s.q, buttonID);
    }

    if (buttonID == enter) {
        solver_s.currentSelected++;
        solver_needsRedraw = 1;
    }
}