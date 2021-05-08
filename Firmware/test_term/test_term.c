#include "../common/term.h"
#include <stdio.h>
#include <math.h>

int evalterm(u8 *input, double varVal, double *out) {
    int syntaxErrRes = term_checkSyntax(input);
    if (syntaxErrRes > -1) {
        return syntaxErrRes;
    }
    opNode* term = term_parse(input);
    printf("  ");
    term_print_node(term);
    printf("\n");
    *out = term_evaluate(term, varVal);
    term_free(term, 1);
    return -1;
}

typedef struct {
    u8 input[70];
    const char *disp;
    double varVal;
    int syntaxError;
    double returnVal;
} testParams;

void startTest(char *name) {
    printf("Running tests \"%s\"\n", name);
}

void testPass(int index, const char *name) {
    printf("  Test %d: %s [\x1B[32mPASS\x1B[0m]\n", index + 1, name);
}

void failSyntaxTest(int index, const char *name, int actual, int expect) {
    printf("  Test %d (%s) [\x1B[31mFAIL\x1B[0m]\n", index + 1, name);
    if (expect == -1) {
        printf("    No syntax error should have been detected, but was found at %d\n", actual);
    } else {
        printf("    Syntax error detected at %d, should have been at %d\n", actual, expect);
    }
}

void failEquality(int index, const char *name, double actual, double expect) {
    printf("  Test %d (%s) [\x1B[31mFAIL\x1B[0m]\n", index + 1, name);
    printf("    Result was %g, should have been %g\n", actual, expect);
}

void doTests(testParams *inputs, int count) {
    for (int i = 0; i < count; i++) {
        u8 *input = inputs[i].input;
        double res;
        int syntaxError = evalterm(input, inputs[i].varVal, &res);
        if (syntaxError != inputs[i].syntaxError) {
            failSyntaxTest(i, inputs[i].disp, syntaxError, inputs[i].syntaxError);
            continue;
        }
        if (syntaxError == -1) {
            if (fabs(res - inputs[i].returnVal) > 1e-7) {
                failEquality(i, inputs[i].disp, res, inputs[i].returnVal);
                continue;
            }
        }
        testPass(i, inputs[i].disp);
    }
}

#define runTest(t) startTest(#t);doTests(t, sizeof(t)/sizeof(testParams))

testParams simple_order[] = {
    {{TERM_CONST_1, TERM_OP_PLUS, TERM_CONST_2, CHAR_END}, "1 + 2", 0, -1, 3},
    {{TERM_CONST_2, TERM_OP_PLUS, TERM_CONST_1, CHAR_END}, "2 + 1", 0, -1, 3},
    {{TERM_CONST_1, TERM_OP_PLUS, TERM_CONST_9, TERM_OP_MULT, TERM_CONST_2, CHAR_END}, "1 + 9 * 2", 0, -1, 19},
    {{TERM_CONST_1, TERM_OP_MULT, TERM_CONST_9, TERM_OP_PLUS, TERM_CONST_2, CHAR_END}, "1 * 9 + 2", 0, -1, 11}
};

testParams simple_floating[] = {
    {{TERM_CONST_1, TERM_OP_DIV, TERM_CONST_2, CHAR_END}, "1 / 2", 0, -1, 0.5},
    {{TERM_CONST_3, TERM_CONST_POINT, TERM_CONST_5, TERM_OP_MULT, TERM_CONST_1, TERM_CONST_0, CHAR_END}, "3.5 * 10", 0, -1, 35},
    {{TERM_CONST_4, TERM_CONST_POINT, TERM_CONST_5, TERM_OP_MULT, TERM_CONST_1, TERM_CONST_0, TERM_CONST_POINT, TERM_CONST_3, TERM_OP_DIV, TERM_CONST_3, CHAR_END}, "4.5 * 10.3 / 3", 0, -1, 15.45},
};

testParams variable_implicit_mult[] = {
    {{TERM_CONST_3, TERM_VAR_X, CHAR_END}, "3x", 2, -1, 6},
    {{TERM_CONST_3, TERM_VAR_X, TERM_OP_PLUS, TERM_CONST_2, CHAR_END}, "3x+2", 2, -1, 8},
    {{TERM_VAR_X, TERM_OP_PLUS, TERM_CONST_3, TERM_VAR_X, CHAR_END}, "x+3x", 2, -1, 8},
    {{TERM_CONST_3, TERM_VAR_X, TERM_OP_MINUS, TERM_VAR_X, TERM_CONST_2, CHAR_END}, "3x-x2", 2, -1, 2}
};

testParams function_sin[] = {
    {{TERM_OP_SIN, TERM_CONST_2, TERM_OP_BRACK_CLOSE, CHAR_END}, "sin(2)", 0, -1, 0.90929742682},
    {{TERM_OP_SIN, TERM_CONST_2, TERM_OP_PLUS, TERM_CONST_3, TERM_OP_BRACK_CLOSE, CHAR_END}, "sin(2+3)", 0, -1, -0.95892427},
    {{TERM_OP_SIN, TERM_CONST_2, TERM_OP_BRACK_CLOSE, TERM_CONST_3, CHAR_END}, "sin(2)3", 0, -1, 2.72789228}
};

testParams brackets[] = {
    {{TERM_CONST_2, TERM_OP_PLUS, TERM_OP_BRACK_OPEN, TERM_CONST_4, TERM_OP_MULT, TERM_2, TERM_OP_BRACK_CLOSE, CHAR_END}, "2 + (4 * 2)", 0, -1, 10},
    {{TERM_OP_BRACK_OPEN, TERM_CONST_2, TERM_OP_PLUS, TERM_CONST_4, TERM_OP_BRACK_CLOSE, TERM_OP_MULT, TERM_2, CHAR_END}, "(2 + 4) * 2", 0, -1, 12},
    {{TERM_CONST_4, TERM_OP_BRACK_OPEN, TERM_CONST_2, TERM_OP_PLUS, TERM_CONST_4, TERM_OP_BRACK_CLOSE, TERM_2, TERM_OP_PLUS, TERM_1, CHAR_END}, "4(2 + 4)2 + 1", 0, -1, 49},
};

testParams syntax_errors[] = {
    {{TERM_CONST_2, TERM_OP_PLUS, CHAR_END}, "2 +", 0, 2, -1},
    {{TERM_OP_PLUS, TERM_CONST_2, CHAR_END}, "+ 2", 0, 0, -1},
    {{TERM_CONST_2, TERM_OP_PLUS, TERM_OP_BRACK_OPEN, TERM_CONST_5, CHAR_END}, "2 + (5", 0, 4, -1},
    {{TERM_CONST_2, TERM_OP_PLUS, TERM_OP_BRACK_OPEN, TERM_CONST_5, TERM_OP_POW, TERM_OP_BRACK_CLOSE, TERM_CONST_2, CHAR_END}, "2 + (5 ^ ) 2", 0, 5, -1},
};

int main(int argc, char **argv) {
    term_init();
    runTest(simple_order);
    runTest(simple_floating);
    runTest(variable_implicit_mult);
    runTest(brackets);
    runTest(function_sin);
    runTest(syntax_errors);
}