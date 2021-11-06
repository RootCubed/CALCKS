#include "mathinput.h"
#include <stdio.h>
#include <string.h>

inputBox *mathinput_initBox(int font, int maxChars, int x, int y) {
    inputBox *theBox = malloc(sizeof(inputBox));
    theBox->font = font;
    theBox->maxChars = maxChars;
    theBox->posX = x;
    theBox->posY = y;
    theBox->cursor = 0;
    theBox->buffer = malloc(maxChars * sizeof(unsigned char));
    memset(theBox->buffer, CHAR_END, theBox->maxChars);
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

int bufferCharToFontChar(unsigned char bufChar) {
    if (bufChar <= TERM_CONST_F) return bufChar;
    switch (bufChar) {
        case TERM_OP_PLUS:
            return CHAR_PLUS;
        case TERM_OP_MINUS:
            return CHAR_MINUS;
        case TERM_OP_MULT:
            return CHAR_MULT;
        case TERM_OP_DIV:
            return CHAR_DIV;
        case TERM_OP_POW:
            return CHAR_POW;
        case TERM_VAR_X:
            return 59;
        case TERM_OP_BRACK_OPEN:
            return CHAR_BROPEN;
        case TERM_OP_BRACK_CLOSE:
            return CHAR_BRCLOSE;
        case TERM_CONST_POINT:
            return CHAR_POINT;
        case TERM_OP_SIN:
            return 's' - 'a' + 36;
        default:
            return -1;
    }
}

void mathinput_redraw(inputBox *box) {
    // Commented out so that the extra character gets cleared when a character somewhere else gets deleted
    //int posOffset = MIN(SCREEN_WIDTH - box->posX, (box->length - box->scroll) * fonts[box->font][2]);
    gui_clear_rect(box->posX, box->posY, SCREEN_WIDTH - box->posX, fonts[box->font][3] + 1);
    if (box->cursor > box->scroll + (SCREEN_WIDTH - box->posX) / fonts[box->font][2]) {
        box->scroll = ((box->cursor * fonts[box->font][2]) - (SCREEN_WIDTH - box->posX)) / fonts[box->font][2] + 1;
    }
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

    unsigned char charToPutInBuffer = -1;
	if (buttonID <= nine) {
        charToPutInBuffer = buttonID;
	} else if (buttonID <= divide) {
		charToPutInBuffer = (buttonID - plus) | (OPTYPE_SIMPLE << 5);
	}
	if (buttonID == variable) {
		charToPutInBuffer = TERM_VAR_X;
	}
	if (buttonID == bracket_open) {
		charToPutInBuffer = TERM_OP_BRACK_OPEN;
	}
	if (buttonID == bracket_close) {
		charToPutInBuffer = TERM_OP_BRACK_CLOSE;
	}
	if (buttonID == exponent) {
		charToPutInBuffer = TERM_OP_POW;
	}
    if (buttonID == point) {
        charToPutInBuffer = TERM_CONST_POINT;
    }
    if (buttonID == btn_sin) {
        charToPutInBuffer = TERM_OP_SIN;
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

    if (buttonID == del && box->cursor > -1) {
        if (box->length == 0) {
            mathinput_setCursor(box, CURSOR_ON);
        } else {
            if (box->cursor == box->length) { // after last character
                box->length--;
                mathinput_setCursor(box, CURSOR_OFF);
                box->cursor--;
                mathinput_setCursor(box, CURSOR_ON);
            } else {
                int i = box->cursor;
                box->length--;
                while (i < box->length) {
                    box->buffer[i] = box->buffer[i + 1];
                    i++;
                }
                mathinput_redraw(box);
            }
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
    if (box->cursor == -1) {
        if (box->cursorBlinkState == CURSOR_HIDDEN) return;
        box->cursor = 0;
    };
    int fontWidth = fonts[box->font][2];
    int fontHeight = fonts[box->font][3];
    if (onOff == CURSOR_ON) {
        gui_draw_rect(box->posX + (box->cursor - box->scroll) * fontWidth, box->posY, fontWidth, fontHeight, 1);
    } else {
        gui_clear_rect(box->posX + (box->cursor - box->scroll) * fontWidth, box->posY, fontWidth, fontHeight + 1);
        int charToDraw = bufferCharToFontChar(box->buffer[box->cursor]);
        if (box->cursor < box->length && charToDraw != -1) {
            gui_draw_char(box->posX + (box->cursor - box->scroll) * fontWidth, box->posY, charToDraw, box->font, 0);
        }
    }
}

void mathinput_clear(inputBox *box) {
    gui_clear_rect(box->posX, box->posY, box->length * fonts[box->font][2], fonts[box->font][3] + 1); // height + 1 because of inverted text taking up one more space
    box->scroll = 0;
    box->cursor = 0;
    box->length = 0;
    memset(box->buffer, CHAR_END, box->maxChars);
}

int mathinput_checkSyntax(inputBox *box) {
    int syntaxErrorLoc = term_checkSyntax(box->buffer);
    if (syntaxErrorLoc > -1) {
        box->cursor = syntaxErrorLoc;
        mathinput_setCursor(box, CURSOR_HIDDEN);
        return 1;
    }
    return 0;
}

double mathinput_calcContent(inputBox *box) {
    opNode *termTree = term_parse(box->buffer);
    double res = term_evaluate(termTree, 0);
    term_free(termTree, 1);
    return res;
}