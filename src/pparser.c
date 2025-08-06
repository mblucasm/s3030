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

#include <pparser.h>
#include <stdio.h>

Token parser_create_op(Op op) {
    return (Token) {
        .type = TT_OP,
        .as.an_op = op,
    };
}

Token parser_create_float(float value) {
    return (Token) {
        .type = TT_FLOAT,
        .as.a_float = value,
    };
}

int parser_eval_postfix(Token *expr, size_t expr_len, float *stack, float *result) {

    size_t idx = 0;
    for(size_t i = 0; i < expr_len; ++i) {

        switch(expr[i].type) {

            case TT_FLOAT:
                stack[idx++] = expr[i].as.a_float;
            break;

            case TT_OP: {
                float b = stack[--idx];
                float a = stack[--idx];
                switch(expr[i].as.an_op) {
                    case OP_ADD: stack[idx++] = a + b; break;
                    case OP_SUB: stack[idx++] = a - b; break;
                    case OP_MUL: stack[idx++] = a * b; break;
                    case OP_DIV: {
                        if(b == 0) return PARSER_E_DIV0;
                        stack[idx++] = a / b;
                    } break;
                    default: return PARSER_E_UNKNOP;
                }
            } break;

            default: return PARSER_E_UNKNTT;
        }
    }

    if(idx != 1) return PARSER_E_WNGEXPR;
    *result = stack[0];
    return 0;
}

char parser_char_from_op(Op op) {
    switch(op) {
        case OP_ADD: return '+';
        case OP_SUB: return '-';
        case OP_MUL: return '*';
        case OP_DIV: return '/';
        case OP_N: default: return 'X';
    }
}

Op parser_op_from_char(const char c) {
    switch(c) {
        case '+': return OP_ADD;
        case '-': return OP_SUB;
        case '*': return OP_MUL;
        case '/': return OP_DIV;
        default: return OP_N;
    }
}

void parser_print_expr(Token *expr, size_t expr_len) {
    for(size_t i = 0; i < expr_len; ++i) {
        if(expr[i].type == TT_FLOAT) printf("%g ", expr[i].as.a_float);
        else if(expr[i].type == TT_OP) printf("%c ", parser_char_from_op(expr[i].as.an_op));
    }
}