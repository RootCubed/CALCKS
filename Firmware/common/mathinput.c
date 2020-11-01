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
    theBox->length = 0;
    theBox->scroll = 0;
    theBox->cursorBlinkState = CURSOR_HIDDEN;
    theBox->cursorFrameCount = 0;
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
    int posOffset = MIN(SCREEN_WIDTH - box->posX, (box->length - box->scroll) * fonts[box->font][2]);
    gui_clear_rect(box->posX, box->posY, posOffset, fonts[box->font][3]);
    int pos = box->scroll;
    int drawPos = 0;
    if (pos > 0) {
        gui_draw_char(box->posX, box->posY, CHAR_ARROW_LEFT, box->font, 1);
        drawPos = 1;
        pos++;
    }
    while (pos < box->length && box->posX + drawPos * fonts[box->font][2] < SCREEN_WIDTH) {
        int charToDraw = bufferCharToFontChar(box->buffer[pos]);
        if (charToDraw == -1) break;
        gui_draw_char(box->posX + drawPos * fonts[box->font][2], box->posY, charToDraw, box->font, 0);
        pos++;
        drawPos++;
    }
    if (box->posX + drawPos * fonts[box->font][2] >= SCREEN_WIDTH) {
        gui_draw_char(SCREEN_WIDTH - fonts[box->font][2], box->posY, CHAR_ARROW_RIGHT, box->font, 1);
    }
}

void mathinput_buttonPress(inputBox *box, int buttonID) {
    if (buttonID == enter) {
        mathinput_setCursor(box, CURSOR_HIDDEN);
        box->buffer[box->length] = CHAR_END;
        box->scroll = 0;
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
        if (box->cursor == box->length) {
            box->length++;
        }
        if (box->posX + (box->cursor + 2) * fonts[box->font][2] > SCREEN_WIDTH) {
            box->scroll++;
            gui_clear_rect(box->posX, box->posY, box->length * fonts[box->font][2], fonts[box->font][3]);
            mathinput_redraw(box);
        } else {
            gui_draw_char(box->posX + (box->cursor - box->scroll) * fonts[box->font][2], box->posY, charToDraw, box->font, 0);
        }
        box->cursor++;
    }

    if (buttonID == left) {
        mathinput_setCursor(box, CURSOR_OFF);
        if (box->cursor > 0) box->cursor--;
        if (box->cursor < box->scroll) {
            box->scroll = box->cursor;
            mathinput_redraw(box);
        }
        mathinput_setCursor(box, CURSOR_ON);
    }
    
    if (buttonID == right) {
        mathinput_setCursor(box, CURSOR_OFF);
        if (box->cursor < box->length) box->cursor++;
        int hadToChangeScroll = 0;
        while (box->posX + (box->cursor - box->scroll + 1) * fonts[box->font][2] > SCREEN_WIDTH) {
            box->scroll++;
            hadToChangeScroll = 1;
        }
        if (hadToChangeScroll) {
            mathinput_redraw(box);
        }
        mathinput_setCursor(box, CURSOR_ON);
        if (box->scroll > 0 && box->cursor > box->scroll) {
            gui_draw_char(box->posX, box->posY, CHAR_ARROW_LEFT, box->font, 1);
        }
    }

    if (box->cursorBlinkState == CURSOR_HIDDEN) box->cursorBlinkState++;
    mathinput_setCursor(box, CURSOR_ON);
}

void mathinput_cursorFrame(inputBox *box) {
    if (box->cursorBlinkState != CURSOR_HIDDEN) {
        box->cursorFrameCount = (box->cursorFrameCount + 1);
        if (box->cursorFrameCount == 63) {
            mathinput_setCursor(box, !box->cursorBlinkState);
        }
    }
}

void mathinput_setCursor(inputBox *box, int onOff) {
    box->cursorBlinkState = onOff;
    box->cursorFrameCount = 0;
    if (box->cursor == -1) return;
    int fontWidth = fonts[box->font][2];
    int fontHeight = fonts[box->font][3];
    if (onOff == CURSOR_ON) {
        gui_draw_rect(box->posX + (box->cursor - box->scroll) * fontWidth, box->posY, fontWidth, fontHeight, 1);
    } else {
        gui_clear_rect(box->posX + (box->cursor - box->scroll) * fontWidth, box->posY, fontWidth, fontHeight);
        int charToDraw = bufferCharToFontChar(box->buffer[box->cursor]);
        if (box->cursor < box->length && charToDraw != -1) {
            gui_draw_char(box->posX + (box->cursor - box->scroll) * fontWidth, box->posY, charToDraw, box->font, 0);
        }
    }
}

void mathinput_clear(inputBox *box) {
    gui_clear_rect(box->posX, box->posY, box->length * fonts[box->font][2], fonts[box->font][3]);
    box->scroll = 0;
    box->cursor = 0;
    box->length = 0;
    box->buffer[0] = CHAR_END;
}

double mathinput_calcContent(inputBox *box) {
    opNode *termTree = parse_term(box->buffer);
    double res = evaluate_term(termTree, 0);
    term_free(termTree);
    return res;
}