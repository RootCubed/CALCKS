#include "mathinput.h"
#include <stdio.h>

inputBox *mathinput_initBox(int font, int maxChars, int x, int y) {
    inputBox *theBox = malloc(sizeof(inputBox));
    theBox->font = font;
    theBox->maxChars = maxChars;
    theBox->posX = x;
    theBox->posY = y;
    theBox->cursor = 0;
    theBox->buffer = malloc(maxChars * sizeof(char));
    theBox->buffer[0] = CHAR_END;
    theBox->currWidth = 0;
    return theBox;
}

void mathinput_freeBox(inputBox *box) {
    free(box->buffer);
    free(box);
}

int bufferCharToFontChar(int bufChar) {
    if (bufChar <= NUM_F) return bufChar;
    switch (bufChar) {
        case OP_PLUS:
            return CHAR_PLUS;
        case OP_MINUS:
            return CHAR_MINUS;
        case OP_MULT:
            return CHAR_MULT;
        case OP_DIV:
            return CHAR_DIV;
        case OP_POW:
            return CHAR_POW;
        case VAR_X:
            return 59;
        case OP_BRACK_OPEN:
            return CHAR_BROPEN;
        case OP_BRACK_CLOSE:
            return CHAR_BRCLOSE;
        case NUM_POINT:
            return CHAR_POINT;
        default:
            return -1;
    }
}

void mathinput_redraw(inputBox *box) {
    int pos = 0;
    while (box->buffer[pos] != CHAR_END && pos < box->maxChars) {
        int charToDraw = bufferCharToFontChar(box->buffer[pos]);
        if (charToDraw == -1) break;
        gui_draw_char(box->posX + pos * fonts[box->font][2], box->posY, charToDraw, box->font, 0);
        pos++;
    }
}

void mathinput_buttonPress(inputBox *box, int buttonID) {
    if (buttonID == enter) {
		box->cursor = -1;
        return;
	}
    if (buttonID != enter && box->cursor == -1) mathinput_clear(box);
    int charToPutInBuffer = -1;
	if (buttonID <= nine) {
        charToPutInBuffer = buttonID;
	} else if (buttonID <= divide) {
		charToPutInBuffer = (buttonID - plus) | (OPTYPE_SIMPLE << 5);
	}
	if (buttonID == variable) {
		charToPutInBuffer = VAR_X;
	}
	if (buttonID == bracket_open) {
		charToPutInBuffer = OP_BRACK_OPEN;
	}
	if (buttonID == bracket_close) {
		charToPutInBuffer = OP_BRACK_CLOSE;
	}
	if (buttonID == exponent) {
		charToPutInBuffer = OP_POW;
	}
    if (buttonID == point) {
        charToPutInBuffer = NUM_POINT;
    }
    int charToDraw = bufferCharToFontChar(charToPutInBuffer);
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
        gui_draw_rect(box->posX + box->cursor * fonts[box->font][2], box->posY, fonts[box->font][2] - 1, fonts[box->font][3] - 1, 1);
    } else {
        gui_clear_rect(box->posX + box->cursor * fonts[box->font][2], box->posY, fonts[box->font][2] - 1, fonts[box->font][3] - 1);
        int charToDraw = bufferCharToFontChar(box->buffer[box->cursor]);
        if (charToDraw != -1) {
            gui_draw_char(box->posX + box->cursor * fonts[box->font][2], box->posY, charToDraw, box->font, 0);
        }
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