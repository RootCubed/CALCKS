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
    theBox->cursorX = x;
    theBox->cursorY = y;
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

void mathinput_recalcLayout(inputBox *box) {
    int nextCharX = box->posX - box->scroll;
    int nextCharY = box->posY;
    int pos = 0;
    int foundCursor = 0;
    while ((!foundCursor || nextCharX < SCREEN_WIDTH) && pos < box->length) {
        if (pos == box->cursor) {
            box->cursorX = nextCharX;
            box->cursorY = nextCharY;
            if (nextCharX < box->posX) {
                box->scroll += nextCharX;
                return;
            }
            foundCursor = 1;
        }
        mathinput_drawChar(box, box->buffer[pos], &nextCharX, &nextCharY, 0);
        if (pos == box->cursor) {
            if (nextCharX > SCREEN_WIDTH) {
                box->scroll += nextCharX - SCREEN_WIDTH;
                return;
            }
        }
        pos++;
    }
    if (!foundCursor && nextCharX >= SCREEN_WIDTH) {
        box->scroll += (nextCharX + fonts[box->font][FNT_WIDTH]) - SCREEN_WIDTH;
        return;
    }
    if (pos == box->cursor) {
        box->cursorX = nextCharX;
        box->cursorY = nextCharY;
    }
}

void mathinput_redraw(inputBox *box) {
    gui_clear_rect(box->posX, box->posY, SCREEN_WIDTH - box->posX, fonts[box->font][FNT_HEIGHT] + 1);
    int nextCharX = box->posX - box->scroll;
    int nextCharY = box->posY;
    int pos = 0;
    while (nextCharX < SCREEN_WIDTH && pos < box->length) {
        mathinput_drawChar(box, box->buffer[pos], &nextCharX, &nextCharY, 1);
        pos++;
    }
    if (box->posX - box->scroll < box->posX) {
        gui_draw_char(box->posX, box->posY, CHAR_ARROW_LEFT, box->font, 1);
    }
    if (nextCharX >= SCREEN_WIDTH) {
        gui_draw_char(SCREEN_WIDTH - fonts[box->font][2], box->posY, CHAR_ARROW_RIGHT, box->font, 1);
    }
}

void mathinput_drawChar(inputBox *box, u8 c, int *x, int *y, int display) {
    if (c == CHAR_END) return;
    int fontCharWidth = fonts[box->font][FNT_WIDTH];
    u8 character = 0;
    int oX = *x;
    int oY = *y;
    if (c <= TERM_CONST_F) {
        character = c;
    } else {
        switch (c) {
            case TERM_OP_PLUS:
                character = CHAR_PLUS;
                break;
            case TERM_OP_MINUS:
                character = CHAR_MINUS;
                break;
            case TERM_OP_MULT:
                character = CHAR_MULT;
                break;
            case TERM_OP_DIV:
                character = CHAR_DIV;
                break;
            case TERM_OP_POW:
                character = CHAR_POW;
                break;
            case TERM_VAR_X:
                character = 'x' - 'a' + 36;
                break;
            case TERM_OP_BRACK_OPEN:
                character = CHAR_BROPEN;
                break;
            case TERM_OP_BRACK_CLOSE:
                character = CHAR_BRCLOSE;
                break;
            case TERM_CONST_POINT:
                character = CHAR_POINT;
                break;
            case TERM_OP_SIN:
                *x += 4 * fontCharWidth;
                if (x > 0 && display) gui_draw_string("sin(", oX, oY, box->font, 0);
                return;
            case TERM_OP_COS:
                *x += 4 * fontCharWidth;
                if (x > 0 && display) gui_draw_string("cos(", oX, oY, box->font, 0);
                return;
            case TERM_OP_TAN:
                *x += 4 * fontCharWidth;
                if (x > 0 && display) gui_draw_string("tan(", oX, oY, box->font, 0);
                return;
        }
    }
    *x += fontCharWidth;
    if (*x > 0 && *x < SCREEN_WIDTH && display) {
        gui_draw_char(oX, oY, character, box->font, 0);
    }
}

void mathinput_buttonPress(inputBox *box, int buttonID) {
    if (buttonID == enter) {
        mathinput_setCursor(box, CURSOR_HIDDEN);
        box->buffer[box->length] = CHAR_END;
		box->cursor = -1;
        mathinput_recalcLayout(box);
        mathinput_redraw(box);
        return;
	}
    if (buttonID != enter && box->cursor == -1) {
        mathinput_clear(box);
        mathinput_recalcLayout(box);
    }

    unsigned char charToPutInBuffer = 255;
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
    if (buttonID == btn_cos) {
        charToPutInBuffer = TERM_OP_COS;
    }
    if (buttonID == btn_tan) {
        charToPutInBuffer = TERM_OP_TAN;
    }
    
    if (charToPutInBuffer != 255) {
        box->buffer[box->cursor] = charToPutInBuffer;

        if (box->cursor == box->length) {
            box->length++;
        }
        int tmpX = box->cursorX;
        int tmpY = box->cursorY;
        mathinput_drawChar(box, charToPutInBuffer, &tmpX, &tmpY, 0);
        if (tmpX > SCREEN_WIDTH) {
            mathinput_recalcLayout(box);
            mathinput_redraw(box);
        } else {
            mathinput_drawChar(box, charToPutInBuffer, &box->cursorX, &box->cursorY, 1);
        }
        box->cursor++;
    }

    if (buttonID == btn_left) {
        mathinput_setCursor(box, CURSOR_OFF);
        if (box->cursor > 0) box->cursor--;
        int oX = box->scroll;
        mathinput_recalcLayout(box);
        if (oX != box->scroll) {
            mathinput_redraw(box);
        }
        mathinput_setCursor(box, CURSOR_ON);
    }

    if (buttonID == btn_right) {
        mathinput_setCursor(box, CURSOR_OFF);
        if (box->cursor < box->length) box->cursor++;
        int oX = box->scroll;
        mathinput_recalcLayout(box);
        if (oX != box->scroll) {
            mathinput_redraw(box);
        }
        mathinput_setCursor(box, CURSOR_ON);
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
            }
            mathinput_recalcLayout(box);
            mathinput_redraw(box);
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
    int fontWidth = fonts[box->font][FNT_WIDTH];
    int fontHeight = fonts[box->font][FNT_HEIGHT];
    if (onOff == CURSOR_ON) {
        gui_draw_rect(box->cursorX, box->cursorY, fontWidth, fontHeight, 1);
    } else {
        gui_clear_rect(box->cursorX, box->cursorY, fontWidth, fontHeight + 1);
        if (box->cursor >= box->length) return;
        int tmpX = box->cursorX;
        int tmpY = box->cursorY;
        mathinput_drawChar(box, box->buffer[box->cursor], &tmpX, &tmpY, 1);
    }
}

void mathinput_clear(inputBox *box) {
    gui_clear_rect(box->posX, box->posY, box->length * fonts[box->font][FNT_WIDTH], fonts[box->font][FNT_HEIGHT] + 1); // height + 1 because of inverted text taking up one more space
    box->scroll = 0;
    box->cursor = 0;
    box->length = 0;
    memset(box->buffer, CHAR_END, box->maxChars);
}

int mathinput_checkSyntax(inputBox *box) {
    int syntaxErrorLoc = term_checkSyntax(box->buffer);
    if (syntaxErrorLoc > -1) {
        box->cursor = syntaxErrorLoc;
        mathinput_recalcLayout(box);
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