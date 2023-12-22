#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include "9cc.h"

bool is_ident1(char c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_');
}

bool is_ident2(char c)
{
    return is_ident1(c) || ('0' <= c && c <= '9');
}

bool is_alnum(char c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || (c == '_');
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

Token *tokenize(char *p)
{
    Token head;
    head.next = NULL;
    Token *cur = &head;
    while (*p)
    {
        if (isspace(*p))
        {
            p++;
            continue;
        }
        if (strncmp(p, ">=", 2) == 0 || strncmp(p, "<=", 2) == 0 || strncmp(p, "==", 2) == 0 || strncmp(p, "!=", 2) == 0)
        {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }
        if (strchr("+-*/()<>;=", *p))
        {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            // fprintf(stderr, "tokenize reserved. cur->str: '%s'\n", cur->str);
            continue;
        }

        if (!strncmp(p, "return", 6) && !is_alnum(p[6]))
        {
            cur = new_token(TK_RETURN, cur, p, 6);
            // fprintf(stderr, "tokenize return. cur->str: '%s'\n", cur->str);
            p += 6;
            continue;
        }

        if (is_ident1(*p))
        {
            char *q = p;
            cur = new_token(TK_IDENT, cur, p++, 0);
            while (is_ident2(*p))
            {
                p++;
            }
            cur->len = p - q;
            continue;
        }
        if (isdigit(*p))
        {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            // fprintf(stderr, "tokenize digit. cur->str: '%s'\n", cur->str);
            continue;
        }
        error("トークナイズできません。*p: %c", *p);
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}

bool consume(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(op, token->str, token->len))
    {
        return false;
    }
    token = token->next;
    return true;
}

Token *consume_ident()
{
    if (token->kind != TK_IDENT || token->str[0] < 'a' || 'z' < token->str[0])
    {
        return NULL;
    }
    Token *ident_token = token;
    token = token->next;
    return ident_token;
}

bool consume_return()
{
    if (token->kind != TK_RETURN || strncmp(token->str, "return", 6))
    {
        // printf("tokenkind: %d\n", token->kind);
        return false;
    }
    token = token->next;
    return true;
}

void expect(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(op, token->str, token->len))
    {
        error("'%s'ではありません", op);
    }
    token = token->next;
}

int expect_number()
{
    if (token->kind != TK_NUM)
    {
        error_at(token->str, "数ではありません");
    }
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof()
{
    return (token->kind == TK_EOF);
}
