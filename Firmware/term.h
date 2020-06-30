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

#define CHAR_SFRAC 0x10
#define CHAR_EFRAC 0x20
#define CHAR_SPOW 0x11
#define CHAR_EPOW 0x21

#define CHAR_END 255

#define OP_0 0
#define OP_1 1
#define OP_2 2
#define OP_3 3
#define OP_4 4
#define OP_5 5
#define OP_6 6
#define OP_7 7
#define OP_8 8
#define OP_9 9
#define OP_A 10
#define OP_B 11
#define OP_C 12
#define OP_D 13
#define OP_E 14
#define OP_F 15
#define OP_ADD 16
#define OP_SUB 17
#define OP_MULT 18
#define OP_DIV 19
#define OP_FRAC 20
#define OP_POW 21

typedef struct opNode {
    opNode* l;
    opNode* r;
    unsigned int val;
} opNode;

opNode* parse_input(char*);