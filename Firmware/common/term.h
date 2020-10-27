#ifndef TERM_H_
#define TERM_H_

#include "types.h"

#define NUM_0 0x00
#define NUM_1 0x01
#define NUM_2 0x02
#define NUM_3 0x03
#define NUM_4 0x04
#define NUM_5 0x05
#define NUM_6 0x06
#define NUM_7 0x07
#define NUM_8 0x08
#define NUM_9 0x09
#define NUM_A 0x0A
#define NUM_B 0x0B
#define NUM_C 0x0C
#define NUM_D 0x0D
#define NUM_E 0x0E
#define NUM_F 0x0F
#define NUM_POINT 0x10

#define OP_PLUS  0b00100000
#define OP_MINUS 0b00100001
#define OP_MULT  0b00100010
#define OP_DIV   0b00100011
#define OP_POW   0b00100101

#define OP_BRACK_OPEN  0b01000100
#define OP_BRACK_CLOSE 0b10000100

#define VAR_X 0b10100000
#define VAR_Y 0b10100001
#define VAR_Z 0b10100010

#define CHAR_END 0xFF

#define OPTYPE_CONST  0
#define OPTYPE_SIMPLE 1
#define OPTYPE_BEGIN  2
#define OPTYPE_MIDDLE 3
#define OPTYPE_END    4
#define OPTYPE_VAR    5

#define OPNODE_OP    0
#define OPNODE_CONST 1
#define OPNODE_VAR   2

#define VALTYPE_UNDEF  0
#define VALTYPE_NUMBER 1
#define VALTYPE_VAR    2
#define VALTYPE_OP     3

typedef struct opNode opNode;

struct opNode {
    u8 operation;
    opNode* parent;
    u8 op1Type; // 0: op, 1: const, 2: var
    u8 op2Type; // 0: op, 1: const, 2: var
    opNode* op1;
    opNode* op2;
    double val1;
    double val2;
    u8 varField1;
    u8 varField2;
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

opNode* node_stack_pop(opStack*);
void node_stack_remove(opStack*, opStackNode*);
void node_stack_push(opStack*, opNode*);
int node_stack_is_empty(opStack*);
int node_stack_length(opStack*);

void term_free(opNode*);
opNode* parse_term(u8*);
double evaluate_term(opNode*, double);
symbolField getFields(u8);

#endif /* TERM_H_ */