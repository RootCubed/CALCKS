#include "solver.h"

solver solver_s;
char resBuf[32];

int solver_needsRedraw = 1;
int blinkCounter = 0;

void solver_init() {
    solver_s.a = mathinput_initBox(FNT_SM, 40, 13, 10);
    solver_s.b = mathinput_initBox(FNT_SM, 40, 13, 20);
    solver_s.c = mathinput_initBox(FNT_SM, 40, 13, 30);
    solver_s.currentSelected = 0;
    solver_s.solveType = SOLV_LINEAR;
}

void solver_free() {
    mathinput_freeBox(solver_s.a);
    mathinput_freeBox(solver_s.b);
    mathinput_freeBox(solver_s.c);
}

void solver_solveLinear() {
    double m = mathinput_calcContent(solver_s.a);
    double q = mathinput_calcContent(solver_s.b);
    double x = -q / m;

    solver_s.x1 = x;
}

void solver_redrawScreenLinear() {
    if (solver_s.currentSelected == 3) {
        disp_clear();
        mathinput_buttonPress(solver_s.a, enter);
        mathinput_buttonPress(solver_s.b, enter);

        solver_solveLinear();
        sprintf(resBuf, "x=%g", solver_s.x1);
        gui_draw_string(resBuf, 0, 26, FNT_MD, 0);
        gui_draw_char(SCREEN_WIDTH / 2 - 3, 0, CHAR_ARROW_UP, FNT_SM, 0);
    } else {
        gui_draw_string("mx+q=0", SCREEN_WIDTH / 2 - 3 * fonts[FNT_SM][2], 0, FNT_SM, (solver_s.currentSelected == -1));
        gui_draw_char(SCREEN_WIDTH - fonts[FNT_SM][2], 0, CHAR_ARROW_RIGHT, FNT_SM, 0);
        gui_draw_string("m=", 0, 10, FNT_SM, (solver_s.currentSelected == 0));
        gui_draw_string("q=", 0, 20, FNT_SM, (solver_s.currentSelected == 1));
        gui_draw_string("Calculate", 0, 45, FNT_MD, (solver_s.currentSelected == 2));

        mathinput_redraw(solver_s.a);
        mathinput_redraw(solver_s.b);

        mathinput_setCursor(solver_s.a, CURSOR_HIDDEN);
        mathinput_setCursor(solver_s.b, CURSOR_HIDDEN);
        if (solver_s.currentSelected == 0) {
            mathinput_setCursor(solver_s.a, CURSOR_ON);
        }
        if (solver_s.currentSelected == 1) {
            mathinput_setCursor(solver_s.b, CURSOR_ON);
        }
    }
}

void solver_solveQuadratic() {
    double a = mathinput_calcContent(solver_s.a);
    double b = mathinput_calcContent(solver_s.b);
    double c = mathinput_calcContent(solver_s.c);
    double x1 = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
    double x2 = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);

    double discr = b * b - 4 * a * c;
    if (discr == 0) {
        solver_s.numSolutions = 1;
    } else if (discr > 0) {
        solver_s.numSolutions = 2;
    } else {
        solver_s.numSolutions = 0;
    }

    solver_s.x1 = x1;
    solver_s.x2 = x2;
}

void solver_redrawScreenQuadratic() {
    if (solver_s.currentSelected == 4) {
        disp_clear();
        mathinput_buttonPress(solver_s.a, enter);
        mathinput_buttonPress(solver_s.b, enter);
        mathinput_buttonPress(solver_s.c, enter);

        solver_solveQuadratic();
        if (solver_s.numSolutions == 0) {
            gui_draw_string("No real solutions", 15, 26, FNT_SM, 0);
        }
        if (solver_s.numSolutions == 1) {
            sprintf(resBuf, "x=%g", solver_s.x1);
            gui_draw_string(resBuf, 0, 26, FNT_MD, 0);
        }
        if (solver_s.numSolutions == 2) {
            sprintf(resBuf, "x1=%g", solver_s.x1);
            gui_draw_string(resBuf, 0, 15, FNT_MD, 0);
            sprintf(resBuf, "x2=%g", solver_s.x2);
            gui_draw_string(resBuf, 0, 37, FNT_MD, 0);
        }
        gui_draw_char(SCREEN_WIDTH / 2 - 3, 0, CHAR_ARROW_UP, FNT_SM, 0);
    } else {
        gui_draw_string("ax^2+bx+c=0", SCREEN_WIDTH / 2 - 6 * fonts[FNT_SM][2], 0, FNT_SM, (solver_s.currentSelected == -1));
        gui_draw_char(0, 0, CHAR_ARROW_LEFT, FNT_SM, 0);
        gui_draw_string("a=", 0, 10, FNT_SM, (solver_s.currentSelected == 0));
        gui_draw_string("b=", 0, 20, FNT_SM, (solver_s.currentSelected == 1));
        gui_draw_string("c=", 0, 30, FNT_SM, (solver_s.currentSelected == 2));
        gui_draw_string("Calculate", 0, 45, FNT_MD, (solver_s.currentSelected == 3));

        mathinput_redraw(solver_s.a);
        mathinput_redraw(solver_s.b);
        mathinput_redraw(solver_s.c);

        mathinput_setCursor(solver_s.a, CURSOR_HIDDEN);
        mathinput_setCursor(solver_s.b, CURSOR_HIDDEN);
        mathinput_setCursor(solver_s.c, CURSOR_HIDDEN);
        if (solver_s.currentSelected == 0) {
            mathinput_setCursor(solver_s.a, CURSOR_ON);
        }
        if (solver_s.currentSelected == 1) {
            mathinput_setCursor(solver_s.b, CURSOR_ON);
        }
        if (solver_s.currentSelected == 2) {
            mathinput_setCursor(solver_s.c, CURSOR_ON);
        }
    }
}

void solver_updateScreen() {
    if (solver_needsRedraw) {
        disp_clear();
        switch (solver_s.solveType) {
            case SOLV_LINEAR:
                solver_redrawScreenLinear();
                break;
            case SOLV_QUADRATIC:
                solver_redrawScreenQuadratic();
                break;
        }
        solver_needsRedraw = 0;
    }
    mathinput_cursorFrame(solver_s.a);
    mathinput_cursorFrame(solver_s.b);
    mathinput_cursorFrame(solver_s.c);
}

int solver_buttonPress(int buttonID) {
    if (buttonID == back) {
        solver_needsRedraw = 1;
        solver_s.currentSelected = 0;
        mathinput_setCursor(solver_s.a, CURSOR_HIDDEN);
        mathinput_setCursor(solver_s.b, CURSOR_HIDDEN);
        mathinput_setCursor(solver_s.c, CURSOR_HIDDEN);
        return 0;
    }
    if (buttonID == left && solver_s.currentSelected == -1) {
        if (solver_s.solveType == SOLV_QUADRATIC) {
            solver_s.solveType = SOLV_LINEAR;
            solver_needsRedraw = 1;
            mathinput_clear(solver_s.a);
            mathinput_clear(solver_s.b);
            mathinput_clear(solver_s.c);
        }
    }
    if (buttonID == right && solver_s.currentSelected == -1) {
        if (solver_s.solveType == SOLV_LINEAR) {
            solver_s.solveType = SOLV_QUADRATIC;
            solver_needsRedraw = 1;
            mathinput_clear(solver_s.a);
            mathinput_clear(solver_s.b);
            mathinput_clear(solver_s.c);
        }
    }
    if (solver_s.currentSelected == 0) {
        mathinput_buttonPress(solver_s.a, buttonID);
        if (buttonID == enter || buttonID == up || buttonID == down) {
            if (mathinput_checkSyntax(solver_s.a) == 1) {
                solver_needsRedraw = 1;
                return 1;
            }
        }
    }
    if (solver_s.currentSelected == 1) {
        mathinput_buttonPress(solver_s.b, buttonID);
        if (buttonID == enter || buttonID == up || buttonID == down) {
            if (mathinput_checkSyntax(solver_s.b) == 1) {
                solver_needsRedraw = 1;
                return 1;
            }
        }
    }
    if (solver_s.solveType == SOLV_QUADRATIC && solver_s.currentSelected == 2) {
        mathinput_buttonPress(solver_s.c, buttonID);
        if (buttonID == enter || buttonID == up || buttonID == down) {
            if (mathinput_checkSyntax(solver_s.c) == 1) {
                solver_needsRedraw = 1;
                return 1;
            }
        }
    }

    if (buttonID == enter || buttonID == down) {
        solver_s.currentSelected++;
        solver_needsRedraw = 1;
    }

    if (buttonID == up) {
        if (solver_s.currentSelected > -1) {
            solver_s.currentSelected--;
            solver_needsRedraw = 1;
        }
    }
    return 0;
}