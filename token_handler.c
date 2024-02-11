#include "9cc.h"

// 変数のinitialかどうかを判別する。
bool is_ident1(char c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_');
}

// 変数の二文字目以降の文字かどうかを判別する。
bool is_ident2(char c)
{
    return is_ident1(c) || ('0' <= c && c <= '9');
}

bool is_alnum(char c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || (c == '_');
}

bool consume_reserved(char *op)
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
    Token *cur_token = token;
    token = token->next;
    return cur_token;
}

bool consume_controls(char *s)
{
    if (token->kind != TK_CONTROLS || strlen(s) != token->len || memcmp(s, token->str, token->len))
    {
        return false;
    }
    token = token->next;
    return true;
}

bool consume_sizeof()
{
    if (token->kind != TK_SIZEOF || strlen("sizeof") != token->len || memcmp("sizeof", token->str, token->len))
    {
        return false;
    }
    token = token->next;
    return true;
}

bool consume_type(char *s)
{
    if (token->kind != TK_TYPE || strlen(s) != token->len || memcmp(s, token->str, token->len))
    {
        return false;
    }
    token = token->next;
    return true;
}

void expect_type(char *s)
{
    if (token->kind != TK_TYPE || strlen(s) != token->len || memcmp(s, token->str, token->len))
    {
        error_at(token->str, "型宣言をしてください。");
    }
    token = token->next;
    return;
}

bool consume_return()
{
    if (token->kind != TK_RETURN || strncmp(token->str, "return", 6))
    {
        return false;
    }
    token = token->next;
    return true;
}

void expect_reserved(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(op, token->str, token->len))
    {
        error_at(token->str, "'%s'ではありません", op);
    }
    token = token->next;
    return;
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
