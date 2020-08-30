#ifndef TERM_H_
#define TERM_H_

#include "types.h"

#define CHAR_0 0x30
#define CHAR_1 0x31
#define CHAR_2 0x32
#define CHAR_3 0x33
#define CHAR_4 0x34
#define CHAR_5 0x35
#define CHAR_6 0x36
#define CHAR_7 0x37
#define CHAR_8 0x38
#define CHAR_9 0x39
#define CHAR_A 0x3A
#define CHAR_B 0x3B
#define CHAR_C 0x3C
#define CHAR_D 0x3D
#define CHAR_E 0x3E
#define CHAR_F 0x3F

#define CHAR_ADD 0x00
#define CHAR_SUB 0x01
#define CHAR_MULT 0x02
#define CHAR_DIV 0x03

#define CHAR_END 0xFF

#define OPTYPE_CONST  0
#define OPTYPE_SIMPLE 1
#define OPTYPE_BEGIN  2
#define OPTYPE_MIDDLE 3
#define OPTYPE_END    4

typedef struct opNode {
    u8 operation;
    opNode* parent;
    opNode* op1;
    opNode* op2;
} opNode;

typedef struct symbolField {
    u8 type;
    u8 value;
} symbolField;

opNode* parse_term(char*);

#endif /* TERM_H_ */