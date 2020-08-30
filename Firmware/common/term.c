#include "term.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_LENGTH 9
u8 input[] = {NUM_1, OP_PLUS, NUM_2, NUM_8, OP_MULT, NUM_7, OP_PLUS, NUM_3, NUM_3, CHAR_END};

opNode* node_stack_pop(opStack* stack) {
    opStackNode* first = stack->first;
    opNode* opPtr = first->ptr;
    opStackNode* second = first->next;
    free(first);
    stack->first = second;
    if (second != NULL) {
        second->prev = NULL;
    }
    return opPtr;
}

void node_stack_push(opStack* stack, opNode* toInsert) {
    opStackNode* newNode = (opStackNode *) malloc(sizeof(opStackNode));
    memset(newNode, 0, sizeof(opStackNode));
    newNode->ptr = toInsert;
    if (stack->first != NULL) {
        newNode->next = stack->first;
    } else {
        newNode->next = NULL;
    }
    newNode->prev = NULL;

    opStackNode* first = stack->first;
    if (first != NULL) {
        first->prev = newNode;
    }
    stack->first = newNode;
}

int node_stack_is_empty(opStack* stack) {
    return stack->first == NULL;
}

int node_stack_length(opStack* stack) {
    int counter = 0;
    opStackNode* current = stack->first;
    while (current != NULL) {
        counter++;
        current = current->next;
    }
    return counter;
}

void print_node(opNode* n) {
    char opNameBuf[16];
    switch(n->operation) {
        case 0:
            strcpy(opNameBuf, "plus");
            break;
        case 1:
            strcpy(opNameBuf, "minus");
            break;
        case 2:
            strcpy(opNameBuf, "mult");
            break;
        case 3:
            strcpy(opNameBuf, "div");
            break;
    }
    printf("%s[", opNameBuf);
    if (n->usedOperands & 0b01) {
        printf("%d, ", n->val1);
    } else {
        print_node(n->op1);
        printf(", ");
    }
    if (n->usedOperands & 0b10) {
        printf("%d", n->val2);
    } else {
        print_node(n->op2);
    }
    printf("]");
}

opNode* parse_term(u8* input) {
    u64 currNum = 0;
    opStack stack = {NULL};
    u8 pos = 0;
    opNode* currOp;
    while (input[pos] != CHAR_END) {
        symbolField f = getFields(input[pos]);
        switch (f.type) {
            case OPTYPE_CONST:
                currNum *= 10;
                currNum += f.value;
                break;
            case OPTYPE_SIMPLE:
                currOp = (opNode *) malloc(sizeof(opNode));
                memset(currOp, 0, sizeof(opNode));
                currOp->operation = f.value;

                // set val1 in case of non-empty stack, or if the current operator has higher precedence than the previous one
                currOp->val1 = currNum;
                currOp->usedOperands |= 0b01;
                if (!node_stack_is_empty(&stack)) {
                    opStackNode* currOpInStack = stack.first;
                    while (currOpInStack != NULL) {
                        opNode* opInStack = currOpInStack->ptr;
                        if (PRECEDENCE[currOp->operation] >= PRECEDENCE[opInStack->operation]) {
                            break;
                        }
                        opNode* poppedNode = node_stack_pop(&stack);
                        poppedNode->parent = currOp;
                        poppedNode->val2 = currNum;
                        poppedNode->usedOperands |= 0b10;
                        currOp->op1 = poppedNode;
                        currOp->usedOperands &= ~0b01;
                        currOpInStack = stack.first;
                    }
                }
                node_stack_push(&stack, currOp);
                currNum = 0;
        }
        pos++;
    }
    currOp->val2 = currNum;
    currOp->usedOperands |= 0b10;
    node_stack_pop(&stack); // remove currOp from stack
    printf("Nodes remaining on stack: %d\n", node_stack_length(&stack));
    while (!node_stack_is_empty(&stack)) {
        opNode* poppedNode = node_stack_pop(&stack);
        currOp->parent = poppedNode;
        poppedNode->op2 = currOp;
        poppedNode->usedOperands &= ~0b10;
        currOp = poppedNode;
    }
    printf("Traversing nodes:\n");
    print_node(currOp);
}

symbolField getFields(u8 symbol) {
    symbolField res = {symbol >> 5, symbol & 0b11111};
    return res;
}

void print_char_console(u8 input) {
    if (getFields(input).type == OPTYPE_CONST) {
        printf("%d", input);
    } else {
        char opNameBuf[16];
        switch(getFields(input).value) {
            case 0:
                strcpy(opNameBuf, "+");
                break;
            case 1:
                strcpy(opNameBuf, "-");
                break;
            case 2:
                strcpy(opNameBuf, "*");
                break;
            case 3:
                strcpy(opNameBuf, "/");
                break;
        }
        printf("%s", opNameBuf);
    }
}

int main(int argc, char** argv) {
    for (int i = 0; i < INPUT_LENGTH; i++) {
        print_char_console(input[i]);
    }
    printf("\n");
    parse_term(input);
}