#include "graph.h"
#include "term.h"
#include "display.h"
#include "gui.h"
#include "mathinput.h"

int graph_hasDrawn = 0;
inputBox *graphInput;

void graph_init() {
    graphInput = mathinput_initBox(FNT_SM, 30, 5 * fonts[FNT_SM][2], 64 - fonts[FNT_SM][3]);
}

double map(double value, double i1, double i2, double o1, double o2) {
    if (isnan(value) || !isfinite(value)) { // NaN or infinity
        return o1 - 1;
    }
    double rangeInput = i2 - i1;
    double rangeOutput = o2 - o1;
    return (value - i1) * (rangeOutput / rangeInput) + o1;
}

void graph_draw(char* term) {
    double rangeX1 = -10;
    double rangeX2 = 10;
    double rangeY1 = 10;
    double rangeY2 = -10;
    if (!graph_hasDrawn) {
        disp_clear();
        opNode *termTree = parse_term(term);
        double transf0x = map(0, rangeX1, rangeX2, 0, 128);
        double transf0y = map(0, rangeY1, rangeY2, 0, 64);
        gui_draw_line(0, transf0y, 127, transf0y);
        gui_draw_line(transf0x, 0, transf0x, 64);
        double prevRes = 0;
        for (double x = 0; x < 128; x += 0.1) {
            double tfX = map(x, 0, 128, rangeX1, rangeX2);
            double funcRes = evaluate_term(termTree, tfX);
            int transfY = map(funcRes, rangeY1, rangeY2, 0, 64);

            if (x > 0 && ((transfY >= 0 && transfY <= 64) || (prevRes >= 0 && prevRes <= 64))) {
                gui_draw_line(x - 0.1, prevRes, x, transfY);
            }
            prevRes = transfY;
        }
        graph_hasDrawn = 1;
        term_free(termTree);

        gui_clear_rect(0, 64 - fonts[FNT_SM][3], 128, 64);
        gui_draw_string("f(x)=", 0, 64 - fonts[FNT_SM][3], FNT_SM, 0);
        mathinput_clear(graphInput);
    }
}

void graph_reset_state() {
    graph_hasDrawn = 0;
}

void graph_buttonPress(int buttonID) {
    mathinput_buttonPress(graphInput, buttonID);
    
    if (buttonID == enter) {
		graph_hasDrawn = 0;
        graph_draw(graphInput->buffer);
	}
}