#include "graph.h"
#include "term.h"
#include "display.h"
#include "gui.h"

int graph_hasDrawn = 0;

double map(double value, double i1, double i2, double o1, double o2) {
    double rangeInput = i2 - i1;
    double rangeOutput = o2 - o1;
    return (value - i1) * (rangeOutput / rangeInput) + o1;
}

void graph_draw(opNode* term) {
    double rangeX1 = -10;
    double rangeX2 = 10;
    double rangeY1 = 10;
    double rangeY2 = -10;
    if (!graph_hasDrawn) {
        disp_clear();
        double transf0x = map(0, rangeX1, rangeX2, 0, 128);
        double transf0y = map(0, rangeY1, rangeY2, 0, 64);
        gui_draw_line(0, transf0y, 128, transf0y);
        gui_draw_line(transf0x, 0, transf0x, 64);
        for (double x = 0; x < 128; x += 0.1) {
            double tfX = map(x, 0, 128, rangeX1, rangeX2);
            double funcRes = evaluate_term(term, tfX);
            int transfY = map(funcRes, rangeY1, rangeY2, 0, 64);
            if (transfY >= 0 && transfY <= 64) gui_set_pixel(x, transfY, 1);
        }
        graph_hasDrawn = 1;
    }
}

void graph_reset_state() {
    graph_hasDrawn = 0;
}