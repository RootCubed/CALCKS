#include "mathinput.h"

inputBox *mathinput_initBox(int font, int maxChars, int x, int y) {
    inputBox *theBox = malloc(sizeof(inputBox));
    theBox->font = font;
    theBox->maxChars = maxChars;
    theBox->posX = x;
    theBox->posY = y;
    theBox->cursor = 0;
    theBox->buffer = malloc(maxChars * sizeof(char));
    theBox->currWidth = 0;
    return theBox;
}

void mathinput_freeBox(inputBox *box) {
    free(box->buffer);
    free(box);
}

void mathinput_redraw(inputBox *box) {
    int pos = 0;
    while (box->buffer[pos] != CHAR_END) {
        pos++;
    }
}

void mathinput_buttonPress(inputBox *box, int buttonID) {
    if (buttonID == enter) {
		box->cursor = -1;
        return;
	}
    if (buttonID != enter && box->cursor == -1) mathinput_clear(box);
    int charToDraw = -1;
    int charToPutInBuffer = -1;
	if (buttonID <= nine) {
        charToPutInBuffer = buttonID;
        charToDraw = buttonID;
	} else if (buttonID <= divide) {
		charToPutInBuffer = (buttonID - plus) | (OPTYPE_SIMPLE << 5);
        charToDraw = 62 + (buttonID - plus);
	}
	if (buttonID == variable) {
		charToPutInBuffer = VAR_X;
        charToDraw = 59; // x
	}
	if (buttonID == bracket_open) {
		charToPutInBuffer = OP_BRACK_OPEN;
        charToDraw = 68; // (
	}
	if (buttonID == bracket_close) {
		charToPutInBuffer = OP_BRACK_CLOSE;
        charToDraw = 69; // )
	}
    if (charToDraw != -1) {
        box->buffer[box->cursor] = charToPutInBuffer;
        box->buffer[box->cursor + 1] = CHAR_END;
        gui_draw_char(box->posX + box->cursor * fonts[box->font][2], box->posY, charToDraw, box->font, 0);
        box->cursor++;
        box->currWidth += fonts[box->font][2];
    }
}

void mathinput_blinkCursor(inputBox *box, int onOff) {
    if (onOff) {
        gui_draw_rect(box->posX + box->cursor * fonts[box->font][2], box->posY, fonts[box->font][2], fonts[box->font][3], 1);
    } else {
        gui_clear_rect(box->posX + box->cursor * fonts[box->font][2], box->posY, fonts[box->font][2], fonts[box->font][3]);
    }
}

void mathinput_clear(inputBox *box) {
    gui_clear_rect(box->posX, box->posY, box->currWidth, fonts[box->font][3]);
    box->cursor = 0;
    box->currWidth = 0;
    box->buffer[0] = CHAR_END;
}

double mathinput_calcContent(inputBox *box) {
    opNode *termTree = parse_term(box->buffer);
    double res = evaluate_term(termTree, 0);
    term_free(termTree);
    return res;
}