#include "term.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

u8 input[] = {NUM_8, OP_MULT, NUM_2, NUM_8, OP_DIV, NUM_2, OP_PLUS, NUM_3, NUM_8, OP_MINUS, NUM_3, OP_MULT, NUM_2, OP_PLUS, NUM_3, NUM_8, NUM_8, CHAR_END};

opNode* node_stack_pop(opStack* stack) {
    opStackNode* top = stack->top;
    opNode* opPtr = top->ptr;
    opStackNode* second = top->next;
    free(top);
    stack->top = second;
    if (second != NULL) {
        second->prev = NULL;
    }
    return opPtr;
}

void node_stack_remove(opStack* stack, opStackNode* node) {
    if (node->next != NULL) {
        node->next->prev = node->prev;
    }
    if (node->prev != NULL) {
        node->prev->next = node->next;
    } else {
        if (node->next != NULL) {
            stack->top = node->next;
        } else {
            stack->top = NULL;
        }
    }
    free(node);
}

void node_stack_push(opStack* stack, opNode* toInsert) {
    opStackNode* newNode = (opStackNode *) malloc(sizeof(opStackNode));
    memset(newNode, 0, sizeof(opStackNode));
    newNode->ptr = toInsert;
    if (stack->top != NULL) {
        newNode->next = stack->top;
    } else {
        newNode->next = NULL;
    }
    newNode->prev = NULL;

    opStackNode* top = stack->top;
    if (top != NULL) {
        top->prev = newNode;
    }
    stack->top = newNode;
}

int node_stack_is_empty(opStack* stack) {
    return stack->top == NULL;
}

int node_stack_length(opStack* stack) {
    int counter = 0;
    opStackNode* current = stack->top;
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
    printf("%s(", opNameBuf);
    if (n->usedOperands & 0b01) {
        printf("%ld, ", n->val1);
    } else {
        print_node(n->op1);
        printf(", ");
    }
    if (n->usedOperands & 0b10) {
        printf("%ld", n->val2);
    } else {
        print_node(n->op2);
    }
    printf(")");
}

void term_free(opNode* node) {
    opNode currNode = *node;
    if ((currNode.usedOperands & 0b01) == 0) {
        term_free(currNode.op1);
    }
    if ((currNode.usedOperands & 0b10) == 0) {
        term_free(currNode.op2);
    }
    free(node);
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
                    int currOpPre = PRECEDENCE[currOp->operation];
                    if (currOpPre > PRECEDENCE[stack.top->ptr->operation]) {
                        // if the precedence of the new operator is higher, leave the stack as-is
                    } else {
                        // first, find the node furthest back that still has higher precedence
                        opStackNode* currOpInStack = stack.top;
                        while (currOpInStack->next != NULL && currOpPre <= PRECEDENCE[currOpInStack->next->ptr->operation]) currOpInStack = currOpInStack->next;
                        // connect currOp to that node
                        currOpInStack->ptr->parent = currOp;
                        currOp->op1 = currOpInStack->ptr;
                        currOp->usedOperands &= ~0b01;
                        // now, while traversing the stack upwards, connect the nodes with eachother
                        while (currOpInStack != NULL) {
                            opStackNode* nextOpInStack = currOpInStack->prev;
                            if (nextOpInStack == NULL) break;
                            nextOpInStack->ptr->parent = currOpInStack->ptr;
                            currOpInStack->ptr->op2 = nextOpInStack->ptr;
                            currOpInStack->ptr->usedOperands &= ~0b10;
                            node_stack_remove(&stack, currOpInStack);
                            currOpInStack = nextOpInStack;
                        }
                        // finally, add the constant value as val2 on the last operation in the stack
                        currOpInStack->ptr->val2 = currNum;
                        currOpInStack->ptr->usedOperands |= 0b10;
                        node_stack_remove(&stack, currOpInStack);
                    }
                }
                node_stack_push(&stack, currOp);
                currNum = 0;
                printf("Current stack: ");
                opStackNode* curr = stack.top;
                char opNameBuf[16];
                while (curr != NULL) {
                    switch(curr->ptr->operation) {
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
                    printf("%s ", opNameBuf);
                    curr = curr->next;
                }
                printf("\n");
                break;
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
    printf("\n");
    return currOp;
}

double evaluate_term(opNode* startNode) {
    opNode currNode = *startNode;
    double value1;
    if (currNode.usedOperands & 0b01) {
        value1 = currNode.val1;
    } else {
        value1 = evaluate_term(currNode.op1);
    }
    double value2;
    if (currNode.usedOperands & 0b10) {
        value2 = currNode.val2;
    } else {
        value2 = evaluate_term(currNode.op2);
    }
    switch (currNode.operation) {
        case 0:
            return value1 + value2;
        case 1:
            return value1 - value2;
        case 2:
            return value1 * value2;
        case 3:
            return value1 / value2;
    }
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
    int i = 0;
    while (input[i] != CHAR_END) {
        print_char_console(input[i]);
        i++;
    }
    printf("\n");
    opNode* term = parse_term(input);
    printf("res = %f\n", evaluate_term(term));
    term_free(term);
}