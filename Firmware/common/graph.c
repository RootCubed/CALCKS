#include "graph.h"
#include "term.h"
#include "gui.h"
#include "mathinput.h"

int graph_hasDrawn = 0;
int graph_shouldPlot = 0;
inputBox *graphInput;

void graph_init() {
    graphInput = mathinput_initBox(FNT_SM, 30, 5 * fonts[FNT_SM][FNT_WIDTH], 64 - fonts[FNT_SM][FNT_HEIGHT]);
}

double map(double value, double i1, double i2, double o1, double o2) {
    if (isnan(value) || !isfinite(value)) { // NaN or infinity
        return INFINITY;
    }
    double rangeInput = i2 - i1;
    double rangeOutput = o2 - o1;
    return (value - i1) * (rangeOutput / rangeInput) + o1;
}

void graph_updateScreen() {
    double rangeX1 = -10;
    double rangeX2 = 10;
    double rangeY1 = 10;
    double rangeY2 = -10;
    if (!graph_hasDrawn) {
        gui_clear_screen();

        double transfX0 = map(0, rangeX1, rangeX2, 0, SCREEN_WIDTH);
        double transfY0 = map(0, rangeY1, rangeY2, 0, 64);
        gui_draw_line(transfX0, 0, transfX0, SCREEN_HEIGHT);
        gui_draw_line(0, transfY0, SCREEN_WIDTH, transfY0);

        if (graph_shouldPlot && graphInput->buffer[0] != CHAR_END) {
            opNode *graphTerm = term_parse(graphInput->buffer);
            double prevRes = INFINITY;
            for (double x = 0; x < SCREEN_WIDTH; x += 0.1) {
                double transfX = map(x, 0, SCREEN_WIDTH, rangeX1, rangeX2);
                double funcRes = term_evaluate(graphTerm, transfX);
                int transfY = map(funcRes, rangeY1, rangeY2, 0, SCREEN_HEIGHT);

                if (x > 0 && 
                    transfY >= 0 && transfY <= SCREEN_HEIGHT &&
                    prevRes >= 0 && prevRes <= SCREEN_HEIGHT) { // allow bigger margin
                    gui_draw_line(x - 0.1, prevRes, x, transfY);
                }
                prevRes = transfY;
            }
            term_free(graphTerm, 1);
            graph_shouldPlot = 0;
            mathinput_clear(graphInput);
        }

        int fontHeight = fonts[FNT_SM][FNT_HEIGHT];
        gui_clear_rect(0, SCREEN_HEIGHT - fontHeight, SCREEN_WIDTH, fontHeight);
        gui_draw_string("f(x)=", 0, SCREEN_HEIGHT - fontHeight, FNT_SM, 0);
        mathinput_redraw(graphInput);
        mathinput_setCursor(graphInput, CURSOR_ON);
        
        graph_hasDrawn = 1;
    }
    mathinput_cursorFrame(graphInput);
}

void graph_reset_state() {
    graph_hasDrawn = 0;
    graph_shouldPlot = 0;
    mathinput_clear(graphInput);
}

int graph_buttonPress(int buttonID) {
    mathinput_buttonPress(graphInput, buttonID);

    if (buttonID == enter) {
        int hasSyntaxError = mathinput_checkSyntax(graphInput);
		graph_hasDrawn = 0;
        if (hasSyntaxError == 1) return 1;
        graph_shouldPlot = 1;
	}
    return 0;
}