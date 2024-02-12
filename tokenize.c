#include "9cc.h"

// keyword
static bool is_keyword(char *p, char *s);
static bool is_alnum(char c);

bool is_keyword(char *p, char *s)
{
    return !strncmp(p, s, strlen(s)) && !is_alnum(p[strlen(s)]);
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
        // 長い記号を先にトークナイズする。
        if (strncmp(p, ">=", 2) == 0 || strncmp(p, "<=", 2) == 0 || strncmp(p, "==", 2) == 0 || strncmp(p, "!=", 2) == 0)
        {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }
        if (strchr("+-*&/()<>;={}[],", *p))
        {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }
        if (strchr("\"", *p))
        {
            char *q = ++p;
            while (!strchr("\"", *p))
                p++;
            cur = new_token(TK_STRING, cur, q, p++ - q);
            continue;
        }
        if (is_keyword(p, "return"))
        {
            cur = new_token(TK_RETURN, cur, p, 6);
            p += 6;
            continue;
        }
        if (is_keyword(p, "if"))
        {
            cur = new_token(TK_CONTROLS, cur, p, 2);
            p += 2;
            continue;
        }
        if (is_keyword(p, "else"))
        {
            cur = new_token(TK_CONTROLS, cur, p, 4);
            p += 4;
            continue;
        }
        if (is_keyword(p, "while"))
        {
            cur = new_token(TK_CONTROLS, cur, p, 5);
            p += 5;
            continue;
        }
        if (is_keyword(p, "for"))
        {
            cur = new_token(TK_CONTROLS, cur, p, 3);
            p += 3;
            continue;
        }
        if (is_keyword(p, "int"))
        {
            cur = new_token(TK_TYPE, cur, p, 3);
            p += 3;
            continue;
        }
        if (is_keyword(p, "char"))
        {
            cur = new_token(TK_TYPE, cur, p, 4);
            p += 4;
            continue;
        }
        if (is_keyword(p, "sizeof"))
        {
            cur = new_token(TK_SIZEOF, cur, p, 6);
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
            continue;
        }
        error("トークナイズできません。*p: %c", *p);
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}
