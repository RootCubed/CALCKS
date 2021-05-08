#include "term.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// This array contains information about operation precedence.
// It is initialized in term_init so that it is clearer which operation has which precedence.
u8 precedences[8];

void term_init() {
    precedences[TERM_PLUS]    = 1;
    precedences[TERM_MINUS]   = 1;
    precedences[TERM_MULT]    = 2;
    precedences[TERM_DIV]     = 2;
    precedences[TERM_POW]     = 3;
    precedences[TERM_BRACKET] = 0;
}

// Functions related to the operation stack

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

// Deallocates an opNode (with recursion or without)
void term_free(opNode* node, int recursive) {
    opNode currNode = *node;
    if (currNode.op1Type == OPNODE_OP && recursive) {
        term_free(currNode.arg1.op, recursive);
    }
    if (currNode.op2Type == OPNODE_OP && recursive) {
        term_free(currNode.arg2.op, recursive);
    }
    free(node);
}

int term_checkSyntax(u8 *input) {
    int bracketDepth = 0;
    int i = 0;
    int hasHadComma = 0;
    if (input[0] == CHAR_END) return 0;
    symbolField firstInput = getFields(input[0]);
    if (firstInput.type != OPTYPE_CONST && firstInput.type != OPTYPE_VAR && firstInput.type != OPTYPE_BEGIN) return 0;
    while (input[i] != CHAR_END) {
        symbolField f = getFields(input[i]);
        symbolField fNext = getFields(input[i + 1]);
        int isLastInput = (input[i + 1] == CHAR_END);
        if (f.type != OPTYPE_CONST) {
            hasHadComma = 0;
        }
        if (input[i] == TERM_CONST_POINT) {
            if (hasHadComma) return i;
            if (isLastInput || fNext.type != OPTYPE_CONST) return i + 1;
            hasHadComma = 1;
        }
        if (f.type == OPTYPE_SIMPLE) {
            if (isLastInput || (fNext.type != OPTYPE_CONST && fNext.type != OPTYPE_VAR && input[i + 1] != TERM_OP_BRACK_OPEN)) return i + 1;
        }
        if (f.type == OPTYPE_BEGIN) {
            switch (input[i]) {
                case TERM_OP_BRACK_OPEN:
                case TERM_OP_SIN:
                case TERM_OP_COS:
                case TERM_OP_TAN:
                    if (input[i + 1] == TERM_OP_BRACK_CLOSE) return i + 1;
                    bracketDepth++;
                    break;
            }
        }
        if (input[i] == TERM_OP_BRACK_CLOSE) {
            if (bracketDepth == 0) return i;
            bracketDepth--;
        }
        i++;
    }
    if (bracketDepth != 0) return i;
    return NO_SYNTAX_ERROR;
}

// Allocates and initializes an opNode with the given operation
opNode *term_createOpNode(u8 op) {
    opNode *node = (opNode *) malloc(sizeof(opNode));
    memset(node, 0, sizeof(opNode));
    node->operation = op;
    return node;
}

// Nodes to be multiplied by implicit multiplication
opNode *currImplMult[64];
int currImplMultPos = -1;
opNode *topImplMultNode() {
    if (currImplMultPos < 0) return NULL;
    return currImplMult[currImplMultPos];
}

int addImplMultNode(opNode *node) {
    if (currImplMultPos >= 63) return 0;
    currImplMultPos++;
    currImplMult[currImplMultPos] = node;
    return 1;
}

// This is executed whenever a block of implicit multiplications is ended.
opNode *collateMultNodes() {
    if (currImplMultPos < 0) return NULL;
    if (getFields(topImplMultNode()->operation).type == OPTYPE_BEGIN) return NULL;

    /*printf("collating mult nodes from array state:\n");
    debugMultNodes();
    printf("\n");*/

    opNode *resNode = term_createOpNode(TERM_MULT);

    opNode *currNode = resNode;
    currNode->op1Type = OPNODE_OP;
    currNode->arg1.op = topImplMultNode();
    currNode->op2Type = OPNODE_CONST;
    currNode->arg2.d = 1;
    currImplMultPos--;
    while (currImplMultPos >= 0) {
        switch (topImplMultNode()->operation) {
            case TERM_BRACKET:
            case TERM_SIN:
            case TERM_COS:
            case TERM_TAN:
                return resNode;
        }
        opNode *newNode = term_createOpNode(TERM_MULT);
        newNode->op1Type = OPNODE_OP;
        newNode->arg1.op = topImplMultNode();
        newNode->op2Type = OPNODE_CONST;
        newNode->arg2.d = 1;

        newNode->parent = currNode;
        currNode->op2Type = OPNODE_OP;
        currNode->arg2.op = newNode;

        currNode = newNode;
        currImplMultPos--;
    }
    return resNode;
}

opNode* term_parse(u8 *input) {
    opStack stack = {NULL}; // Initialize empty operation stack
    u8 pos = 0;
    opNode* currOp = NULL;
    int currNumAfterPoint = -1;
    while (input[pos] != CHAR_END) {
        symbolField f = getFields(input[pos]);
        if (f.type != OPTYPE_CONST) {
            currNumAfterPoint = -1;
        }
        opNode *currImplMultNode = topImplMultNode();
        switch (f.type) {
            case OPTYPE_CONST:
                if (currImplMultNode == NULL || currImplMultNode->op1Type != OPNODE_CONST) {
                    opNode *newNode = term_createOpNode(TERM_PLUS);
                    newNode->op1Type = OPNODE_CONST;
                    newNode->arg1.d = 0;
                    newNode->op2Type = OPNODE_CONST;
                    newNode->arg2.d = 0;

                    if (!addImplMultNode(newNode)) {
                        // TODO: Error handling
                    }
                    currImplMultNode = topImplMultNode();
                    currNumAfterPoint = -1;
                }
                if (f.value == TERM_CONST_POINT) {
                    currNumAfterPoint = 0;
                    break;
                }
                double valueToAdd = f.value;
                if (currNumAfterPoint == -1) {
                    currImplMultNode->arg1.d *= 10;
                } else {
                    currNumAfterPoint++;
                    for (int i = 0; i < currNumAfterPoint; i++) {
                        valueToAdd /= 10;
                    }
                }
                currImplMultNode->arg1.d += valueToAdd;
                break;
            case OPTYPE_VAR: {
                opNode *newNode = term_createOpNode(TERM_MULT);
                newNode->op1Type = OPNODE_VAR;
                newNode->arg1.v = f.value;
                newNode->op2Type = OPNODE_CONST;
                newNode->arg2.d = 1;

                if (!addImplMultNode(newNode)) {
                    // TODO: Error handling
                }
                break;
            }
            case OPTYPE_SIMPLE:
                currOp = term_createOpNode(f.value);

                opNode *leftSide = collateMultNodes();
                if (leftSide == NULL) {
                    // TODO: Error handling
                }
                
                // Set op1 in case of empty stack, or if the current operator has higher precedence than the previous one
                currOp->op1Type = OPNODE_OP;
                currOp->arg1.op = leftSide;
                leftSide->parent = currOp;
                if (!node_stack_is_empty(&stack)) {
                    int currOpPre = precedences[currOp->operation];
                    if (currOpPre > precedences[stack.top->ptr->operation]) {
                        // If the precedence of the new operator is higher, leave the stack as-is
                    } else {
                        // First, find the node furthest back that has lower precedence
                        opStackNode* currOpInStack = stack.top;
                        while (currOpInStack->prev != NULL && currOpPre <= precedences[currOpInStack->prev->ptr->operation]) {
                            currOpInStack = currOpInStack->prev;
                        }
                        // Connect currOp to that node
                        currOpInStack->ptr->parent = currOp;
                        currOp->op1Type = OPNODE_OP;
                        currOp->arg1.op = currOpInStack->ptr;
                        // Now, while traversing the stack upwards, connect the nodes with each other
                        while (currOpInStack->next != NULL) {
                            opStackNode* nextOpInStack = currOpInStack->next;
                            nextOpInStack->ptr->parent = currOpInStack->ptr;
                            currOpInStack->ptr->op2Type = OPNODE_OP;
                            currOpInStack->ptr->arg2.op = nextOpInStack->ptr;
                            node_stack_remove(&stack, currOpInStack);
                            currOpInStack = nextOpInStack;
                        }
                        // Finally, add op2 on the last operation in the stack
                        currOpInStack->ptr->op2Type = OPNODE_OP;
                        currOpInStack->ptr->arg2.op = leftSide;
                        leftSide->parent = currOpInStack->ptr;
                        node_stack_remove(&stack, currOpInStack);
                    }
                }
                node_stack_push(&stack, currOp);
                break;
            case OPTYPE_BEGIN:
                currOp = term_createOpNode(f.value);
                node_stack_push(&stack, currOp);
                addImplMultNode(currOp);
                break;
            case OPTYPE_END: {
                opNode *lastBeforeEnd = collateMultNodes();
                opNode *operationContent;
                
                if (lastBeforeEnd == NULL) {
                    // TODO: Error handling
                }
                opStackNode* currOpInStack = stack.top;

                u8 endOp = topImplMultNode()->operation;

                if (currOpInStack->ptr->operation == endOp) {
                    operationContent = lastBeforeEnd;
                } else {
                    currOpInStack->ptr->op2Type = OPNODE_OP;
                    currOpInStack->ptr->arg2.op = lastBeforeEnd;

                    opNode* lastRemaining;
                    while (currOpInStack != NULL) {
                        if (currOpInStack->ptr->operation == endOp) {
                            break;
                        }
                        // Connect the nodes with each other
                        opStackNode* nextOpInStack = currOpInStack->prev;
                        if (nextOpInStack == NULL) {
                            break;
                        }
                        currOpInStack->ptr->parent = nextOpInStack->ptr;
                        currOpInStack->ptr->op2Type = OPNODE_OP;
                        nextOpInStack->ptr->arg2.op = currOpInStack->ptr;
                        lastRemaining = currOpInStack->ptr;
                        node_stack_remove(&stack, currOpInStack);
                        currOpInStack = nextOpInStack;
                    }
                    operationContent = lastRemaining;
                }
                node_stack_remove(&stack, currOpInStack);
                opNode *currImplMultNode = topImplMultNode(); // This should now be the opening operation of the block we're looking at
                currImplMultNode->op1Type = OPNODE_OP;
                currImplMultNode->arg1.op = operationContent;
                currImplMultNode->op2Type = OPNODE_CONST;
                currImplMultNode->arg2.d = 0;
                switch (currImplMultNode->operation) {
                    case TERM_BRACKET:
                        /*printf("before:\n");
                        debugMultNodes();*/
                        currImplMult[currImplMultPos] = currImplMult[currImplMultPos]->arg1.op; // Don't actually use the bracket node, but the child node of it
                        term_free(currImplMultNode, 0);
                        /*printf("after:\n");
                        debugMultNodes();*/
                        //addImplMultNode();
                        break;
                    case TERM_SIN:
                    case TERM_COS:
                    case TERM_TAN: {
                        opNode *dummyAdd = term_createOpNode(TERM_PLUS);
                        dummyAdd->op1Type = OPNODE_OP;
                        dummyAdd->arg1.op = currImplMultNode;
                        currImplMultNode->parent = dummyAdd;
                        dummyAdd->op2Type = OPNODE_CONST;
                        dummyAdd->arg2.d = 0;
                        currImplMultNode = dummyAdd;
                        break;
                    }
                }
                
                /*if (currImplMult[currImplMultPos]->operation != f.value) {
                    // error handling
                    printf("Something went wrong while parsing term! (OPNODE_END)\n");
                    printf("Expected %d, was %d.\n", f.value, currImplMult[currImplMultPos]->operation);
                    debugMultNodes();
                }*/
                break;
            }
        }
        pos++;
    }
    opNode *leftSide = collateMultNodes();
    if (stack.top == NULL) {
        currOp = NULL;
    } else {
        currOp = stack.top->ptr;
        currOp->op2Type = OPNODE_OP;
        currOp->arg2.op = leftSide;
    }
    if (currOp == NULL) {
        // use the constant
        return leftSide;
    } else {
        currOp->op2Type = OPNODE_OP;
        currOp->arg2.op = leftSide;
    }
    node_stack_pop(&stack); // Remove currOp from stack
    //printf("Nodes remaining on stack: %d\n", node_stack_length(&stack));
    while (!node_stack_is_empty(&stack)) {
        opNode* poppedNode = node_stack_pop(&stack);
        currOp->parent = poppedNode;
        poppedNode->op2Type = OPNODE_OP;
        poppedNode->arg2.op = currOp;
        currOp = poppedNode;
    }
    return currOp;
}

double term_evaluate(opNode* startNode, double varVal) {
    opNode currNode = *startNode;
    double value1;
    //term_debugNode(&currNode);
    switch (currNode.op1Type) {
        case OPNODE_CONST:
            value1 = currNode.arg1.d;
            break;
        case OPNODE_VAR:
            //value1 = currNode.arg1.v;
            value1 = varVal;
            break;
        case OPNODE_OP:
            value1 = term_evaluate(currNode.arg1.op, varVal);
            break;
    }
    double value2;
    if (currNode.operation != TERM_SIN) {
        switch (currNode.op2Type) {
            case OPNODE_CONST:
                value2 = currNode.arg2.d;
                break;
            case OPNODE_VAR:
                //value2 = currNode.arg2.v;
                value2 = varVal;
                break;
            case OPNODE_OP:
                value2 = term_evaluate(currNode.arg2.op, varVal);
                break;
        }
    }
    switch (currNode.operation) {
        case TERM_PLUS:
            return value1 + value2;
        case TERM_MINUS:
            return value1 - value2;
        case TERM_MULT:
            return value1 * value2;
        case TERM_DIV:
            return value1 / value2;
        case TERM_POW:
            return pow(value1, value2);
        case TERM_SIN:
            return sin(value1);
    }
}

symbolField getFields(u8 symbol) {
    symbolField res = {symbol >> 5, symbol & 0b11111};
    return res;
}

// Debug functions

void term_debugNode(opNode *node) {
    switch (node->op1Type) {
        case OPNODE_CONST:
            printf("|%f", node->arg1.d);
            break;
        case OPNODE_VAR:
            printf("|var_%d", node->arg1.v);
            break;
        case OPNODE_OP:
            printf("|(op 0x%08x)", (u32) node->arg1.op);
            break;
    }
    printf("|(operator %d)", node->operation);
    switch (node->op2Type) {
        case OPNODE_CONST:
            printf("|%f", node->arg2.d);
            break;
        case OPNODE_VAR:
            printf("|var_%d", node->arg2.v);
            break;
        case OPNODE_OP:
            printf("|(op 0x%08x)", (u32) node->arg2.op);
            break;
    }
    printf("|");
}

void debugMultNodes() {
    for (int i = 0; i < currImplMultPos + 1; i++) {
        term_debugNode(currImplMult[i]);
        printf(", ");
    }
}

void term_print_node(opNode* n) {
    char opNameBuf[16];
    switch(n->operation) {
        case TERM_PLUS:
            strcpy(opNameBuf, "plus");
            break;
        case TERM_MINUS:
            strcpy(opNameBuf, "minus");
            break;
        case TERM_MULT:
            strcpy(opNameBuf, "mult");
            break;
        case TERM_DIV:
            strcpy(opNameBuf, "div");
            break;
        case TERM_POW:
            strcpy(opNameBuf, "pow");
            break;
        case TERM_SIN:
            strcpy(opNameBuf, "sin");
            break;
    }
    printf("%s(", opNameBuf);
    switch (n->op1Type) {
        case OPNODE_CONST:
            printf("%g", n->arg1.d);
            break;
        case OPNODE_VAR:
            printf("var_%d", n->arg1.v);
            break;
        case OPNODE_OP:
            if (n->arg1.op != NULL) term_print_node(n->arg1.op);
            break;
    }
    printf(", ");
    switch (n->op2Type) {
        case OPNODE_CONST:
            printf("%g", n->arg2.d);
            break;
        case OPNODE_VAR:
            printf("var_%d", n->arg2.v);
            break;
        case OPNODE_OP:
            if (n->arg2.op != NULL) term_print_node(n->arg2.op);
            break;
    }
    printf(")");
}