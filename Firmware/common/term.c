#include "term.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

const u8 PRECEDENCE[] = {
    1, 1, // plus, minus
    2, 2, // mult, div
    0,    // brackets
    3,    // power
};

//u8 input[] = {OP_BRACK_OPEN, NUM_4, OP_DIV, NUM_2, OP_PLUS, NUM_3, NUM_8, OP_BRACK_CLOSE, OP_MULT, NUM_2, CHAR_END};

opNode* node_stack_pop(opStack* stack) {
    opStackNode* top = stack->top;
    opNode* opPtr = top->ptr;
    opStackNode* second = top->prev;
    free(top);
    stack->top = second;
    if (second != NULL) {
        second->next = NULL;
    }
    return opPtr;
}

void node_stack_remove(opStack* stack, opStackNode* node) {
    if (node->next != NULL) {
        node->next->prev = node->prev;
    } else {
        stack->top = node->prev;
    }
    if (node->prev != NULL) {
        node->prev->next = node->next;
    }
    free(node);
}

void node_stack_push(opStack* stack, opNode* toInsert) {
    opStackNode* newNode = (opStackNode *) malloc(sizeof(opStackNode));
    memset(newNode, 0, sizeof(opStackNode));
    newNode->ptr = toInsert;
    if (stack->top != NULL) {
        newNode->prev = stack->top;
    } else {
        newNode->prev = NULL;
    }
    newNode->next = NULL;

    if (stack->top != NULL) {
        stack->top->next = newNode;
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
        current = current->prev;
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
    if (n->op1Type == OPNODE_CONST) {
        printf("%ld, ", n->val1);
    } else if (n->op1Type == OPNODE_VAR) {
        printf("var%d, ", n->varField1);
    } else {
        if (n->op1 != NULL) print_node(n->op1);
        printf(", ");
    }
    if (n->op2Type == OPNODE_CONST) {
        printf("%ld", n->val2);
    } else if (n->op2Type == OPNODE_VAR) {
        printf("var%d", n->varField2);
    } else {
        if (n->op2 != NULL) print_node(n->op2);
    }
    printf(")");
}

void term_free(opNode* node) {
    opNode currNode = *node;
    if (currNode.op1Type == OPNODE_OP) {
        term_free(currNode.op1);
    }
    if (currNode.op2Type == OPNODE_OP) {
        term_free(currNode.op2);
    }
    free(node);
}

opNode* parse_term(u8* input) {
    double currNum = 0;
    u8 currVar = 0;
    u8 currValType = VALTYPE_NUMBER;
    opNode* currValOp;
    opStack stack = {NULL};
    u8 pos = 0;
    opNode* currOp = NULL;
    int currNumAfterPoint = -1;
    while (input[pos] != CHAR_END) {
        symbolField f = getFields(input[pos]);
        if (f.type != OPTYPE_CONST) {
            currNumAfterPoint = -1;
        }
        switch (f.type) {
            case OPTYPE_CONST:
                if (f.value == NUM_POINT) {
                    currNumAfterPoint = 0;
                    break;
                }
                currValType = VALTYPE_NUMBER;
                double valueToAdd = f.value;
                if (currNumAfterPoint == -1) {
                    currNum *= 10;
                } else {
                    currNumAfterPoint++;
                    for (int i = 0; i < currNumAfterPoint; i++) {
                        valueToAdd /= 10;
                    }
                }
                currNum += valueToAdd;
                break;
            case OPTYPE_SIMPLE:
                currOp = (opNode *) malloc(sizeof(opNode));
                memset(currOp, 0, sizeof(opNode));
                currOp->operation = f.value;

                // set val1/varField1 in case of non-empty stack, or if the current operator has higher precedence than the previous one
                if (currValType == VALTYPE_NUMBER) {
                    currOp->val1 = currNum;
                    currOp->op1Type = OPNODE_CONST;
                } else if (currValType == VALTYPE_VAR) {
                    currOp->varField1 = currVar;
                    currOp->op1Type = OPNODE_VAR;
                } else if (currValType == VALTYPE_OP) {
                    currOp->op1 = currValOp;
                    currOp->op1Type = OPNODE_OP;
                } 
                if (!node_stack_is_empty(&stack)) {
                    int currOpPre = PRECEDENCE[currOp->operation];
                    if (currOpPre > PRECEDENCE[stack.top->ptr->operation]) {
                        // if the precedence of the new operator is higher, leave the stack as-is
                    } else {
                        // first, find the node furthest back that has lower precedence
                        opStackNode* currOpInStack = stack.top;
                        while (currOpInStack->prev != NULL && currOpPre <= PRECEDENCE[currOpInStack->prev->ptr->operation]) {
                            currOpInStack = currOpInStack->prev;
                        }
                        // connect currOp to that node
                        currOpInStack->ptr->parent = currOp;
                        currOp->op1 = currOpInStack->ptr;
                        currOp->op1Type = OPNODE_OP;
                        // now, while traversing the stack upwards, connect the nodes with eachother
                        while (currOpInStack->next != NULL) {
                            opStackNode* nextOpInStack = currOpInStack->next;
                            nextOpInStack->ptr->parent = currOpInStack->ptr;
                            currOpInStack->ptr->op2 = nextOpInStack->ptr;
                            currOpInStack->ptr->op2Type = OPNODE_OP;
                            node_stack_remove(&stack, currOpInStack);
                            currOpInStack = nextOpInStack;
                        }
                        // finally, add the constant value/variable as val2 on the last operation in the stack
                        if (currValType == VALTYPE_NUMBER) {
                            currOpInStack->ptr->val2 = currNum;
                            currOpInStack->ptr->op2Type = OPNODE_CONST;
                        } else if (currValType == VALTYPE_VAR) {
                            currOpInStack->ptr->varField2 = currVar;
                            currOpInStack->ptr->op2Type = OPNODE_VAR;
                        } else if (currValType == VALTYPE_OP) {
                            currOpInStack->ptr->op2 = currValOp;
                            currOpInStack->ptr->op2Type = OPNODE_OP;
                        }
                        node_stack_remove(&stack, currOpInStack);
                    }
                }
                node_stack_push(&stack, currOp);
                currNum = 0;
                break;
            case OPTYPE_VAR:
                currValType = VALTYPE_VAR;
                currVar = f.value;
                currNum = 0;
                break;
            case OPTYPE_BEGIN:
                currOp = (opNode *) malloc(sizeof(opNode));
                memset(currOp, 0, sizeof(opNode));
                currOp->operation = f.value;
                node_stack_push(&stack, currOp);
                break;
            case OPTYPE_END:
                if (f.value == (OP_BRACK_CLOSE & 0b11111)) {
                    opStackNode* currOpInStack = stack.top;
                    //printf("current type is %d\n", currValType);
                    if (currValType == VALTYPE_NUMBER) {
                        currOpInStack->ptr->val2 = currNum;
                        currOpInStack->ptr->op2Type = OPNODE_CONST;
                    } else if (currValType == VALTYPE_VAR) {
                        currOpInStack->ptr->varField2 = currVar;
                        currOpInStack->ptr->op2Type = OPNODE_VAR;
                    } else if (currValType == VALTYPE_OP) {
                        currOpInStack->ptr->op2 = currValOp;
                        currOpInStack->ptr->op2Type = OPNODE_OP;
                    }
                    opNode* lastRemaining;
                    while (currOpInStack != NULL) {
                        if (currOpInStack->ptr->operation == (OP_BRACK_OPEN & 0b11111)) break;
                        // connect the nodes with each other
                        opStackNode* nextOpInStack = currOpInStack->prev;
                        if (nextOpInStack == NULL) break;
                        currOpInStack->ptr->parent = nextOpInStack->ptr;
                        nextOpInStack->ptr->op2 = currOpInStack->ptr;
                        nextOpInStack->ptr->op2Type = OPNODE_OP;
                        lastRemaining = currOpInStack->ptr;
                        node_stack_remove(&stack, currOpInStack);
                        currOpInStack = nextOpInStack;
                    }
                    currValOp = lastRemaining;
                    currValType = VALTYPE_OP;
                    node_stack_remove(&stack, currOpInStack);
                }
                break;
        }
        /*printf("Current stack: ");
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
                case 4:
                    strcpy(opNameBuf, "(");
                    break;
            }
            printf("%s ", opNameBuf);
            curr = curr->prev;
        }
        printf("\n");*/
        pos++;
    }
    if (currValType == VALTYPE_OP) {
        if (stack.top == NULL) {
            currOp = NULL;
        } else {
            currOp = stack.top->ptr;
            currOp->op2Type = OPNODE_OP;
            currOp->op2 = currValOp;
        }
    }
    if (currOp == NULL) {
        currOp = (opNode *) malloc(sizeof(opNode));
        currOp->operation = 0; // plus
        if (currValType == VALTYPE_NUMBER) {
            currOp->op1Type = OPNODE_CONST;
            currOp->val1 = currNum;
        } else if (currValType == VALTYPE_VAR) {
            currOp->op1Type = OPNODE_VAR;
            currOp->val1 = currVar;
        } else if (currValType == VALTYPE_OP) {
            currOp->op1Type = OPNODE_OP;
            currOp->op1 = currValOp;
        }
        currOp->op2Type = OPNODE_CONST;
        currOp->val2 = 0;
        /*printf("Traversing nodes:\n");
        print_node(currOp);
        printf("\n");*/
        return currOp;
    }
    if (currOp->op2Type == VALTYPE_UNDEF) {
        if (currValType == VALTYPE_NUMBER) {
            currOp->op2Type = OPNODE_CONST;
            currOp->val2 = currNum;
        } else if (currValType == VALTYPE_VAR) {
            currOp->op2Type = OPNODE_VAR;
            currOp->val2 = currVar;
        } else if (currValType == VALTYPE_OP) {
            currOp->op2Type = OPNODE_OP;
            currOp->op2 = currValOp;
        }
    }
    node_stack_pop(&stack); // remove currOp from stack
    //printf("Nodes remaining on stack: %d\n", node_stack_length(&stack));
    while (!node_stack_is_empty(&stack)) {
        opNode* poppedNode = node_stack_pop(&stack);
        currOp->parent = poppedNode;
        poppedNode->op2 = currOp;
        poppedNode->op2Type = OPNODE_OP;
        currOp = poppedNode;
    }
    /*printf("Traversing nodes:\n");
    print_node(currOp);
    printf("\n");*/
    return currOp;
}

double evaluate_term(opNode* startNode, double varVal) {
    opNode currNode = *startNode;
    double value1;
    if (currNode.op1Type == OPNODE_CONST) {
        value1 = currNode.val1;
    } else if (currNode.op1Type == OPNODE_VAR) {
        value1 = varVal;
    } else {
        value1 = evaluate_term(currNode.op1, varVal);
    }
    double value2;
    if (currNode.op2Type == OPNODE_CONST) {
        value2 = currNode.val2;
    } else if (currNode.op2Type == OPNODE_VAR) {
        value2 = varVal;
    } else {
        value2 = evaluate_term(currNode.op2, varVal);
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
        case 5:
            return pow(value1, value2);
    }
}

symbolField getFields(u8 symbol) {
    symbolField res = {symbol >> 5, symbol & 0b11111};
    return res;
}

void print_char_console(u8 input) {
    if (getFields(input).type == OPTYPE_CONST) {
        printf("%d", input);
    } else if (getFields(input).type == OPTYPE_SIMPLE) {
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
    } else if (getFields(input).type == OPTYPE_VAR) {
        char varNameBuf[16];
        switch(getFields(input).value) {
            case 0:
                strcpy(varNameBuf, "x");
                break;
            case 1:
                strcpy(varNameBuf, "y");
                break;
            case 2:
                strcpy(varNameBuf, "z");
                break;
        }
        printf("%s", varNameBuf);
    } else if (getFields(input).type == OPTYPE_BEGIN) {
        char varNameBuf[16];
        switch(getFields(input).value) {
            case 4:
                strcpy(varNameBuf, "(");
                break;
        }
        printf("%s", varNameBuf);
    } else if (getFields(input).type == OPTYPE_END) {
        char varNameBuf[16];
        switch(getFields(input).value) {
            case 4:
                strcpy(varNameBuf, ")");
                break;
        }
        printf("%s", varNameBuf);
    }
}

/*int main(int argc, char** argv) {
    int i = 0;
    while (input[i] != CHAR_END) {
        print_char_console(input[i]);
        i++;
    }
    printf("\n");
    opNode* term = parse_term(input);
    printf("res = %f\n", evaluate_term(term, 0.3));
    term_free(term);
}*/