#ifndef TERM_H_
#define TERM_H_

#include "types.h"

#define OPTYPE_CONST  0
#define OPTYPE_SIMPLE 1
#define OPTYPE_BEGIN  2
#define OPTYPE_MIDDLE 3
#define OPTYPE_END    4
#define OPTYPE_VAR    5

#define NO_SYNTAX_ERROR (-1)

#define CONSTOP(x) ((OPTYPE_CONST << 5) | (x))
#define SIMPLEOP(x) ((OPTYPE_SIMPLE << 5) | (x))
#define BEGINOP(x) ((OPTYPE_BEGIN << 5) | (x))
#define MIDDLEOP(x) ((OPTYPE_MIDDLE << 5) | (x))
#define ENDOP(x) ((OPTYPE_END << 5) | (x))
#define VAROP(x) ((OPTYPE_VAR << 5) | (x))

// constants

#define TERM_0 0
#define TERM_1 1
#define TERM_2 2
#define TERM_3 3
#define TERM_4 4
#define TERM_5 5
#define TERM_6 6
#define TERM_7 7
#define TERM_8 8
#define TERM_9 9
#define TERM_A 10
#define TERM_B 11
#define TERM_C 12
#define TERM_D 13
#define TERM_E 14
#define TERM_F 15
#define TERM_POINT 16

#define TERM_CONST_0     CONSTOP(TERM_0)
#define TERM_CONST_1     CONSTOP(TERM_1)
#define TERM_CONST_2     CONSTOP(TERM_2)
#define TERM_CONST_3     CONSTOP(TERM_3)
#define TERM_CONST_4     CONSTOP(TERM_4)
#define TERM_CONST_5     CONSTOP(TERM_5)
#define TERM_CONST_6     CONSTOP(TERM_6)
#define TERM_CONST_7     CONSTOP(TERM_7)
#define TERM_CONST_8     CONSTOP(TERM_8)
#define TERM_CONST_9     CONSTOP(TERM_9)
#define TERM_CONST_A     CONSTOP(TERM_A)
#define TERM_CONST_B     CONSTOP(TERM_B)
#define TERM_CONST_C     CONSTOP(TERM_C)
#define TERM_CONST_D     CONSTOP(TERM_D)
#define TERM_CONST_E     CONSTOP(TERM_E)
#define TERM_CONST_F     CONSTOP(TERM_F)
#define TERM_CONST_POINT CONSTOP(TERM_POINT)

// operations

#define TERM_PLUS    0
#define TERM_MINUS   1
#define TERM_MULT    2
#define TERM_DIV     3
#define TERM_POW     4
#define TERM_BRACKET 5
#define TERM_SIN     6
#define TERM_COS     7
#define TERM_TAN     8

#define TERM_OP_PLUS  SIMPLEOP(TERM_PLUS)
#define TERM_OP_MINUS SIMPLEOP(TERM_MINUS)
#define TERM_OP_MULT  SIMPLEOP(TERM_MULT)
#define TERM_OP_DIV   SIMPLEOP(TERM_DIV)
#define TERM_OP_POW   SIMPLEOP(TERM_POW)

#define TERM_OP_BRACK_OPEN  BEGINOP(TERM_BRACKET)
#define TERM_OP_BRACK_CLOSE ENDOP(TERM_BRACKET)

#define TERM_OP_SIN   BEGINOP(TERM_SIN)
#define TERM_OP_COS   BEGINOP(TERM_COS)
#define TERM_OP_TAN   BEGINOP(TERM_TAN)

// variables

#define TERM_X 0
#define TERM_Y 1
#define TERM_Z 2

#define TERM_VAR_X VAROP(TERM_X)
#define TERM_VAR_Y VAROP(TERM_Y)
#define TERM_VAR_Z VAROP(TERM_Z)

#define OPNODE_OP    0
#define OPNODE_CONST 1
#define OPNODE_VAR   2

#define CHAR_END 0xFF

typedef struct opNode opNode;

typedef union {
    double d;
    int v;
    opNode* op;
} termArg;

struct opNode {
    u8 operation;
    opNode* parent;
    int op1Type;
    int op2Type;
    termArg arg1;
    termArg arg2;
};

typedef struct opStackNode opStackNode;

struct opStackNode {
    opNode* ptr;
    opStackNode* prev;
    opStackNode* next;
};

typedef struct {
    opStackNode* top;
} opStack;

typedef struct symbolField {
    u8 type;
    u8 value;
} symbolField;

opNode *node_stack_pop(opStack *);
void node_stack_remove(opStack *, opStackNode *);
void node_stack_push(opStack *, opNode *);
int node_stack_is_empty(opStack *);
int node_stack_length(opStack *);

void term_init();
void term_free(opNode *, int);
int term_checkSyntax(u8 *);
void term_print_node(opNode *);
opNode *term_parse(u8 *);
void term_simplify(opNode *);
double term_evaluate(opNode *, double);
symbolField getFields(u8);
void print_char_console(u8);

#endif /* TERM_H_ */