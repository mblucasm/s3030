// Copyright © 2025 Lucas Martín
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the “Software”), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef PPARSER_H
#define PPARSER_H

#include <stddef.h>

typedef enum {TT_FLOAT, TT_OP, TT_N} TokenType;
typedef enum {OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_N} Op;
enum {PARSER_E_UNKNTT = 1, PARSER_E_UNKNOP, PARSER_E_WNGEXPR, PARSER_E_DIV0};

typedef union {
    Op an_op;
    float a_float;
} TokenValue;

typedef struct {
    TokenType type;
    TokenValue as;
} Token;

Token parser_create_op(Op op);
Token parser_create_float(float value);

Op parser_op_from_char(char c);
char parser_char_from_op(Op op);

void parser_print_expr(Token *expr, size_t expr_len);

// Evaluates a postfix expression into a float result
// Returns 0 on success
int parser_eval_postfix(Token *expr, size_t expr_len, float *stack, float *result);

#endif // PPARSER_H