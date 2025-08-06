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

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include <pparser.h>
#include <catalan.h>
#include <perms.h>

#ifdef _WIN32
#   define DELIM '\\'
#else
#   define DELIM '/'
#endif

#define FLAG_a  (1 << 0)
#define FLAG_h  (1 << 1)
#define FLAG_n  (1 << 2)
#define FLAG_f  (1 << 3)
#define FLAG_s  (1 << 4)
#define FLAG_qm (1 << 5)

#define XCLUSIVE_a  (0 | FLAG_h | FLAG_qm)
#define XCLUSIVE_h  (0 | FLAG_a | FLAG_n | FLAG_qm)
#define XCLUSIVE_n  (0 | FLAG_h | FLAG_qm)
#define XCLUSIVE_f  (0)
#define XCLUSIVE_s  (0)
#define XCLUSIVE_qm (0 | FLAG_a | FLAG_h | FLAG_n)

#define FACTORIAL_UB   (20)
#define DEFAULT_TARGET (10.0f)
const char * PROGRAM;

struct fc {
    float n;
    int count;
};

void usage(void);
char *arg_shift(int *argc, char ***argv);
int args_parse(int *argc, char ***argv, float *nums, int *nums_len, float *target, float *epsilon, bool bans[OP_N], uint32_t *flags);
int args_parse_h(uint32_t *flags);
int args_parse_a(uint32_t *flags);
int args_parse_n(uint32_t *flags);
int args_parse_qm(uint32_t *flags);

const char *findr(const char *hay_stack, const char needle);
bool starts_with(const char *base, size_t len, const char *prefix, size_t plen);
void updatefc(struct fc *haystack, int *len, const float needle);

Op get_first_op(bool bans[OP_N]);
bool isop(char c);
bool all_ops_banned(bool bans[OP_N]);
bool update_ops(Token *expr, int expr_len, Op first_op, bool bans[OP_N]);

int get_expr_len(int nops);
uint64_t factorial_lookup(int x);
uint64_t calc_den(float *nums, int nums_len);
void load_ith_permutation_jth_path(Token *expr, float *ith_perm, uint8_t *paths, uint64_t j, int nops, Op first_op);

int main(int argc, char **argv) {

    const char * const program_path = arg_shift(&argc, &argv);
    PROGRAM = findr(program_path, DELIM);
    PROGRAM = PROGRAM ? PROGRAM + 1 : program_path;

    if(!argc) {
        usage();
        return 1;
    }

    bool bans[OP_N] = {0};
    float *nums = malloc(argc * sizeof(float));
    int nums_len = 0;

    if(nums == NULL) {
        fprintf(stderr, "ERROR: Could not allocate for the numbers provided, exiting...\n");
        return 1;
    }

    uint32_t flags = 0;
    float epsilon = 0.0f;
    float target = DEFAULT_TARGET;
    if(args_parse(&argc, &argv, nums, &nums_len, &target, &epsilon, bans, &flags)) goto defer1;

    if(nums_len < 2) {
        fprintf(stderr, "ERROR: At least 2 numbers need to be provided, exiting...\n");
        goto defer1;
    }

    if(all_ops_banned(bans)) {
        fprintf(stderr, "ERROR: At least 1 op needs to be unbanned, exiting...\n");
        goto defer1;
    }

    Op first_op = get_first_op(bans);
    if(first_op == OP_N) {
        fprintf(stderr, "UNREACHABLE\n");
        goto defer1;
    }

    uint64_t num = (flags & FLAG_f) ? 1 : factorial_lookup(nums_len);
    if(num == 0) {
        fprintf(stderr, "ERROR: Calculation of factorial(%d) overflows uint64_t, upper bound is factorial(%d), exiting...\n", nums_len, FACTORIAL_UB);
        goto defer1;
    }
    uint64_t den = (flags & FLAG_f) ? 1 : calc_den(nums, nums_len);
    if(den == 0) {
        fprintf(stderr, "ERROR: Could not allocate for the temp 'struct fc' array, exiting...\n");
        goto defer1;
    }

    uint64_t nperms = num / den;
    float *perms = malloc(sizeof(float) * nperms * nums_len);
    if(perms == NULL) {
        fprintf(stderr, "ERROR: Could not allocate to store all the permutations, exiting...\n");
        goto defer1;
    }

    if(flags & FLAG_s && !(flags & FLAG_f)) printf("Generating permutations...\n");

    if(flags & FLAG_f) {
        memcpy(perms, nums, sizeof(float) * nperms * nums_len);
    } else if(perms_unique(nums, nums_len, perms)) {
        fprintf(stderr, "ERROR: Could not allocate for the necessary temp variables when generating the permutations, exiting...\n");
        goto defer2;
    }

    int nops = nums_len - 1;
    int path_len = catalan_path_len_bits(nops);
    uint64_t npaths = catalan_number_for(nops);

    if(flags & FLAG_s) printf("Generating Catalan paths (Dyck words)...\n");
    uint8_t *paths = catalan_paths_for(nops);
    if(paths == NULL) {
        fprintf(stderr, "ERROR: Failed to generate all possible catalan paths, could not allocate the necessary buffers, exiting...\n");
        goto defer2;
    }
    
    int expr_len = get_expr_len(nops);
    Token *expr = malloc(sizeof(Token) * expr_len);
    if(expr == NULL) {
        fprintf(stderr, "ERROR: Could not allocate for the expression, exiting...\n");
        goto defer3;
    }
    
    float *stack = malloc(sizeof(float) * nums_len);
    if(stack == NULL) {
        fprintf(stderr, "ERROR: Could not allocate for the parser's stack, exiting...\n");
        goto defer4;
    }
    
    float result;
    uint64_t sols = 0;
    
    if(flags & FLAG_s) printf("Building and evaluating expressions...\n");
    for(uint64_t i = 0; i < nperms; ++i) {
        for(uint64_t j = 0; j < npaths; ++j) {
            
            bool rpt = true;
            load_ith_permutation_jth_path(expr, perms + (i * nums_len), paths, j, path_len, first_op);
            
            while(rpt) {
                
                int err = parser_eval_postfix(expr, expr_len, stack, &result);
                switch(err) {
                    case 0: break;
                    case PARSER_E_DIV0: break;
                    case PARSER_E_UNKNTT:  fprintf(stderr, "ERROR: When evaluating expression found an unknown token type, exiting...\n");      goto defer5;
                    case PARSER_E_UNKNOP:  fprintf(stderr, "ERROR: When evaluating expression found an unknown or unhandled op, exiting...\n"); goto defer5;
                    case PARSER_E_WNGEXPR: fprintf(stderr, "ERROR: When evaluating expression, this was wrongfuly formulated, exiting...\n");   goto defer5;
                    default: fprintf(stderr, "ERROR: Unhandled error by parser, exiting...\n"); goto defer5;
                }
                
                if(!err && (result >= target - epsilon) && (result <= target + epsilon)) {
                    ++sols;
                    if(flags & FLAG_n) printf("%zu >> ", sols);
                    if(flags & FLAG_a) {
                        parser_print_expr(expr, expr_len);
                        printf("= %g", target);
                        if(epsilon) printf(" (%+g)\n", result - target);
                        else putchar('\n');
                    } else if(flags & FLAG_qm) {
                        printf("Yes\n");
                        goto exit0;
                    } else if(!(flags & FLAG_h)) {
                        parser_print_expr(expr, expr_len);
                        printf("= %g", target);
                        if(epsilon) printf(" (%+g)\n", result - target);
                        else putchar('\n');
                        goto exit0;
                    }
                }
                
                rpt = update_ops(expr, expr_len, first_op, bans);
            }
        }
    }
    
    if(flags & FLAG_s) printf("Search exhausted\n");
    if(flags & FLAG_qm) printf("No\n");
    if(flags & FLAG_h) {
        if(sols) printf("Total number of solutions: %zu", sols);
        else printf("No solutions found\n");
    }
    
exit0:
    free(stack);
    free(expr);
    free(paths);
    free(perms);
    free(nums);
    return 0;

defer5:
    free(stack);
defer4:
    free(expr);
defer3:
    free(paths);
defer2:
    free(perms);
defer1:
    free(nums);
    return 1;
}

bool update_ops(Token *expr, int expr_len, Op first_op, bool bans[OP_N]) {
    int i = expr_len;
    while((--i) > 0) {
        if(expr[i].type == TT_OP) {
            expr[i].as.an_op++;
            while(bans[expr[i].as.an_op] && expr[i].as.an_op < OP_N) expr[i].as.an_op++;
            if(expr[i].as.an_op != OP_N) return true;
            expr[i].as.an_op = first_op;
        }
    }
    return false;
}

int get_expr_len(int nops) {
    return 2*nops + 1; // 2*nops + 1 = 2*(nums_len - 1) + 1 = 2*nums_len - 1
}

Op get_first_op(bool bans[OP_N]) {
    for(int i = 0; i < OP_N; ++i) if(!bans[i]) return i;
    return OP_N;
}

void load_ith_permutation_jth_path(Token *expr, float *ith_perm, uint8_t *paths, uint64_t j, int path_len, Op first_op) {
    expr[0] = parser_create_float(ith_perm[0]);
    expr[1] = parser_create_float(ith_perm[1]);
    int num_idx = 2;
    for(int k = 0; k < path_len; ++k) {
        int idx = j * path_len + k;
        if((paths[idx / 8] >> idx % 8) & 1) expr[k + 2] = parser_create_float(ith_perm[num_idx++]);
        else expr[k + 2] = parser_create_op(first_op);
    }
    expr[path_len + 2] = parser_create_op(first_op);
}

void updatefc(struct fc *haystack, int *len, const float needle) {
    bool found = false;
    for(int i = 0; i < *len; ++i) {
        if(haystack[i].n == needle) {
            haystack[i].count++;
            found = true;
        }
    }
    if(!found) {
        haystack[*len].n = needle;
        haystack[(*len)++].count = 1;
    }
}

uint64_t calc_den(float *nums, int nums_len) {

    int len = 0;
    struct fc *fc = malloc(sizeof(struct fc) * nums_len);
    if(fc == NULL) return 0;
    for(int i = 0; i < nums_len; ++i) updatefc(fc, &len, nums[i]);

    uint64_t den = 1;
    for(int i = 0; i < len; ++i) den *= factorial_lookup(fc[i].count);

    free(fc);
    return den;
}

void usage(void) {
    printf("usage: %s [options] <float1> <float2> [float3 ...]\n\n", PROGRAM);
    printf("options:\n");
    printf("  --help       print this message and exit\n");
    printf("  -e <float>   set the tolerance (epsilon) (default is 0)\n");
    printf("  -e=<float>   set the tolerance (alternative syntax)\n");
    printf("  -a           find all solutions\n");
    printf("  -h           find but hide all solutions\n");
    printf("  -n           number the solutions\n");
    printf("  -f           freeze numbers in place (do not permute them)\n");
    printf("  -s           shows progress messages\n");
    printf("  -?           reports whether at least one solution exists\n");
    printf("  op           ban the specified operator\n");
    printf("  -t <float>   set the target (default is %g)\n", DEFAULT_TARGET);
    printf("  -t=<float>   set the target (alternative syntax)\n\n");

    printf("Bannable ops are:\n");
    printf("  +   the addition operator\n");
    printf("  -   the subtraction operator\n");
    printf("  *   the multiplication operator (can also be banned with 'm')\n");
    printf("  /   the division operator\n\n");

    printf("Brute-forces all valid expressions to reach a target value\n");
    printf("Tries permutations, operator arrangements and expression shapes unless specified otherwise\n\n");
    printf("Note: s3030 outputs expressions using postfix notation\n");
}

bool all_ops_banned(bool bans[OP_N]) {
    int s = 0;
    for(int i = 0; i < OP_N; ++i) s += bans[i];
    return (s == OP_N);
}

char *arg_shift(int *argc, char ***argv) {
    char *arg = NULL;
    if(*argc > 0) {
        arg = **argv;
        --(*argc);
        ++(*argv);
    } return arg;
}

bool starts_with(const char *base, size_t len, const char *prefix, size_t plen) {
    if(len < plen) return false;
    return (memcmp(base, prefix, sizeof(char) * plen) == 0);
}

int args_parse_h(uint32_t *flags) {
    if(*flags & XCLUSIVE_h) {
        fprintf(stderr, "ERROR: Can't specify -h with -a nor -n nor -?\n");
        return 1;
    }
    *flags |= FLAG_h;
    return 0;
}

int args_parse_a(uint32_t *flags) {
    if(*flags & XCLUSIVE_a) {
        fprintf(stderr, "ERROR: Can't specify -a with -h nor -?\n");
        return 1;
    }
    *flags |= FLAG_a;
    return 0;
}

int args_parse_n(uint32_t *flags) {
    if(*flags & XCLUSIVE_n) {
        fprintf(stderr, "ERROR: Can't specify -n with -h nor -?\n");
        return 1;
    }
    *flags |= FLAG_n;
    return 0;
}

int args_parse_qm(uint32_t *flags) {
    if(*flags & XCLUSIVE_qm) {
        fprintf(stderr, "ERROR: Can't specify -? with -a nor -h nor -n\n");
        return 1;
    }
    *flags |= FLAG_qm;
    return 0;
}

int args_parse(int *argc, char ***argv, float *nums, int *nums_len, float *target, float *epsilon, bool bans[OP_N], uint32_t *flags) {

    while(*argc > 0) {

        char *arg = arg_shift(argc, argv);
        size_t len = strlen(arg);

        if(strcmp(arg, "--help") == 0) {
            usage();
            free(nums);
            exit(0);
        }

        if(isop(arg[0]) && len == 1) bans[parser_op_from_char(arg[0])] = true;
        else if(arg[0] == 'm' && len == 1) bans[OP_MUL] = true;

        else if(strcmp(arg, "-t") == 0) {
            char *nt = arg_shift(argc, argv);
            if(nt == NULL) {
                fprintf(stderr, "ERROR: Expected a number (float) after -t argument but got nothing, exiting...\n");
                return 1;
            }
            *target = strtof(nt, NULL);
        }

        else if(strcmp(arg, "-e") == 0) {
            char *eps = arg_shift(argc, argv);
            if(eps == NULL) {
                fprintf(stderr, "ERROR: Expected a number (float) after -e argument but got nothing, exiting...\n");
                return 1;
            }
            float e = strtof(eps, NULL);
            *epsilon = e > 0 ? e : -e;
        }

        else if(starts_with(arg, len, "-t=", 3)) {
            if(len - 3 == 0) {
                fprintf(stderr, "ERROR: Expected a number (float) after -t= argument but got nothing, exiting...\n");
                return 1;
            }
            *target = strtof(arg + 3, NULL);
        }

        else if(starts_with(arg, len, "-e=", 3)) {
            if(len - 3 == 0) {
                fprintf(stderr, "ERROR: Expected a number (float) after -e= argument but got nothing, exiting...\n");
                return 1;
            }
            float e = strtof(arg + 3, NULL);
            *epsilon = e > 0 ? e : -e;
        }

        else if(arg[0] == '-' && len > 1 && (isalpha(arg[1]) || arg[1] == '?')) {
            for(int i = 1; i < (int)len; ++i) {
                switch(arg[i]) {
                    case 'a': if(args_parse_a(flags)) return 1; break;
                    case 'h': if(args_parse_h(flags)) return 1; break;
                    case 'n': if(args_parse_n(flags)) return 1; break;
                    case 'f': *flags |= FLAG_f;  break;
                    case 's': *flags |= FLAG_s;  break;
                    case '?': if(args_parse_qm(flags)) return 1; break;
                    default: fprintf(stderr, "ERROR: Unknown symbol '%c' in arg '%s', exiting...\n", arg[i], arg); return 1; break;
                }
            }
        }

        else nums[(*nums_len)++] = strtof(arg, NULL);
    }

    return 0;
}

bool isop(const char c) {
    for(int i = 0; i < OP_N; ++i) if(c == parser_char_from_op(i)) return true;
    return false;
}

uint64_t factorial_lookup(int x) {
    if(x < 0 || x > FACTORIAL_UB) return 0;
    return (uint64_t[]) {1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880, 3628800, 39916800, 479001600, 6227020800, 87178291200, 1307674368000, 20922789888000, 355687428096000, 6402373705728000, 121645100408832000}[x];
}

const char *findr(const char *hay_stack, const char needle) {
    size_t len = strlen(hay_stack);
    for(size_t i = 0; i < len; ++i) if(hay_stack[len - i - 1] == needle) return hay_stack + (len - i - 1);
    return NULL;
}