#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include "9cc.h"

// call時に16倍数へのalignmentせず
// 符号なし整数のみ実装中。
// 関数の識別子を保存していない。
// 関数の型がintのみ
// 全ての型のサイズが8byte

// コンパイルする文字列
char *user_input;
Token *token;
Node *code[100];

// 変数のvector
LVar *locals;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        error("引数の個数が正しくありません");
        return 1;
    }

    user_input = argv[1];
    token = tokenize(user_input);

    // function vector
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
    for (int i = 0; i < pos; i++)
    {
        fprintf(stderr, "%s", " ");
    }
    // fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^\n");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    fprintf(stderr, "token->str rest: '%s'\n\n", token->str);
    exit(1);
}

int align_to(int n, int align)
{
    // printf("n: %d\n", n);
    return (n - 1 + align) / align * align;
}