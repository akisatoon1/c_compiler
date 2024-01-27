#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include "9cc.h"

// 注意点
// call時に16倍数へのalignmentせず
// 符号付き整数(int)のみ実装中。ただしマイナスの表現は実装せず。
// ポインタの演算や代入は、全て同じ型であると仮定。
// a[num]は認めるが、num[a]は認められない(本来は認められるが使わないので認められないことにした)。

// 実装済み
// int 4byte, ptr 8byte

// コンパイルする文字列
char *user_input;

Token *token;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        error("引数の個数が正しくありません");
        return 1;
    }

    user_input = argv[1];
    token = tokenize(user_input);

    // function vector (main)
    Function *funcs = program();

    printf(".intel_syntax noprefix\n");

    while (funcs)
    {
        gen_function(funcs);
        funcs = funcs->next;
    }

    return 0;
}

void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    fprintf(stderr, "token->str: %s\n\n", token->str);
    exit(1);
}

void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);

    // print space ' ' at fixed number times.
    for (int i = 0; i < pos; i++)
    {
        fprintf(stderr, "%s", " ");
    }

    // point to a place where an error occurs
    fprintf(stderr, "^\n");

    // print error message
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");

    // print the rest of token
    fprintf(stderr, "token->str rest: '%s'\n\n", token->str);
    exit(1);
}

// (8,16)=>16, (17,16)=>32, (9,8)=>16
int align_to(int n, int align)
{
    return (n - 1 + align) / align * align;
}